/*
 * TS-02: teste de sistema do botão de travessia, numa placa só.
 *
 * Módulos do sistema: lib/semaforo_veiculo (com o botão de travessia no D10)
 * e lib/semaforo_pedestres. O enlace (placa única) e o modo noturno (sempre
 * desligado) são dublês (src/dubles.c).
 *
 * Comportamento esperado:
 *   - toque com os carros no VERMELHO ou no AMARELO: nada muda; a fase dura o
 *     tempo normal (vermelho 4 s, amarelo 1 s), e o verde seguinte também
 *     (3 s), porque o verde descarta os toques antigos ao começar;
 *   - toque com os carros no VERDE: o verde acaba, os carros passam pelo
 *     amarelo (1 s) e fecham, e o verde dos pedestres acende. O verde dura
 *     pelo menos 1 s (VERDE_MIN_MS): um toque no primeiro segundo encerra o
 *     verde em 1 s; um toque depois disso encerra o verde na hora.
 *
 * Além das linhas que os módulos já mandam para a serial (cada troca de luz,
 * `botao travessia: toque` e `carros travessia pedida: verde encurtado`),
 * este programa tem um MONITOR: uma thread que lê os quatro LEDs e o botão,
 * espera 1 tick (0,1 ms) e lê de novo. Com o tempo da própria leitura, sai
 * uma amostra a cada 0,2 ms, mais ou menos.
 *   - Toques: cada vez que o botão passa de solto para apertado, ignorando
 *     novas bordas por TRAVA_MS (o repique do fio).
 *   - Fases dos carros: ao fim de cada fase (VERMELHO, VERDE ou AMARELO), a
 *     duração dela, os toques que caíram nela e um veredito, numa linha
 *     `TS-02 fase ...`:
 *       VERMELHO: 4000 ms (± 1%), com ou sem toques;
 *       AMARELO:  1000 ms (± 1%), com ou sem toques;
 *       VERDE sem toque: 3000 ms (± 1%);
 *       VERDE com toque: termina em max(primeiro toque, 1000 ms), com até
 *         FOLGA_MS de atraso.
 *   - Violações (devem ser 0):
 *       A. pedestres no verde sem os carros no vermelho fixo;
 *       B. pedestres com o verde e o vermelho acesos juntos.
 * A cada RESUMO_MS (10 s), main() manda um resumo para a serial.
 *
 * O monitor lê o nível dos pinos (o que o firmware comanda nos LEDs), e não
 * a luz em si: o vídeo continua sendo a evidência do que aparece nos LEDs.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "semaforo_pedestres.h"
#include "semaforo_veiculo.h"

#define RESUMO_MS          10000
#define TRAVA_MS           200    /* toques mais próximos que isso contam como um */
#define FOLGA_MS           50     /* atraso aceito para o verde acabar depois do toque */
#define TOLERANCIA_PCT     1      /* tolerância das fases com tempo fixo */
#define VERDE_MS           3000   /* os mesmos tempos de lib/semaforo_veiculo */
#define VERDE_MIN_MS       1000
#define AMARELO_MS         1000
#define VERMELHO_MS        4000
#define TAMANHO_PILHA      1024
#define PRIORIDADE_MONITOR 4   /* acima das luzes (5): amostra até no meio de uma troca */
#define PRIORIDADE_RESUMO  5   /* a mesma das luzes: as linhas da serial não se misturam */

/* Os mesmos pinos que os módulos usam (frdm_kl25z.overlay e aliases da placa) */
static const struct gpio_dt_spec carro_vermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec carro_verde = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec ped_vermelho = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_vermelho), gpios);
static const struct gpio_dt_spec ped_verde = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_verde), gpios);
static const struct gpio_dt_spec botao = GPIO_DT_SPEC_GET(DT_NODELABEL(botao_travessia), gpios);

enum fase { FASE_NENHUMA, FASE_VERMELHO, FASE_VERDE, FASE_AMARELO };

static const char *const nome_fase[] = {"?", "VERMELHO", "VERDE", "AMARELO"};

/* Uma fase dos carros que terminou: o monitor manda, main() imprime */
struct registro_fase {
	enum fase fase;
	uint32_t inicio_ms;      /* tempo da placa em que a fase começou */
	uint32_t duracao_ms;
	uint32_t toques;
	uint32_t primeiro_ms;    /* primeiro toque, contado do início da fase */
};

K_MSGQ_DEFINE(fila_fases, sizeof(struct registro_fase), 16, 4);

/* Contadores do monitor, protegidos por mutex_resultado */
static struct {
	uint32_t amostras;
	uint32_t toques;
	uint32_t viol_abertos;    /* A */
	uint32_t viol_ped_duas;   /* B */
	int64_t primeira_viol;    /* tick da primeira violação; -1 sem violação */
	uint32_t fases_perdidas;  /* fila cheia */
} res;

K_MUTEX_DEFINE(mutex_resultado);

static enum fase fase_dos_carros(bool cvm, bool cvd)
{
	if (cvm && cvd) {
		return FASE_AMARELO;
	}
	if (cvd) {
		return FASE_VERDE;
	}
	if (cvm) {
		return FASE_VERMELHO;
	}
	return FASE_NENHUMA;   /* entre duas luzes: a fase anterior continua */
}

static void t_monitor(void *p1, void *p2, void *p3)
{
	enum fase fase = FASE_NENHUMA;
	int64_t inicio_fase = 0;
	uint32_t toques_fase = 0;
	int64_t primeiro_toque = 0;
	bool apertado_antes = false;
	int64_t ultimo_toque = INT64_MIN / 2;

	while (1) {
		/* 1 = aceso/apertado, qualquer que seja a polaridade do pino */
		bool cvm = gpio_pin_get_dt(&carro_vermelho) == 1;
		bool cvd = gpio_pin_get_dt(&carro_verde) == 1;
		bool pvm = gpio_pin_get_dt(&ped_vermelho) == 1;
		bool pvd = gpio_pin_get_dt(&ped_verde) == 1;
		bool apertado = gpio_pin_get_dt(&botao) == 1;
		int64_t agora = k_uptime_ticks();
		bool carros_fechados = cvm && !cvd;   /* vermelho fixo; vermelho + verde é o amarelo */

		/* Fim de fase: uma luz nova acendeu */
		enum fase nova = fase_dos_carros(cvm, cvd);

		if (nova != FASE_NENHUMA && nova != fase) {
			if (fase != FASE_NENHUMA) {
				struct registro_fase r = {
					.fase = fase,
					.inicio_ms = (uint32_t)k_ticks_to_ms_floor64(inicio_fase),
					.duracao_ms = (uint32_t)k_ticks_to_ms_floor64(agora - inicio_fase),
					.toques = toques_fase,
					.primeiro_ms = (uint32_t)k_ticks_to_ms_floor64(primeiro_toque - inicio_fase),
				};

				if (k_msgq_put(&fila_fases, &r, K_NO_WAIT) != 0) {
					k_mutex_lock(&mutex_resultado, K_FOREVER);
					res.fases_perdidas++;
					k_mutex_unlock(&mutex_resultado);
				}
			}
			fase = nova;
			inicio_fase = agora;
			toques_fase = 0;
		}

		/* Toque: borda de solto para apertado, fora da trava do repique */
		bool toque = apertado && !apertado_antes &&
			     agora - ultimo_toque >= k_ms_to_ticks_ceil64(TRAVA_MS);

		apertado_antes = apertado;
		if (toque) {
			ultimo_toque = agora;
			if (toques_fase == 0) {
				primeiro_toque = agora;
			}
			toques_fase++;
		}

		k_mutex_lock(&mutex_resultado, K_FOREVER);
		res.amostras++;
		res.toques += toque ? 1 : 0;
		if (pvd && !carros_fechados) {
			res.viol_abertos++;
		}
		if (pvd && pvm) {
			res.viol_ped_duas++;
		}
		if (res.primeira_viol < 0 && pvd && (!carros_fechados || pvm)) {
			res.primeira_viol = agora;
		}
		k_mutex_unlock(&mutex_resultado);

		k_sleep(K_TICKS(1));
	}
}

/* Começa só depois de os semáforos configurarem os pinos, em main() */
K_THREAD_DEFINE(id_t_monitor, TAMANHO_PILHA, t_monitor,
		NULL, NULL, NULL, PRIORIDADE_MONITOR, 0, SYS_FOREVER_MS);

static uint32_t fases_ok;
static uint32_t fases_fora;
static uint32_t verdes_encurtados;

static bool dentro(uint32_t medido, uint32_t esperado)
{
	uint32_t folga = esperado * TOLERANCIA_PCT / 100;

	return medido + folga >= esperado && medido <= esperado + folga;
}

/* Imprime uma fase que terminou, com o veredito */
static void imprime_fase(const struct registro_fase *r)
{
	uint32_t esperado;
	bool ok;
	char toques[48] = "sem toque";

	if (r->toques > 0) {
		snprintk(toques, sizeof(toques), "%u toque(s), o primeiro aos %u ms",
			 r->toques, r->primeiro_ms);
	}

	if (r->fase == FASE_VERDE && r->toques > 0 && r->primeiro_ms < VERDE_MS) {
		/* Verde encurtado: acaba no primeiro toque, mas não antes de 1 s */
		esperado = MAX(r->primeiro_ms, VERDE_MIN_MS);
		ok = r->duracao_ms + 1 >= esperado && r->duracao_ms <= esperado + FOLGA_MS;
		verdes_encurtados += ok ? 1 : 0;
		printk("%u ms | TS-02 fase VERDE (inicio %u ms): %u ms, %s -> esperado encurtar para "
		       "%u ms: %s\n", k_uptime_get_32(), r->inicio_ms, r->duracao_ms, toques, esperado,
		       ok ? "OK" : "FORA DO ESPERADO");
	} else {
		esperado = r->fase == FASE_VERDE ? VERDE_MS :
			   r->fase == FASE_AMARELO ? AMARELO_MS : VERMELHO_MS;
		ok = dentro(r->duracao_ms, esperado);
		printk("%u ms | TS-02 fase %s (inicio %u ms): %u ms, %s -> esperado %u ms (+-%d%%): %s\n",
		       k_uptime_get_32(), nome_fase[r->fase], r->inicio_ms, r->duracao_ms, toques,
		       esperado, TOLERANCIA_PCT, ok ? "OK" : "FORA DO ESPERADO");
	}

	if (ok) {
		fases_ok++;
	} else {
		fases_fora++;
	}
}

static void imprime_resumo(void)
{
	k_mutex_lock(&mutex_resultado, K_FOREVER);
	uint32_t amostras = res.amostras;
	uint32_t toques = res.toques;
	uint32_t viol_a = res.viol_abertos;
	uint32_t viol_b = res.viol_ped_duas;
	int64_t primeira = res.primeira_viol;
	uint32_t perdidas = res.fases_perdidas;
	k_mutex_unlock(&mutex_resultado);

	uint32_t agora = k_uptime_get_32();

	printk("%u ms | TS-02 resumo: %u amostras, %u toques; fases: %u OK, %u fora do esperado, "
	       "%u verdes encurtados; violacoes: A=%u (pedestres verde com carros abertos), "
	       "B=%u (pedestres verde e vermelho juntos)\n", agora, amostras, toques, fases_ok,
	       fases_fora, verdes_encurtados, viol_a, viol_b);
	if (primeira >= 0) {
		printk("%u ms | TS-02 resumo: PRIMEIRA VIOLACAO em %u ms\n", agora,
		       (uint32_t)k_ticks_to_ms_floor64(primeira));
	}
	if (perdidas > 0) {
		printk("%u ms | TS-02 resumo: %u fases nao impressas (fila cheia)\n", agora, perdidas);
	}
}

int main(void)
{
	/* main imprime as fases e o resumo: mesma prioridade das luzes */
	k_thread_priority_set(k_current_get(), PRIORIDADE_RESUMO);

	res.primeira_viol = -1;

	printk("TS-02: botao de travessia (D10); toque no verde encurta o verde (minimo 1 s), "
	       "no vermelho ou no amarelo nao muda nada\n");

	int ret = semaforo_pedestres_iniciar();   /* pedestres começam no vermelho */

	if (ret == 0) {
		/* true: configura o botão de travessia do D10 */
		ret = semaforo_veiculo_iniciar(true);   /* carros começam no vermelho */
	}
	if (ret < 0) {
		printk("TS-02: erro %d ao iniciar\n", ret);
		return ret;
	}

	k_thread_start(id_t_monitor);

	k_timepoint_t resumo = sys_timepoint_calc(K_MSEC(RESUMO_MS));

	while (1) {
		struct registro_fase r;

		if (k_msgq_get(&fila_fases, &r, sys_timepoint_timeout(resumo)) == 0) {
			imprime_fase(&r);
		} else {
			imprime_resumo();
			resumo = sys_timepoint_calc(K_MSEC(RESUMO_MS));
		}
	}

	return 0;
}
