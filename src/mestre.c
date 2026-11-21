/*
 * Placa MESTRE: semáforo de carros e pedestres.
 *
 * Etapa 2: roda com a escrava (src/escrava.c). O mestre decide de quem é a
 * vez e tem o botão do modo noturno.
 *   west build -b frdm_kl25z -d build/mestre -- -DPAPEL=mestre
 *   west flash -d build/mestre
 * Etapa 1: roda sozinho, sem fios para a outra placa.
 *   west build -b frdm_kl25z -d build/placa_unica -- -DPAPEL=placa_unica
 *   west flash -d build/placa_unica
 *
 * A lógica fica nas bibliotecas de lib/, as mesmas nas duas placas; este
 * arquivo só escolhe o papel. Projeto e fiação: src/README.md.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"
#include "modo_noturno.h"
#include "semaforo_pedestres.h"
#include "semaforo_veiculo.h"

/*
 * Vem do CMakeLists.txt, e não deve ser editado aqui:
 *   1 com -DPAPEL=placa_unica: etapa 1, a placa roda sozinha;
 *   0 com -DPAPEL=mestre: etapa 2, com a escrava.
 * Uma placa gravada como placa_unica e ligada à escrava nunca dá a vez, e a
 * escrava fica com os carros no vermelho.
 */
#ifndef UMA_PLACA
#define UMA_PLACA 0
#endif

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
