/*
 * TI-01: teste de integração entre o semáforo de pedestres e o de carros.
 *
 * Módulos integrados: lib/semaforo_veiculo (3 threads, mutex_carros) e
 * lib/semaforo_pedestres (2 threads, mutex_pedestres), numa placa só. Os
 * carros comandam os pedestres por semaforo_pedestres_liberar() e
 * semaforo_pedestres_encerrar(). O enlace e o modo noturno são dublês
 * (src/dubles.c), e o botão de travessia não é usado.
 *
 * Ciclo esperado (8 s), com os dois semáforos sincronizados:
 *   carros VERMELHO 4 s  <->  pedestres VERDE 4 s
 *   carros VERDE 3 s + AMARELO 1 s  <->  pedestres VERMELHO 4 s
 *
 * Além das linhas que os dois módulos já mandam para a serial (uma por troca
 * de luz), este programa tem um MONITOR: uma thread que lê os quatro LEDs a
 * cada 0,1 ms (1 tick) e confere a sincronia sozinha.
 *   - Violações (devem ser 0):
 *       A. pedestres no verde sem os carros no vermelho fixo (carros no verde,
 *          no amarelo ou apagados);
 *       B. pedestres com o verde e o vermelho acesos juntos.
 *   - Janelas de troca (devem ser curtas, de poucos ms):
 *       C. todos no vermelho (carros e pedestres), que acontece em cada troca
 *          entre os dois semáforos;
 *       D. pedestres sem nenhuma luz acesa;
 *       E. carros sem nenhuma luz acesa.
 *     As janelas D e E só contam depois que os dois semáforos acenderam a
 *     primeira luz (o boot fica de fora).
 * A cada RESUMO_MS (10 s), main() manda o resumo para a serial.
 *
 * O monitor lê o nível dos pinos (registrador PDIR da KL25Z), ou seja, o que
 * o firmware comanda nos LEDs, e não a luz em si: o vídeo continua sendo a
 * evidência do que aparece nos LEDs.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

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

/* Uma janela de troca: mede quanto tempo seguido uma condição fica verdadeira */
struct janela {
	int64_t inicio;   /* tick em que começou; -1 fora da janela */
	int64_t maior;    /* maior duração vista, em ticks */
	uint32_t vezes;   /* quantas janelas terminaram */
};

/* Resultados do monitor, protegidos por mutex_resultado */
static struct {
	uint32_t amostras;
	uint32_t viol_abertos;    /* A */
	uint32_t viol_ped_duas;   /* B */
	int64_t primeira_viol;    /* tick da primeira violação; -1 sem violação */
	struct janela todos_vermelho;   /* C */
	struct janela ped_apagado;      /* D */
	struct janela carros_apagado;   /* E */
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

static void t_monitor(void *p1, void *p2, void *p3)
{
	bool carros_acenderam = false;
	bool ped_acenderam = false;

	while (1) {
		/* 1 = aceso, qualquer que seja a polaridade do pino (vem do devicetree) */
		bool cvm = gpio_pin_get_dt(&carro_vermelho) == 1;
		bool cvd = gpio_pin_get_dt(&carro_verde) == 1;
		bool pvm = gpio_pin_get_dt(&ped_vermelho) == 1;
		bool pvd = gpio_pin_get_dt(&ped_verde) == 1;
		int64_t agora = k_uptime_ticks();

		bool carros_fechados = cvm && !cvd;   /* vermelho fixo; vermelho + verde é o amarelo */

		carros_acenderam |= cvm || cvd;
		ped_acenderam |= pvm || pvd;
		bool depois_do_boot = carros_acenderam && ped_acenderam;

		k_mutex_lock(&mutex_resultado, K_FOREVER);
		res.amostras++;
		if (pvd && !carros_fechados) {
			res.viol_abertos++;
		}
		if (pvd && pvm) {
			res.viol_ped_duas++;
		}
		if (res.primeira_viol < 0 && pvd && (!carros_fechados || pvm)) {
			res.primeira_viol = agora;
		}
		janela_amostra(&res.todos_vermelho, carros_fechados && pvm && !pvd, agora);
		janela_amostra(&res.ped_apagado, depois_do_boot && !pvm && !pvd, agora);
		janela_amostra(&res.carros_apagado, depois_do_boot && !cvm && !cvd, agora);
		k_mutex_unlock(&mutex_resultado);

		k_sleep(K_TICKS(1));
	}
}

/* Começa só depois de os dois semáforos configurarem os pinos, em main() */
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
	int64_t primeira = res.primeira_viol;
	struct janela c = res.todos_vermelho;
	struct janela d = res.ped_apagado;
	struct janela e = res.carros_apagado;
	k_mutex_unlock(&mutex_resultado);

	uint32_t agora = k_uptime_get_32();

	printk("%u ms | TI-01 resumo: %u amostras; violacoes: A=%u (pedestres verde com carros abertos), "
	       "B=%u (pedestres verde e vermelho juntos)\n", agora, amostras, viol_a, viol_b);
	if (primeira >= 0) {
		printk("%u ms | TI-01 resumo: PRIMEIRA VIOLACAO em %u ms\n", agora,
		       (uint32_t)k_ticks_to_ms_floor64(primeira));
	}
	printk("%u ms | TI-01 resumo: maiores janelas: todos no vermelho %u us (%u trocas), "
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

	printk("TI-01: semaforos de pedestres e carros integrados (ciclo de 8 s)\n");

	int ret = semaforo_pedestres_iniciar();   /* pedestres começam no vermelho */

	if (ret == 0) {
		/* false: sem o botão de travessia; o verde dos carros sempre dura 3 s */
		ret = semaforo_veiculo_iniciar(false);   /* carros começam no vermelho */
	}
	if (ret < 0) {
		printk("TI-01: erro %d ao iniciar os semaforos\n", ret);
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
