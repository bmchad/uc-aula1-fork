/*
 * TS-01: teste de sistema do modo noturno, numa placa só.
 *
 * Módulos do sistema: lib/modo_noturno (botão no D9: cada toque alterna o
 * pedido), lib/semaforo_veiculo e lib/semaforo_pedestres. O enlace é um dublê
 * de placa única (src/dubles.c), e o botão de travessia não é usado.
 *
 * Comportamento esperado:
 *   - sem pedido: ciclo normal (carros vermelho 4 s, verde 3 s, amarelo 1 s;
 *     pedestres no verde durante o vermelho dos carros);
 *   - toque no botão: pedido NOTURNO. A placa entra no modo noturno no fim do
 *     vermelho dos carros (ponto seguro), e então os carros piscam o amarelo
 *     e os pedestres piscam o vermelho, 1 s aceso e 1 s apagado;
 *   - novo toque: pedido NORMAL. A placa sai do pisca (em até 1 s), volta ao
 *     vermelho dos carros e retoma o ciclo normal.
 *
 * Além das linhas que os módulos já mandam para a serial (toques do botão,
 * `modo NOTURNO`/`modo NORMAL` e cada troca do pisca), este programa tem um
 * MONITOR: uma thread que lê os quatro LEDs e o modo, espera 1 tick (0,1 ms)
 * e lê de novo. Com o tempo da própria leitura, sai uma amostra a cada
 * 0,2 ms, mais ou menos.
 *   - Entradas e saídas do modo noturno, com o maior atraso entre o pedido
 *     (toque no botão) e a mudança do modo efetivo.
 *   - Meios períodos do pisca, só os inteiros (começam e terminam com o modo
 *     noturno ativo): amarelo dos carros e vermelho dos pedestres, com o
 *     menor e o maior. Esperado: 1000 ms (± 1%).
 *   - Violações (devem ser 0):
 *       A. pedestres no verde sem os carros no vermelho fixo (sempre);
 *       B. pedestres com o verde e o vermelho acesos juntos (sempre);
 *       D. no modo noturno, verde ou vermelho dos carros aceso sozinho
 *          (no pisca, só o amarelo pode acender);
 *       E. no modo noturno, verde dos pedestres aceso.
 * A cada RESUMO_MS (10 s), main() manda o resumo para a serial.
 *
 * O monitor lê o nível dos pinos (o que o firmware comanda nos LEDs), e não
 * a luz em si: o vídeo continua sendo a evidência do que aparece nos LEDs.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "modo_noturno.h"
#include "semaforo_pedestres.h"
#include "semaforo_veiculo.h"

#define RESUMO_MS          10000
#define TAMANHO_PILHA      1024
#define PRIORIDADE_MONITOR 4   /* acima das luzes (5): amostra até no meio de uma troca */
#define PRIORIDADE_RESUMO  5   /* a mesma das luzes: as linhas da serial não se misturam */

/* Os mesmos pinos que os módulos usam (frdm_kl25z.overlay e aliases da placa) */
static const struct gpio_dt_spec carro_vermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec carro_verde = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec ped_vermelho = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_vermelho), gpios);
static const struct gpio_dt_spec ped_verde = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_verde), gpios);

/* Meios períodos de uma luz que pisca */
struct pisca {
	bool estado;          /* aceso na última amostra */
	int64_t ultima_troca; /* tick da última troca com o modo ativo; -1 sem troca */
	uint32_t n;           /* meios períodos inteiros medidos */
	int64_t menor, maior; /* em ticks */
};

/* Entradas ou saídas do modo noturno */
struct mudanca {
	uint32_t n;
	int64_t maior_atraso;   /* em ticks, desde o pedido */
};

/* Resultados do monitor, protegidos por mutex_resultado */
static struct resultado {
	uint32_t amostras;
	uint32_t viol[4];        /* A, B, D, E */
	int64_t primeira_viol;   /* tick da primeira violação; -1 sem violação */
	char tipo_primeira;
	struct mudanca entradas, saidas;
	struct pisca amarelo, ped;
	bool ativo;
} res;

static const char tipos_viol[4] = {'A', 'B', 'D', 'E'};

K_MUTEX_DEFINE(mutex_resultado);

static void pisca_inicia(struct pisca *p)
{
	p->estado = false;
	p->ultima_troca = -1;
	p->n = 0;
	p->menor = INT64_MAX;
	p->maior = 0;
}

/* Mede o meio período que termina nesta troca, se ele começou com o modo ativo */
static void pisca_amostra(struct pisca *p, bool aceso, bool ativo, int64_t agora)
{
	if (!ativo) {
		p->ultima_troca = -1;
	} else if (aceso != p->estado) {
		if (p->ultima_troca >= 0) {
			int64_t meio = agora - p->ultima_troca;

			p->n++;
			p->menor = MIN(p->menor, meio);
			p->maior = MAX(p->maior, meio);
		}
		p->ultima_troca = agora;
	}
	p->estado = aceso;
}

static void violacao(int i, int64_t agora)
{
	res.viol[i]++;
	if (res.primeira_viol < 0) {
		res.primeira_viol = agora;
		res.tipo_primeira = tipos_viol[i];
	}
}

static void t_monitor(void *p1, void *p2, void *p3)
{
	bool pedido_antes = false;
	int64_t pedido_desde = k_uptime_ticks();

	while (1) {
		/* 1 = aceso, qualquer que seja a polaridade do pino (vem do devicetree) */
		bool cvm = gpio_pin_get_dt(&carro_vermelho) == 1;
		bool cvd = gpio_pin_get_dt(&carro_verde) == 1;
		bool pvm = gpio_pin_get_dt(&ped_vermelho) == 1;
		bool pvd = gpio_pin_get_dt(&ped_verde) == 1;
		bool pedido = modo_noturno_pedido();
		bool ativo = modo_noturno_ativo();
		int64_t agora = k_uptime_ticks();

		bool carros_fechados = cvm && !cvd;   /* vermelho fixo; vermelho + verde é o amarelo */

		if (pedido != pedido_antes) {
			pedido_antes = pedido;
			pedido_desde = agora;
		}

		k_mutex_lock(&mutex_resultado, K_FOREVER);
		res.amostras++;

		/* Mudança do modo efetivo: atraso desde o último toque que pediu essa mudança */
		if (ativo != res.ativo) {
			struct mudanca *m = ativo ? &res.entradas : &res.saidas;

			m->n++;
			if (ativo == pedido) {
				m->maior_atraso = MAX(m->maior_atraso, agora - pedido_desde);
			}
			res.ativo = ativo;
		}

		if (pvd && !carros_fechados) {
			violacao(0, agora);   /* A */
		}
		if (pvd && pvm) {
			violacao(1, agora);   /* B */
		}
		if (ativo && cvm != cvd) {
			violacao(2, agora);   /* D: só um dos dois acesos não é amarelo */
		}
		if (ativo && pvd) {
			violacao(3, agora);   /* E */
		}

		pisca_amostra(&res.amarelo, cvm && cvd, ativo, agora);
		pisca_amostra(&res.ped, pvm, ativo, agora);
		k_mutex_unlock(&mutex_resultado);

		k_sleep(K_TICKS(1));
	}
}

/* Começa só depois de os semáforos configurarem os pinos, em main() */
K_THREAD_DEFINE(id_t_monitor, TAMANHO_PILHA, t_monitor,
		NULL, NULL, NULL, PRIORIDADE_MONITOR, 0, SYS_FOREVER_MS);

static uint32_t ms(int64_t ticks)
{
	return (uint32_t)k_ticks_to_ms_floor64(ticks);
}

static void imprime_pisca(uint32_t agora, const char *nome, const struct pisca *p)
{
	if (p->n == 0) {
		printk("%u ms | TS-01 resumo: pisca %s: nenhum meio periodo inteiro ainda\n",
		       agora, nome);
	} else {
		printk("%u ms | TS-01 resumo: pisca %s: %u meios periodos, de %u a %u ms\n",
		       agora, nome, p->n, ms(p->menor), ms(p->maior));
	}
}

static void imprime_resumo(void)
{
	k_mutex_lock(&mutex_resultado, K_FOREVER);
	struct resultado r = res;   /* cópia, para imprimir sem segurar o mutex */
	k_mutex_unlock(&mutex_resultado);

	uint32_t agora = k_uptime_get_32();

	printk("%u ms | TS-01 resumo: modo %s; %u amostras; entradas no noturno: %u (maior atraso %u ms); "
	       "saidas: %u (maior atraso %u ms)\n", agora, r.ativo ? "NOTURNO" : "NORMAL",
	       r.amostras, r.entradas.n, ms(r.entradas.maior_atraso), r.saidas.n,
	       ms(r.saidas.maior_atraso));
	imprime_pisca(agora, "amarelo dos carros", &r.amarelo);
	imprime_pisca(agora, "vermelho dos pedestres", &r.ped);
	printk("%u ms | TS-01 resumo: violacoes: A=%u (pedestres verde com carros abertos), "
	       "B=%u (pedestres verde e vermelho juntos), D=%u (no noturno, luz dos carros "
	       "fora do amarelo), E=%u (no noturno, pedestres verde)\n", agora,
	       r.viol[0], r.viol[1], r.viol[2], r.viol[3]);
	if (r.primeira_viol >= 0) {
		printk("%u ms | TS-01 resumo: PRIMEIRA VIOLACAO (%c) em %u ms\n", agora,
		       r.tipo_primeira, ms(r.primeira_viol));
	}
}

int main(void)
{
	/* main imprime o resumo: mesma prioridade das luzes */
	k_thread_priority_set(k_current_get(), PRIORIDADE_RESUMO);

	res.primeira_viol = -1;
	pisca_inicia(&res.amarelo);
	pisca_inicia(&res.ped);

	printk("TS-01: modo noturno (botao no D9: cada toque alterna NOTURNO e NORMAL)\n");

	int ret = modo_noturno_iniciar(true);   /* true: configura o botão do D9 */

	if (ret == 0) {
		ret = semaforo_pedestres_iniciar();   /* pedestres começam no vermelho */
	}
	if (ret == 0) {
		/* false: sem o botão de travessia; o verde dos carros sempre dura 3 s */
		ret = semaforo_veiculo_iniciar(false);   /* carros começam no vermelho */
	}
	if (ret < 0) {
		printk("TS-01: erro %d ao iniciar\n", ret);
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
