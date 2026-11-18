/*
 * Placa MESTRE: semáforo de carros e pedestres.
 *
 * Etapa 1: roda sozinha (UMA_PLACA true), sem fios para a outra placa.
 * Etapa 2: roda com a escrava (src/escrava.c). O mestre decide de quem é a
 * vez e tem o botão do modo noturno.
 *
 * A lógica fica nas bibliotecas de lib/, as mesmas nas duas placas; este
 * arquivo só escolhe o papel. Projeto e fiação: src/README.md.
 * Gravar: pio run -e mestre -t upload
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"
#include "modo_noturno.h"
#include "semaforo_pedestres.h"
#include "semaforo_veiculo.h"

/*
 * true:  etapa 1, a placa roda sozinha. Os carros usam o LED RGB da placa e
 *        os pedestres, os LEDs do D6 e do D7.
 * false: etapa 2, com a escrava. Com true, a escrava nunca recebe a vez e
 *        fica com os carros no vermelho.
 */
#define UMA_PLACA true

int main(void)
{
	int ret = enlace_iniciar(UMA_PLACA ? ENLACE_UNICA : ENLACE_MESTRE);

	if (ret == 0) {
		ret = modo_noturno_iniciar(true);   /* o botão do D9 fica no mestre */
	}
	if (ret == 0) {
		ret = semaforo_pedestres_iniciar();
	}
	if (ret == 0) {
		printk("Semaforo iniciado como %s\n",
		       UMA_PLACA ? "PLACA UNICA (etapa 1)" : "MESTRE");
		ret = semaforo_veiculo_iniciar();
	}

	if (ret < 0) {
		printk("Erro %d ao iniciar o semaforo\n", ret);
	}

	return ret;
}
