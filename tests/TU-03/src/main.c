/*
 * TU-03: teste unitário da comunicação UART entre as placas.
 *
 * Módulo sob teste: lib/enlace (quadros de 4 bytes pela UART2, TX no D12 e
 * RX no D11, ligados em cruz entre as placas). Nenhum semáforo roda: as duas
 * placas só trocam o SINAL do enlace.
 *
 * Cada placa:
 *   - envio (main): a cada ENVIO_MS (1 s) inverte o próprio SINAL e registra
 *     `TU-03 enviado SINAL=x`;
 *   - recepção (t_recepcao): confere o SINAL da outra placa a cada mudança
 *     avisada pelo enlace (e no máximo a cada CONSULTA_MS) e registra cada
 *     troca como `TU-03 recebido SINAL=y (intervalo N ms)`. Como a outra
 *     placa inverte o SINAL a cada 1 s, o intervalo deve ficar perto de
 *     1000 ms; um valor perto de 2000 ms indica uma troca perdida;
 *   - resumo: a cada RESUMO_MS (10 s) registra quantas trocas recebeu e
 *     quantas ficaram fora da faixa esperada.
 * O próprio enlace registra `enlace conectado` quando começam a chegar
 * quadros válidos, e `enlace perdido` quando eles param por mais de 200 ms.
 *
 * Uma placa é gravada como mestre (-DPAPEL=mestre) e a outra como escrava
 * (-DPAPEL=escrava): o enlace descarta quadros vindos do mesmo papel.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"

#ifndef TU03_MESTRE
#define TU03_MESTRE 1
#endif

#define ENVIO_MS        1000   /* intervalo entre as inversões do SINAL desta placa */
#define CONSULTA_MS     100    /* recepção: intervalo máximo entre conferências */
#define RESUMO_MS       10000  /* intervalo entre os resumos */
#define INTERVALO_MIN_MS 900   /* faixa aceita entre trocas recebidas */
#define INTERVALO_MAX_MS 1100
#define TAMANHO_PILHA   1024
/* A mesma prioridade das threads do enlace: as linhas da serial não se misturam */
#define PRIORIDADE      5

static atomic_t trocas;        /* trocas do SINAL da outra placa recebidas */
static atomic_t fora_da_faixa; /* trocas com intervalo fora de 900 a 1100 ms */

static void t_recepcao(void *p1, void *p2, void *p3)
{
	bool anterior = enlace_ler();
	int64_t ultima = 0;   /* 0: ainda não houve troca */

	while (1) {
		enlace_aguardar_mudanca(K_MSEC(CONSULTA_MS));

		bool atual = enlace_ler();

		if (atual == anterior) {
			continue;
		}
		anterior = atual;

		int64_t agora = k_uptime_get();

		atomic_inc(&trocas);
		if (ultima == 0) {
			printk("%u ms | TU-03 recebido SINAL=%d (primeira troca)\n",
			       (uint32_t)agora, atual);
		} else {
			int64_t intervalo = agora - ultima;
			bool ok = intervalo >= INTERVALO_MIN_MS && intervalo <= INTERVALO_MAX_MS;

			if (!ok) {
				atomic_inc(&fora_da_faixa);
			}
			printk("%u ms | TU-03 recebido SINAL=%d (intervalo %u ms%s)\n",
			       (uint32_t)agora, atual, (uint32_t)intervalo,
			       ok ? "" : ", FORA DA FAIXA");
		}
		ultima = agora;
	}
}

/* Começa só depois de enlace_iniciar(), em main() */
K_THREAD_DEFINE(id_t_recepcao, TAMANHO_PILHA, t_recepcao,
		NULL, NULL, NULL, PRIORIDADE, 0, SYS_FOREVER_MS);

int main(void)
{
	/* main na mesma prioridade das outras threads, pelo mesmo motivo */
	k_thread_priority_set(k_current_get(), PRIORIDADE);

	int ret = enlace_iniciar(TU03_MESTRE ? ENLACE_MESTRE : ENLACE_ESCRAVA);

	if (ret < 0) {
		printk("TU-03: erro %d ao iniciar o enlace\n", ret);
		return ret;
	}
	printk("TU-03: enlace UART como %s (SINAL invertido a cada %d ms)\n",
	       TU03_MESTRE ? "MESTRE" : "ESCRAVA", ENVIO_MS);

	k_thread_start(id_t_recepcao);

	/* Estado inicial do enlace: VEZ = 0 no mestre, PARADO = 1 na escrava */
	bool sinal = !TU03_MESTRE;
	int64_t proximo = k_uptime_get();
	int64_t proximo_resumo = proximo + RESUMO_MS;

	while (1) {
		/* Marca de tempo fixa: os atrasos de cada envio não se acumulam */
		proximo += ENVIO_MS;
		k_sleep(K_TIMEOUT_ABS_MS(proximo));

		sinal = !sinal;
		enlace_publicar(sinal);
		printk("%u ms | TU-03 enviado SINAL=%d\n", k_uptime_get_32(), sinal);

		if (proximo >= proximo_resumo) {
			proximo_resumo += RESUMO_MS;
			printk("%u ms | TU-03 resumo: %d trocas recebidas, %d fora da faixa\n",
			       k_uptime_get_32(), (int)atomic_get(&trocas),
			       (int)atomic_get(&fora_da_faixa));
		}
	}

	return 0;
}
