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
 */

#include "semaforo_veiculo.h"

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
#define CONSULTA_MS    100     /* escrava: intervalo para conferir o modo noturno */
#define TAMANHO_PILHA  768
#define PRIORIDADE_LED 5

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

static bool mestre;
static bool uma_placa;       /* etapa 1: sem handshake */
static bool vez_consumida;   /* escrava: já abriu com o VEZ = 1 atual */

static void registra(const char *evento)
{
	printk("%u ms | carros %s\n", k_uptime_get_32(), evento);
}

/*
 * Sem debounce: encurtar o verde atual pode acontecer várias vezes sem
 * efeito extra, e o verde descarta toques antigos ao começar.
 */
static void isr_travessia(const struct device *dev, struct gpio_callback *cb,
			  uint32_t pinos)
{
	k_sem_give(&pedido_travessia);
}

static void acende_amarelo(bool aceso)
{
	gpio_pin_set_dt(&led_vermelho, aceso);
	gpio_pin_set_dt(&led_verde, aceso);
}

/* Espera a entrada do enlace chegar a um nível, avisando a cada AVISO_MS */
static void esperar_com_aviso(bool nivel, const char *motivo)
{
	while (enlace_esperar(nivel, K_MSEC(AVISO_MS)) != 0) {
		registra(motivo);
	}
}

/* Mestre: dá a vez à escrava e espera ela abrir e fechar de novo */
static void mestre_dar_a_vez(void)
{
	enlace_publicar(true);                                   /* VEZ = 1 */

	if (enlace_esperar(false, K_MSEC(RESPOSTA_MS)) != 0) {   /* PARADO = 0? */
		enlace_publicar(false);                          /* cancela: VEZ = 0 */

		/*
		 * Sem resposta, não dá para saber se a escrava abriu. Com o fio
		 * PARADO em curto com o GND, o mestre lê "parada" para sempre: a
		 * escrava pega a vez e abre sem o mestre ver. Esperar o ciclo
		 * inteiro dela garante que ela já fechou quando o mestre reabrir.
		 */
		registra("escrava nao pegou a vez: esperando o ciclo dela");
		k_msleep(ESPERA_CANCELADA_MS);
		return;
	}

	esperar_com_aviso(true, "aguardando a escrava fechar");  /* PARADO = 1 */
	enlace_publicar(false);                                  /* VEZ = 0 */
}

/* Mestre, regra 1: só abre com VEZ = 0 (já está) e a escrava parada */
static void mestre_esperar_liberacao(void)
{
	k_msleep(GUARDA_MS);
	esperar_com_aviso(true, "aguardando a escrava parar");
}

/*
 * Escrava, no boot: só aceita a vez depois de ver VEZ = 0 estável. Com duas
 * escravas por engano, cada uma vê o PARADO = 1 da outra e nenhuma abre.
 */
static void escrava_esperar_vez_zerada(void)
{
	do {
		esperar_com_aviso(false, "aguardando VEZ = 0 do mestre");
	} while (enlace_aguardar_mudanca(K_MSEC(ESTAVEL_MS)) || enlace_ler());

	vez_consumida = false;
}

/*
 * Escrava: espera a vez. Retorna true para abrir, ou false se o modo noturno
 * foi pedido. Cada VEZ = 1 só abre uma vez: VEZ precisa voltar a 0 antes.
 */
static bool escrava_esperar_a_vez(void)
{
	/* No ciclo normal a espera dura uns 4 s; passar de AVISO_MS é falha */
	k_timepoint_t aviso = sys_timepoint_calc(K_MSEC(AVISO_MS));

	while (!modo_noturno_pedido()) {
		if (sys_timepoint_expired(aviso)) {
			registra("aguardando a vez do mestre");
			aviso = sys_timepoint_calc(K_MSEC(AVISO_MS));
		}

		if (!enlace_ler()) {
			vez_consumida = false;
		} else if (!vez_consumida) {
			enlace_publicar(false);   /* PARADO = 0 antes de abrir */
			k_msleep(GUARDA_MS);

			/* Confere de novo: o mestre pode ter cancelado a vez ao mesmo tempo */
			if (enlace_ler()) {
				vez_consumida = true;
				return true;
			}

			enlace_publicar(true);    /* cancelada: continua parada */
			continue;
		}

		enlace_aguardar_mudanca(K_MSEC(CONSULTA_MS));
	}

	return false;
}

static void t_carro_verde(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&vez_verde, K_FOREVER);
		k_sem_reset(&pedido_travessia);   /* descarta toques e repiques antigos */
		k_mutex_lock(&mutex_carros, K_FOREVER);

		gpio_pin_set_dt(&led_verde, 1);
		registra("VERDE");
		k_msleep(VERDE_MIN_MS);
		if (k_sem_take(&pedido_travessia, K_MSEC(VERDE_MS - VERDE_MIN_MS)) == 0) {
			registra("travessia pedida: verde encurtado");
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

		if (modo_noturno_ativo()) {
			/* Pisca até o pedido do modo noturno acabar */
			registra("AMARELO PISCANDO");
			bool aceso = false;

			while (modo_noturno_pedido()) {
				aceso = !aceso;
				acende_amarelo(aceso);
				k_msleep(PISCA_MS);
			}
			modo_noturno_aplicar(false);
		} else {
			acende_amarelo(true);
			registra("AMARELO");
			k_msleep(AMARELO_MS);
		}

		acende_amarelo(false);
		k_mutex_unlock(&mutex_carros);
		k_sem_give(&vez_vermelho);
	}
}

static void t_carro_vermelho(void *p1, void *p2, void *p3)
{
	bool primeira = true;

	while (1) {
		k_sem_take(&vez_vermelho, K_FOREVER);
		k_mutex_lock(&mutex_carros, K_FOREVER);

		gpio_pin_set_dt(&led_vermelho, 1);
		registra("VERMELHO");
		semaforo_pedestres_liberar();

		bool abrir;   /* false = entrar no modo noturno */

		if (uma_placa) {
			/* Etapa 1: o vermelho dura o tempo da travessia e acabou */
			k_msleep(VERMELHO_MS);
			abrir = !modo_noturno_pedido();
		} else if (mestre) {
			if (!primeira) {
				mestre_dar_a_vez();
			}
			/* Ponto seguro: a escrava está fechada e VEZ = 0 */
			abrir = !modo_noturno_pedido();
			if (abrir) {
				mestre_esperar_liberacao();
			}
		} else {
			enlace_publicar(true);    /* PARADO = 1: só depois de voltar ao vermelho */
			if (primeira) {
				escrava_esperar_vez_zerada();
			}
			abrir = escrava_esperar_a_vez();
		}
		primeira = false;

		semaforo_pedestres_encerrar();
		gpio_pin_set_dt(&led_vermelho, 0);

		if (!abrir) {
			modo_noturno_aplicar(true);
			/* O mestre mantém VEZ = 0 no modo noturno: o próximo VEZ = 1 é uma vez nova */
			vez_consumida = false;
		}

		k_mutex_unlock(&mutex_carros);
		k_sem_give(abrir ? &vez_verde : &vez_amarelo);
	}
}

/* As três começam bloqueadas: só rodam depois de semaforo_veiculo_iniciar() */
K_THREAD_DEFINE(id_t_carro_verde, TAMANHO_PILHA, t_carro_verde,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);
K_THREAD_DEFINE(id_t_carro_amarelo, TAMANHO_PILHA, t_carro_amarelo,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);
K_THREAD_DEFINE(id_t_carro_vermelho, TAMANHO_PILHA, t_carro_vermelho,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);

int semaforo_veiculo_iniciar(void)
{
	int ret;

	mestre = enlace_sou_mestre();
	uma_placa = enlace_uma_placa();

	if (!gpio_is_ready_dt(&led_vermelho) || !gpio_is_ready_dt(&led_verde) ||
	    !gpio_is_ready_dt(&botao_travessia)) {
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

	k_sem_give(&vez_vermelho);   /* começa fechado */
	return 0;
}
