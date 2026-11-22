/*
 * Placa ESCRAVA: semáforo de carros e pedestres.
 *
 * Só abre os próprios carros quando o mestre (src/mestre.c, compilado com
 * -DPAPEL=mestre) dá a vez, e entra no modo noturno pelo bit NOTURNO do enlace. Sem o
 * mestre, fica com os carros no vermelho: falha segura.
 *
 * A lógica da escrava (esperar a vez, conferir de novo antes de abrir,
 * avisar quando fechou) fica em lib/semaforo_veiculo, a mesma biblioteca do
 * mestre; este arquivo só escolhe o papel. Projeto e fiação: src/README.md.
 *   west build -b frdm_kl25z -d build/escrava -- -DPAPEL=escrava
 *   west flash -d build/escrava
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"
#include "modo_noturno.h"
#include "semaforo_pedestres.h"
#include "semaforo_veiculo.h"

int main(void)
{
	int ret = enlace_iniciar(ENLACE_ESCRAVA);

	if (ret == 0) {
		ret = modo_noturno_iniciar(false);   /* o pedido vem do bit NOTURNO do enlace */
	}
	if (ret == 0) {
		ret = semaforo_pedestres_iniciar();
	}
	if (ret == 0) {
		printk("Semaforo iniciado como ESCRAVA\n");
		ret = semaforo_veiculo_iniciar(true);   /* o botão de travessia fica aqui */
	}

	if (ret < 0) {
		printk("Erro %d ao iniciar o semaforo\n", ret);
	}

	return ret;
}
