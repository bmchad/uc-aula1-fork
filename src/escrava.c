/*
 * Placa ESCRAVA: semáforo de carros e pedestres.
 *
 * Só abre os próprios carros quando o mestre (src/mestre.c com UMA_PLACA
 * false) dá a vez, e entra no modo noturno pelo fio NOTURNO. Sem o mestre,
 * fica com os carros no vermelho: falha segura.
 *
 * A lógica da escrava (esperar a vez, conferir de novo antes de abrir,
 * avisar quando fechou) fica em lib/semaforo_veiculo, a mesma biblioteca do
 * mestre; este arquivo só escolhe o papel. Projeto e fiação: src/README.md.
 * Gravar: pio run -e escrava -t upload
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
		ret = modo_noturno_iniciar(false);   /* o pedido vem do fio NOTURNO */
	}
	if (ret == 0) {
		ret = semaforo_pedestres_iniciar();
	}
	if (ret == 0) {
		printk("Semaforo iniciado como ESCRAVA\n");
		ret = semaforo_veiculo_iniciar();
	}

	if (ret < 0) {
		printk("Erro %d ao iniciar o semaforo\n", ret);
	}

	return ret;
}
