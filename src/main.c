/*
 * Semáforo de carros e pedestres em duas placas (Aula 07).
 *
 * O mesmo programa roda nas duas placas; o jumper no D8 define o papel
 * (ligado ao GND = mestre). Projeto e fiação: src/README.md.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"
#include "modo_noturno.h"
#include "semaforo_pedestres.h"
#include "semaforo_veiculo.h"

/*
 * Etapa 1: uma placa só, sem jumper e sem fios para a outra placa. Os carros
 * usam o LED RGB da placa e os pedestres, os LEDs do D6 e do D7.
 * Mude para false na etapa 2, com as duas placas ligadas.
 */
#define UMA_PLACA true

int main(void)
{
	int ret = enlace_iniciar(UMA_PLACA);

	if (ret == 0) {
		ret = modo_noturno_iniciar(enlace_sou_mestre());   /* botão só no mestre */
	}
	if (ret == 0) {
		ret = semaforo_pedestres_iniciar();
	}
	if (ret == 0) {
		printk("Semaforo iniciado como %s\n",
		       enlace_uma_placa()  ? "PLACA UNICA (etapa 1)" :
		       enlace_sou_mestre() ? "MESTRE" : "ESCRAVA");
		ret = semaforo_veiculo_iniciar();
	}

	if (ret < 0) {
		printk("Erro %d ao iniciar o semaforo\n", ret);
	}

	return ret;
}
