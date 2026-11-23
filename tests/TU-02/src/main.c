/*
 * TU-02: teste unitário do semáforo dos carros.
 *
 * Módulo sob teste: lib/semaforo_veiculo (threads t_carro_verde,
 * t_carro_amarelo e t_carro_vermelho, com mutex_carros). Os módulos de que
 * ele depende (enlace, modo noturno e pedestres) são trocados por dublês, em
 * src/dubles.c. O botão de travessia não é usado (testado no TS-02).
 *
 * O módulo roda o próprio ciclo, sem ajuda deste programa:
 *   vermelho 4 s -> verde 3 s -> amarelo 1 s -> vermelho ...
 *
 * Saída na serial (115200 bps), uma linha por troca de luz:
 *   tempo ms | carros VERMELHO
 *   tempo ms | carros VERDE
 *   tempo ms | carros AMARELO
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "semaforo_veiculo.h"

int main(void)
{
	printk("TU-02: semaforo dos carros (4 s vermelho, 3 s verde, 1 s amarelo)\n");

	/* false: sem o botão de travessia; o verde sempre dura 3 s */
	int ret = semaforo_veiculo_iniciar(false);

	if (ret < 0) {
		printk("TU-02: erro %d ao iniciar o semaforo dos carros\n", ret);
	}

	return ret;
}
