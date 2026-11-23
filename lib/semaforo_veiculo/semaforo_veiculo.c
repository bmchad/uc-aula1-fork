/*
 * Semáforo de veículos.
 *
 * Três threads (verde, amarelo e vermelho) em cadeia de k_sem, uma luz por vez
 * sob mutex_carros. A thread do vermelho é a única que conversa com a outra
 * placa e com os pedestres:
 *   - ao acender, libera os pedestres da própria placa;
 *   - faz o handshake (mestre: dá a vez e espera a escrava fechar de novo;
 *     escrava: espera a vez);
 *   - antes de apagar, encerra os pedestres.
 * Regras do handshake: src/README.md, seção 5.2.
 * Na etapa 1 (uma placa só) não há handshake: o vermelho dura VERMELHO_MS.
 *
 * Modo ALERTA (RF-04 a RF-06): quando o enlace com a outra placa falha, a
 * placa sai da fase em que está e pisca, como no modo noturno:
 *   - no verde: o verde acaba na hora, e o amarelo começa a piscar;
 *   - no amarelo: o amarelo termina e continua piscando;
 *   - no vermelho: as esperas do handshake são canceladas, os pedestres
 *     fecham, e o amarelo começa a piscar.
 * Ao entrar no ALERTA, a placa publica o sinal seguro: mestre com VEZ = 0, e
 * escrava com PARADO = 0 (piscando não é vermelho fixo, então o mestre não
 * pode abrir). Quando o enlace volta, o pisca termina (em até 1 s), a placa
 * volta ao vermelho e recomeça como no boot: o mestre abre primeiro, assim
 * que vê a escrava parada no vermelho.
 */

#include "semaforo_veiculo.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"
#include "modo_noturno.h"
#include "semaforo_pedestres.h"

#define VERDE_MS       3000
#define VERDE_MIN_MS   1000    /* o botão de travessia não encurta abaixo disso */
#define AMARELO_MS     1000
#define VERMELHO_MS    4000    /* etapa 1: sem a outra placa, o vermelho tem tempo fixo */
#define PISCA_MS       1000
#define GUARDA_MS      50      /* folga para a outra placa reagir a um sinal */
#define RESPOSTA_MS    3000    /* mestre: prazo para a escrava pegar a vez */
#define MARGEM_MS      1000
/*
 * Mestre: depois de cancelar a vez, espera um verde e um amarelo inteiros da
 * escrava antes de seguir. Vale porque as duas placas usam esta biblioteca,
 * com os mesmos tempos.
 */
#define ESPERA_CANCELADA_MS (VERDE_MS + AMARELO_MS + MARGEM_MS)
#define AVISO_MS       10000   /* intervalo entre avisos de espera longa */
#define ESTAVEL_MS     1000    /* escrava: VEZ em 0 por esse tempo depois do boot */
#define CONSULTA_MS    50      /* intervalo máximo para conferir o enlace e o modo noturno */
#define SEM_PRAZO      (-1)    /* esperar_sinal: sem limite de tempo */
#define TOQUE_BLOQUEIO_MS 200  /* registro: toques mais próximos que isso contam como um */
#define TOQUE_SOLTO_MS    20   /* registro: intervalo para conferir se o botão foi solto */
#define TOQUE_REPIQUE_MS  50   /* registro: espera depois de soltar, para o repique passar */
#define TAMANHO_PILHA  768
#define PRIORIDADE_LED 5

/* O que a thread do vermelho decide no fim da espera */
enum decisao {
	ABRIR,     /* acende o verde */
	NOTURNO,   /* entra no modo noturno (ponto seguro) */
	ALERTA,    /* enlace em falha: entra no modo ALERTA */
};

/* Como o verde terminou */
enum fim_verde {
	VERDE_TEMPO,        /* os 3 s inteiros */
	VERDE_TRAVESSIA,    /* toque no botão de travessia */
	VERDE_ALERTA,       /* enlace em falha */
};

static const struct gpio_dt_spec led_vermelho =
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);   /* PTB18 */
static const struct gpio_dt_spec led_verde =
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);   /* PTB19 */
static const struct gpio_dt_spec botao_travessia =
	GPIO_DT_SPEC_GET(DT_NODELABEL(botao_travessia), gpios);
static struct gpio_callback callback_travessia;

K_MUTEX_DEFINE(mutex_carros);
K_SEM_DEFINE(vez_verde, 0, 1);
K_SEM_DEFINE(vez_amarelo, 0, 1);
K_SEM_DEFINE(vez_vermelho, 0, 1);
K_SEM_DEFINE(pedido_travessia, 0, 1);   /* ISR do botão -> t_carro_verde */
K_SEM_DEFINE(toque_travessia, 0, 1);    /* ISR do botão -> t_registro_travessia */

static bool mestre;
static bool uma_placa;       /* etapa 1: sem handshake */
static bool vez_consumida;   /* escrava: já abriu com o VEZ = 1 atual */
/*
 * true no boot e depois de cada ALERTA: o próximo vermelho recomeça o
 * revezamento (o mestre abre primeiro; a escrava espera VEZ = 0 estável).
 * Só as threads dos carros mexem nele, uma de cada vez (mutex_carros).
 */
static bool recomecar = true;

static void registra(const char *evento)
{
	printk("%u ms | carros %s\n", k_uptime_get_32(), evento);
}

/*
 * Sem debounce no controle: encurtar o verde atual pode acontecer várias vezes
 * sem efeito extra, e o verde descarta toques antigos ao começar. O segundo
 * k_sem é só para o registro na serial, que tem a própria filtragem.
 */
static void isr_travessia(const struct device *dev, struct gpio_callback *cb,
			  uint32_t pinos)
{
	k_sem_give(&pedido_travessia);
	k_sem_give(&toque_travessia);
}

/*
 * Registra cada toque no botão de travessia, em qualquer fase e em qualquer
 * modo, com o tempo do toque. Não muda o ciclo: quem age é a t_carro_verde.
 * O repique do fio é filtrado só aqui:
 *   1. registra o toque com o tempo em que acordou;
 *   2. ignora novas bordas por TOQUE_BLOQUEIO_MS;
 *   3. espera o botão ser solto e mais TOQUE_REPIQUE_MS;
 *   4. descarta as bordas desse intervalo.
 */
static void t_registro_travessia(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&toque_travessia, K_FOREVER);
		printk("%u ms | botao travessia: toque\n", k_uptime_get_32());

		k_msleep(TOQUE_BLOQUEIO_MS);
		while (gpio_pin_get_dt(&botao_travessia) == 1) {
			k_msleep(TOQUE_SOLTO_MS);
		}
		k_msleep(TOQUE_REPIQUE_MS);
		k_sem_reset(&toque_travessia);
	}
}

static void acende_amarelo(bool aceso)
{
	gpio_pin_set_dt(&led_vermelho, aceso);
	gpio_pin_set_dt(&led_verde, aceso);
}

/*
 * Entra no modo ALERTA. Publica o sinal seguro para a outra placa (mestre:
 * VEZ = 0; escrava: PARADO = 0, porque piscando ela não está no vermelho
 * fixo) e marca o recomeço do revezamento para quando o enlace voltar.
 */
static void entra_alerta(void)
{
	enlace_publicar(false);
	vez_consumida = false;
	recomecar = true;
	modo_alerta_aplicar(true);
}

/* Espera `ms`, conferindo o enlace a cada CONSULTA_MS. Retorna 0, ou -ECANCELED se o enlace falhou */
static int esperar_tempo(int32_t ms)
{
	int64_t fim = k_uptime_get() + ms;

	while (!modo_alerta_pedido()) {
		int64_t falta = fim - k_uptime_get();

		if (falta <= 0) {
			return 0;
		}
		k_msleep((int32_t)MIN(falta, CONSULTA_MS));
	}

	return -ECANCELED;
}

/*
 * Espera o sinal da outra placa chegar a `nivel`, por até `prazo_ms`
 * (SEM_PRAZO: sem limite), registrando `aviso` a cada AVISO_MS (NULL: sem
 * aviso). Retorna 0, -EAGAIN se o prazo acabou, ou -ECANCELED se o enlace
 * falhou: nesse caso o sinal não vale nada, porque sem enlace ele é 0.
 */
static int esperar_sinal(bool nivel, int32_t prazo_ms, const char *aviso)
{
	int64_t inicio = k_uptime_get();
	int64_t proximo_aviso = inicio + AVISO_MS;

	while (1) {
		if (modo_alerta_pedido()) {
			return -ECANCELED;
		}
		if (enlace_ler() == nivel) {
			return 0;
		}

		int64_t agora = k_uptime_get();

		if (prazo_ms != SEM_PRAZO && agora - inicio >= prazo_ms) {
			return -EAGAIN;
		}
		if (aviso != NULL && agora >= proximo_aviso) {
			registra(aviso);
			proximo_aviso += AVISO_MS;
		}
		(void)enlace_esperar(nivel, K_MSEC(CONSULTA_MS));
	}
}

/*
 * Mestre: dá a vez à escrava e espera ela abrir e fechar de novo.
 * Retorna 0, ou -ECANCELED se o enlace falhou (VEZ fica em 0).
 */
static int mestre_dar_a_vez(void)
{
	/* A escrava precisa estar no vermelho fixo antes de receber a vez */
	int ret = esperar_sinal(true, SEM_PRAZO, "aguardando a escrava parar");

	if (ret != 0) {
		return ret;
	}

	enlace_publicar(true);                                   /* VEZ = 1 */

	ret = esperar_sinal(false, RESPOSTA_MS, NULL);           /* PARADO = 0? */
	if (ret == -EAGAIN) {
		enlace_publicar(false);                          /* cancela: VEZ = 0 */

		/*
		 * Sem resposta, não dá para saber se a escrava abriu: o quadro dela
		 * pode ter se perdido justamente quando ela pegou a vez. Esperar o
		 * ciclo inteiro dela garante que ela já fechou quando o mestre
		 * reabrir.
		 */
		registra("escrava nao pegou a vez: esperando o ciclo dela");
		return esperar_tempo(ESPERA_CANCELADA_MS);
	}

	if (ret == 0) {
		ret = esperar_sinal(true, SEM_PRAZO, "aguardando a escrava fechar");   /* PARADO = 1 */
	}
	enlace_publicar(false);                                  /* VEZ = 0 */
	return ret;
}

/* Mestre, regra 1: só abre com VEZ = 0 (já está) e a escrava parada */
static int mestre_esperar_liberacao(void)
{
	int ret = esperar_tempo(GUARDA_MS);

	return ret != 0 ? ret : esperar_sinal(true, SEM_PRAZO, "aguardando a escrava parar");
}

/*
 * Escrava, no boot e depois de um ALERTA: só aceita a vez depois de ver
 * VEZ = 0 estável. Com duas escravas por engano, cada uma descarta os
 * quadros da outra (mesmo papel), o VEZ fica em 0 e nenhuma abre.
 * Retorna 0, ou -ECANCELED se o enlace falhou.
 */
static int escrava_esperar_vez_zerada(void)
{
	do {
		int ret = esperar_sinal(false, SEM_PRAZO, "aguardando VEZ = 0 do mestre");

		if (ret != 0) {
			return ret;
		}
	} while (enlace_aguardar_mudanca(K_MSEC(ESTAVEL_MS)) || enlace_ler());

	vez_consumida = false;
	return modo_alerta_pedido() ? -ECANCELED : 0;
}

/*
 * Escrava: espera a vez. Retorna ABRIR, NOTURNO (pedido pelo mestre) ou
 * ALERTA (enlace em falha). Cada VEZ = 1 só abre uma vez: VEZ precisa
 * voltar a 0 antes.
 */
static enum decisao escrava_esperar_a_vez(void)
{
	/* No ciclo normal a espera dura uns 4 s; passar de AVISO_MS é falha */
	int64_t aviso = k_uptime_get() + AVISO_MS;

	while (!modo_noturno_pedido()) {
		if (modo_alerta_pedido()) {
			return ALERTA;
		}
		if (k_uptime_get() >= aviso) {
			registra("aguardando a vez do mestre");
			aviso += AVISO_MS;
		}

		if (!enlace_ler()) {
			vez_consumida = false;
		} else if (!vez_consumida) {
			enlace_publicar(false);   /* PARADO = 0 antes de abrir */
			k_msleep(GUARDA_MS);

			/* Confere de novo: o mestre pode ter cancelado a vez ao mesmo tempo */
			if (enlace_ler() && !modo_alerta_pedido()) {
				vez_consumida = true;
				return ABRIR;
			}

			enlace_publicar(true);    /* cancelada: continua parada */
			continue;
		}

		enlace_aguardar_mudanca(K_MSEC(CONSULTA_MS));
	}

	return NOTURNO;
}

/*
 * Espera o fim do verde: VERDE_MS, ou um toque de travessia depois de
 * VERDE_MIN_MS, ou a falha do enlace (a qualquer momento).
 */
static enum fim_verde esperar_verde(void)
{
	int64_t inicio = k_uptime_get();

	while (1) {
		if (modo_alerta_pedido()) {
			return VERDE_ALERTA;
		}

		int64_t passou = k_uptime_get() - inicio;

		if (passou >= VERDE_MS) {
			return VERDE_TEMPO;
		}

		/* Acorda no máximo a cada CONSULTA_MS, com marcas de tempo fixas */
		int64_t ate = inicio + MIN(passou + CONSULTA_MS, VERDE_MS);

		if (passou < VERDE_MIN_MS) {
			k_sleep(K_TIMEOUT_ABS_MS(MIN(ate, inicio + VERDE_MIN_MS)));
		} else if (k_sem_take(&pedido_travessia, K_TIMEOUT_ABS_MS(ate)) == 0) {
			return VERDE_TRAVESSIA;
		}
	}
}

/*
 * Pisca o amarelo enquanto o modo NOTURNO ou o ALERTA estiverem em vigor.
 * A cada meio período, confere os pedidos: o ALERTA entra ou sai conforme o
 * enlace, e o noturno sai quando o pedido acaba (ele só entra no vermelho,
 * pelo ponto seguro). `aceso`: estado atual do amarelo.
 */
static void piscar(bool aceso)
{
	while (1) {
		bool alerta = modo_alerta_pedido();

		if (alerta && !modo_alerta_ativo()) {
			entra_alerta();
		} else if (!alerta && modo_alerta_ativo()) {
			modo_alerta_aplicar(false);
		}
		if (modo_noturno_ativo() && !modo_noturno_pedido()) {
			modo_noturno_aplicar(false);
		}
		if (!modo_pisca()) {
			return;
		}

		aceso = !aceso;
		acende_amarelo(aceso);
		/* Cada troca vai para a serial: é a evidência do pisca */
		registra(aceso ? "AMARELO aceso" : "AMARELO apagado");
		k_msleep(PISCA_MS);
	}
}

static void t_carro_verde(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&vez_verde, K_FOREVER);
		k_sem_reset(&pedido_travessia);   /* descarta toques e repiques antigos */
		k_mutex_lock(&mutex_carros, K_FOREVER);

		gpio_pin_set_dt(&led_verde, 1);
		registra("VERDE");

		switch (esperar_verde()) {
		case VERDE_TRAVESSIA:
			registra("travessia pedida: verde encurtado");
			break;
		case VERDE_ALERTA:
			registra("enlace em falha: verde encerrado");
			break;
		default:
			break;
		}

		gpio_pin_set_dt(&led_verde, 0);
		k_mutex_unlock(&mutex_carros);
		k_sem_give(&vez_amarelo);
	}
}

static void t_carro_amarelo(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&vez_amarelo, K_FOREVER);
		k_mutex_lock(&mutex_carros, K_FOREVER);

		bool aceso = false;

		/* Vindo do verde, sem falha: o amarelo normal de 1 s */
		if (!modo_pisca() && !modo_alerta_pedido()) {
			acende_amarelo(true);
			registra("AMARELO");
			k_msleep(AMARELO_MS);
			aceso = true;
		}

		/* Modo noturno (vindo do vermelho), ou falha do enlace em qualquer fase */
		if (modo_pisca() || modo_alerta_pedido()) {
			registra("AMARELO PISCANDO");
			piscar(aceso);
		}

		acende_amarelo(false);
		k_mutex_unlock(&mutex_carros);
		k_sem_give(&vez_vermelho);
	}
}

static void t_carro_vermelho(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&vez_vermelho, K_FOREVER);
		k_mutex_lock(&mutex_carros, K_FOREVER);

		gpio_pin_set_dt(&led_vermelho, 1);
		registra("VERMELHO");
		semaforo_pedestres_liberar();

		enum decisao decisao;

		if (uma_placa) {
			/* Etapa 1: o vermelho dura o tempo da travessia e acabou */
			k_msleep(VERMELHO_MS);
			decisao = modo_noturno_pedido() ? NOTURNO : ABRIR;
		} else if (mestre) {
			/* No recomeço (boot ou depois do ALERTA), o mestre abre primeiro */
			int ret = recomecar ? 0 : mestre_dar_a_vez();

			if (ret == 0 && modo_noturno_pedido()) {
				decisao = NOTURNO;   /* ponto seguro: a escrava está fechada e VEZ = 0 */
			} else if (ret == 0 && mestre_esperar_liberacao() == 0) {
				decisao = ABRIR;
			} else {
				decisao = ALERTA;
			}
		} else {
			enlace_publicar(true);    /* PARADO = 1: só depois de voltar ao vermelho */
			if (recomecar && escrava_esperar_vez_zerada() != 0) {
				decisao = ALERTA;
			} else {
				decisao = escrava_esperar_a_vez();
			}
		}
		recomecar = false;

		semaforo_pedestres_encerrar();
		gpio_pin_set_dt(&led_vermelho, 0);

		if (decisao == NOTURNO) {
			modo_noturno_aplicar(true);
			/* O mestre mantém VEZ = 0 no modo noturno: o próximo VEZ = 1 é uma vez nova */
			vez_consumida = false;
		} else if (decisao == ALERTA) {
			registra("enlace em falha: vermelho encerrado");
			entra_alerta();
		}

		k_mutex_unlock(&mutex_carros);
		k_sem_give(decisao == ABRIR ? &vez_verde : &vez_amarelo);
	}
}

/* As três começam bloqueadas: só rodam depois de semaforo_veiculo_iniciar() */
K_THREAD_DEFINE(id_t_carro_verde, TAMANHO_PILHA, t_carro_verde,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);
K_THREAD_DEFINE(id_t_carro_amarelo, TAMANHO_PILHA, t_carro_amarelo,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);
K_THREAD_DEFINE(id_t_carro_vermelho, TAMANHO_PILHA, t_carro_vermelho,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);
/*
 * Mesma prioridade das luzes: uma não interrompe a outra no meio de um printk,
 * então as linhas da serial não se misturam. O custo é o registro do toque
 * poder atrasar alguns ms se uma luz estiver imprimindo.
 */
K_THREAD_DEFINE(id_t_registro_travessia, TAMANHO_PILHA, t_registro_travessia,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);

int semaforo_veiculo_iniciar(bool usar_botao_travessia)
{
	int ret;

	mestre = enlace_sou_mestre();
	uma_placa = enlace_uma_placa();

	if (!gpio_is_ready_dt(&led_vermelho) || !gpio_is_ready_dt(&led_verde)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return ret;
	}

	if (usar_botao_travessia) {
		if (!gpio_is_ready_dt(&botao_travessia)) {
			return -ENODEV;
		}

		/* Pull-up e ativo em nível baixo vêm do devicetree */
		ret = gpio_pin_configure_dt(&botao_travessia, GPIO_INPUT);
		if (ret < 0) {
			return ret;
		}

		ret = gpio_pin_interrupt_configure_dt(&botao_travessia, GPIO_INT_EDGE_TO_ACTIVE);
		if (ret < 0) {
			return ret;
		}

		gpio_init_callback(&callback_travessia, isr_travessia, BIT(botao_travessia.pin));
		ret = gpio_add_callback(botao_travessia.port, &callback_travessia);
		if (ret < 0) {
			return ret;
		}
	}
	/*
	 * Sem o botão, ninguém dá pedido_travessia: a t_carro_verde espera os 3 s
	 * inteiros, e a t_registro_travessia fica bloqueada para sempre.
	 */

	k_sem_give(&vez_vermelho);   /* começa fechado */
	return 0;
}
