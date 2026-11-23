/*
 * TU-01: teste unitário do semáforo de pedestres.
 *
 * Módulo sob teste: lib/semaforo_pedestres (threads t_ped_vermelho e
 * t_ped_verde, com mutex_pedestres). No firmware principal, quem manda os
 * pedestres abrirem e fecharem é o semáforo de veículos. Aqui, este programa
 * faz esse papel sozinho, com tempo fixo:
 *   - 4 s de vermelho -> semaforo_pedestres_liberar()  -> verde;
 *   - 4 s de verde    -> semaforo_pedestres_encerrar() -> vermelho.
 *
 * O módulo também pergunta se deve piscar (modo_pisca: modo noturno ou
 * ALERTA em vigor). Para isolar o teste, a função é substituída aqui por um
 * dublê que responde sempre "não": o pisca é testado no TS-01 e nos TA.
 *
 * Saída na serial (115200 bps), uma linha por troca de luz:
 *   tempo ms | pedestres VERMELHO
 *   tempo ms | pedestres VERDE
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "modo_noturno.h"
#include "semaforo_pedestres.h"

#define VERMELHO_MS 4000
#define VERDE_MS    4000

/* Dublê do lib/modo_noturno: as luzes nunca piscam neste teste */
bool modo_pisca(void)
{
	return false;
}

int main(void)
{
	int ret = semaforo_pedestres_iniciar();   /* começa no vermelho */

	if (ret < 0) {
		printk("TU-01: erro %d ao iniciar o semaforo de pedestres\n", ret);
		return ret;
	}
	printk("TU-01: semaforo de pedestres (4 s vermelho, 4 s verde)\n");

	/* Marca de tempo fixa: os atrasos de cada troca não se acumulam */
	int64_t proxima = k_uptime_get();

	while (1) {
		proxima += VERMELHO_MS;
		k_sleep(K_TIMEOUT_ABS_MS(proxima));
		semaforo_pedestres_liberar();      /* vermelho -> verde */

		proxima += VERDE_MS;
		k_sleep(K_TIMEOUT_ABS_MS(proxima));
		semaforo_pedestres_encerrar();     /* verde -> vermelho; retorna com o vermelho aceso */
	}

	return 0;
}
