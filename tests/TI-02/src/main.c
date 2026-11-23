/*
 * TI-02: teste de integração das duas vias (2 semáforos de pedestres e 2 de
 * carros, em duas placas).
 *
 * Módulos integrados, em cada placa: lib/enlace (UART2 com a outra placa),
 * lib/semaforo_veiculo e lib/semaforo_pedestres. O modo noturno é um dublê
 * (src/dubles.c), e os botões não são usados. Uma placa é o mestre (via A) e
 * a outra, a escrava (via B): -DPAPEL=mestre ou -DPAPEL=escrava.
 *
 * Ciclo esperado (cerca de 8 s), com as vias em fases opostas:
 *   via A: verde 3 s, amarelo 1 s, vermelho ~4 s (enquanto a via B abre)
 *   via B: vermelho ~4 s (enquanto a via A abre), verde 3 s, amarelo 1 s
 * Em cada via, os pedestres ficam no verde durante o vermelho dos carros.
 *
 * Além das linhas que os módulos já mandam para a serial, este programa tem
 * um MONITOR: uma thread que lê os quatro LEDs desta placa e o sinal do
 * enlace, espera 1 tick (0,1 ms) e lê de novo. Com o tempo da própria
 * leitura, sai uma amostra a cada 0,2 ms, mais ou menos.
 *   - Violações (devem ser 0):
 *       A. pedestres no verde sem os carros no vermelho fixo;
 *       B. pedestres com o verde e o vermelho acesos juntos;
 *       C. carros desta placa abertos (verde ou amarelo) sem a permissão da
 *          outra placa: no mestre, com o último quadro da escrava dizendo
 *          PARADO = 0; na escrava, com o último quadro do mestre dizendo
 *          VEZ = 0. Sem enlace, o sinal da outra placa vale 0, e abrir assim
 *          também conta como violação.
 *   - Aberturas: quantas vezes os carros desta placa abriram (verde).
 *   - Janelas de troca (devem ser curtas): todos no vermelho, pedestres
 *     apagados e carros apagados. As duas últimas só contam depois que os
 *     dois semáforos acenderam a primeira luz.
 * A cada RESUMO_MS (10 s), main() manda o resumo para a serial.
 *
 * O monitor de cada placa só vê a própria placa e o que recebe da outra. A
 * regra "nunca as duas vias abertas ao mesmo tempo" é conferida comparando as
 * linhas `carros VERDE`, `carros AMARELO` e `carros VERMELHO` das duas placas
 * pelo tempo do PC, no log com as duas seriais (tests/ferramentas/captura_serial.py),
 * e no vídeo.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"
#include "semaforo_pedestres.h"
#include "semaforo_veiculo.h"

#ifndef TI02_MESTRE
#define TI02_MESTRE 1
#endif

#define RESUMO_MS          10000
#define TAMANHO_PILHA      1024
#define PRIORIDADE_MONITOR 4   /* acima das luzes (5): amostra até no meio de uma troca */
#define PRIORIDADE_RESUMO  5   /* a mesma das luzes: as linhas da serial não se misturam */

#define PAPEL_NOME (TI02_MESTRE ? "MESTRE (via A)" : "ESCRAVA (via B)")
/* Sinal da outra placa que autoriza esta a abrir */
#define PERMISSAO_NOME (TI02_MESTRE ? "PARADO" : "VEZ")

/* Os mesmos pinos que os módulos usam (frdm_kl25z.overlay e aliases da placa) */
static const struct gpio_dt_spec carro_vermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec carro_verde = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec ped_vermelho = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_vermelho), gpios);
static const struct gpio_dt_spec ped_verde = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_verde), gpios);

/* Uma janela de troca: mede quanto tempo seguido uma condição fica verdadeira */
struct janela {
	int64_t inicio;   /* tick em que começou; -1 fora da janela */
	int64_t maior;    /* maior duração vista, em ticks */
	uint32_t vezes;   /* quantas janelas terminaram */
};

/* Resultados do monitor, protegidos por mutex_resultado */
static struct {
	uint32_t amostras;
	uint32_t viol_abertos;      /* A */
	uint32_t viol_ped_duas;     /* B */
	uint32_t viol_permissao;    /* C */
	int64_t primeira_viol;      /* tick da primeira violação; -1 sem violação */
	char tipo_primeira;         /* 'A', 'B' ou 'C' */
	uint32_t aberturas;         /* vezes que o verde dos carros acendeu */
	struct janela todos_vermelho;
	struct janela ped_apagado;
	struct janela carros_apagado;
} res;

K_MUTEX_DEFINE(mutex_resultado);

static void janela_inicia(struct janela *j)
{
	j->inicio = -1;
	j->maior = 0;
	j->vezes = 0;
}

static void janela_amostra(struct janela *j, bool dentro, int64_t agora)
{
	if (dentro && j->inicio < 0) {
		j->inicio = agora;
	} else if (!dentro && j->inicio >= 0) {
		int64_t duracao = agora - j->inicio;

		if (duracao > j->maior) {
			j->maior = duracao;
		}
		j->vezes++;
		j->inicio = -1;
	}
}

static void registra_violacao(uint32_t *contador, char tipo, int64_t agora)
{
	(*contador)++;
	if (res.primeira_viol < 0) {
		res.primeira_viol = agora;
		res.tipo_primeira = tipo;
	}
}

static void t_monitor(void *p1, void *p2, void *p3)
{
	bool carros_acenderam = false;
	bool ped_acenderam = false;
	bool verde_antes = false;

	while (1) {
		/* 1 = aceso, qualquer que seja a polaridade do pino (vem do devicetree) */
		bool cvm = gpio_pin_get_dt(&carro_vermelho) == 1;
		bool cvd = gpio_pin_get_dt(&carro_verde) == 1;
		bool pvm = gpio_pin_get_dt(&ped_vermelho) == 1;
		bool pvd = gpio_pin_get_dt(&ped_verde) == 1;
		bool permissao = enlace_ler();   /* PARADO no mestre, VEZ na escrava */
		int64_t agora = k_uptime_ticks();

		bool carros_fechados = cvm && !cvd;   /* vermelho fixo; vermelho + verde é o amarelo */
		bool carros_abertos = cvd;            /* verde, ou amarelo (verde + vermelho) */

		carros_acenderam |= cvm || cvd;
		ped_acenderam |= pvm || pvd;
		bool depois_do_boot = carros_acenderam && ped_acenderam;

		k_mutex_lock(&mutex_resultado, K_FOREVER);
		res.amostras++;
		if (pvd && !carros_fechados) {
			registra_violacao(&res.viol_abertos, 'A', agora);
		}
		if (pvd && pvm) {
			registra_violacao(&res.viol_ped_duas, 'B', agora);
		}
		if (carros_abertos && !permissao) {
			registra_violacao(&res.viol_permissao, 'C', agora);
		}
		/* Abertura: o verde (sozinho) acendeu agora */
		bool verde_so = cvd && !cvm;

		if (verde_so && !verde_antes) {
			res.aberturas++;
		}
		verde_antes = verde_so;
		janela_amostra(&res.todos_vermelho, carros_fechados && pvm && !pvd, agora);
		janela_amostra(&res.ped_apagado, depois_do_boot && !pvm && !pvd, agora);
		janela_amostra(&res.carros_apagado, depois_do_boot && !cvm && !cvd, agora);
		k_mutex_unlock(&mutex_resultado);

		k_sleep(K_TICKS(1));
	}
}

/* Começa só depois de os semáforos configurarem os pinos, em main() */
K_THREAD_DEFINE(id_t_monitor, TAMANHO_PILHA, t_monitor,
		NULL, NULL, NULL, PRIORIDADE_MONITOR, 0, SYS_FOREVER_MS);

static uint32_t ticks_para_us(int64_t ticks)
{
	return (uint32_t)k_ticks_to_us_floor64(ticks);
}

static void imprime_resumo(void)
{
	k_mutex_lock(&mutex_resultado, K_FOREVER);
	uint32_t amostras = res.amostras;
	uint32_t viol_a = res.viol_abertos;
	uint32_t viol_b = res.viol_ped_duas;
	uint32_t viol_c = res.viol_permissao;
	int64_t primeira = res.primeira_viol;
	char tipo = res.tipo_primeira;
	uint32_t aberturas = res.aberturas;
	struct janela c = res.todos_vermelho;
	struct janela d = res.ped_apagado;
	struct janela e = res.carros_apagado;
	k_mutex_unlock(&mutex_resultado);

	uint32_t agora = k_uptime_get_32();

	printk("%u ms | TI-02 resumo %s: %u amostras, %u aberturas dos carros; violacoes: "
	       "A=%u (pedestres verde com carros abertos), B=%u (pedestres verde e vermelho juntos), "
	       "C=%u (carros abertos sem %s da outra placa)\n", agora, PAPEL_NOME, amostras,
	       aberturas, viol_a, viol_b, viol_c, PERMISSAO_NOME);
	if (primeira >= 0) {
		printk("%u ms | TI-02 resumo: PRIMEIRA VIOLACAO (%c) em %u ms\n", agora, tipo,
		       (uint32_t)k_ticks_to_ms_floor64(primeira));
	}
	printk("%u ms | TI-02 resumo: maiores janelas: todos no vermelho %u us (%u trocas), "
	       "pedestres apagados %u us (%u), carros apagados %u us (%u)\n", agora,
	       ticks_para_us(c.maior), c.vezes, ticks_para_us(d.maior), d.vezes,
	       ticks_para_us(e.maior), e.vezes);
}

int main(void)
{
	/* main imprime o resumo: mesma prioridade das luzes */
	k_thread_priority_set(k_current_get(), PRIORIDADE_RESUMO);

	res.primeira_viol = -1;
	janela_inicia(&res.todos_vermelho);
	janela_inicia(&res.ped_apagado);
	janela_inicia(&res.carros_apagado);

	printk("TI-02: duas vias, esta placa e o %s\n", PAPEL_NOME);

	int ret = enlace_iniciar(TI02_MESTRE ? ENLACE_MESTRE : ENLACE_ESCRAVA);

	if (ret == 0) {
		ret = semaforo_pedestres_iniciar();   /* pedestres começam no vermelho */
	}
	if (ret == 0) {
		/* false: sem o botão de travessia; o verde dos carros sempre dura 3 s */
		ret = semaforo_veiculo_iniciar(false);   /* carros começam no vermelho */
	}
	if (ret < 0) {
		printk("TI-02: erro %d ao iniciar\n", ret);
		return ret;
	}

	k_thread_start(id_t_monitor);

	int64_t proximo = k_uptime_get();

	while (1) {
		/* Marca de tempo fixa: os resumos não escorregam */
		proximo += RESUMO_MS;
		k_sleep(K_TIMEOUT_ABS_MS(proximo));
		imprime_resumo();
	}

	return 0;
}
