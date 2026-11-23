/*
 * Semáforo de pedestres.
 *
 * Ciclo das threads (cadeia de k_sem, uma luz por vez sob mutex_pedestres):
 *   t_ped_vermelho: acende, avisa `encerrado` e espera `liberado`. Nos modos
 *                   NOTURNO e ALERTA, pisca enquanto espera.
 *   t_ped_verde:    acende e espera `encerrar`.
 */

#include "semaforo_pedestres.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "modo_noturno.h"

#define PISCA_MS       1000
#define TAMANHO_PILHA  768
#define PRIORIDADE_LED 5

static const struct gpio_dt_spec led_vermelho =
	GPIO_DT_SPEC_GET(DT_NODELABEL(ped_vermelho), gpios);
static const struct gpio_dt_spec led_verde =
	GPIO_DT_SPEC_GET(DT_NODELABEL(ped_verde), gpios);

K_MUTEX_DEFINE(mutex_pedestres);
K_SEM_DEFINE(vez_ped_vermelho, 0, 1);
K_SEM_DEFINE(vez_ped_verde, 0, 1);
K_SEM_DEFINE(liberado, 0, 1);    /* carros parados: pedestres podem atravessar */
K_SEM_DEFINE(encerrar, 0, 1);    /* pedido para fechar a travessia */
K_SEM_DEFINE(encerrado, 0, 1);   /* vermelho dos pedestres aceso */

static void t_ped_vermelho(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&vez_ped_vermelho, K_FOREVER);
		k_mutex_lock(&mutex_pedestres, K_FOREVER);

		gpio_pin_set_dt(&led_vermelho, 1);
		printk("%u ms | pedestres VERMELHO\n", k_uptime_get_32());
		k_sem_give(&encerrado);

		bool aceso = true;

		while (k_sem_take(&liberado, K_MSEC(PISCA_MS)) != 0) {
			/* Nos modos NOTURNO e ALERTA o vermelho pisca; fora deles fica aceso */
			bool novo = modo_pisca() ? !aceso : true;

			if (novo != aceso) {
				aceso = novo;
				gpio_pin_set_dt(&led_vermelho, aceso);
				/* Cada troca vai para a serial: é a evidência do RF-14 */
				printk("%u ms | pedestres VERMELHO %s\n", k_uptime_get_32(),
				       aceso ? "aceso" : "apagado");
			}
		}

		gpio_pin_set_dt(&led_vermelho, 0);
		k_mutex_unlock(&mutex_pedestres);
		k_sem_give(&vez_ped_verde);
	}
}

static void t_ped_verde(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&vez_ped_verde, K_FOREVER);
		k_mutex_lock(&mutex_pedestres, K_FOREVER);

		gpio_pin_set_dt(&led_verde, 1);
		printk("%u ms | pedestres VERDE\n", k_uptime_get_32());
		k_sem_take(&encerrar, K_FOREVER);

		gpio_pin_set_dt(&led_verde, 0);
		k_mutex_unlock(&mutex_pedestres);
		k_sem_give(&vez_ped_vermelho);
	}
}

/* As duas começam bloqueadas: só rodam depois de semaforo_pedestres_iniciar() */
K_THREAD_DEFINE(id_t_ped_vermelho, TAMANHO_PILHA, t_ped_vermelho,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);
K_THREAD_DEFINE(id_t_ped_verde, TAMANHO_PILHA, t_ped_verde,
		NULL, NULL, NULL, PRIORIDADE_LED, 0, 0);

int semaforo_pedestres_iniciar(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led_vermelho) || !gpio_is_ready_dt(&led_verde)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return ret;
	}

	k_sem_give(&vez_ped_vermelho);   /* começa no vermelho */
	return 0;
}

void semaforo_pedestres_liberar(void)
{
	k_sem_give(&liberado);
}

void semaforo_pedestres_encerrar(void)
{
	/* Descarta o aviso do vermelho anterior: só vale o que vier depois do pedido */
	k_sem_reset(&encerrado);
	k_sem_give(&encerrar);
	k_sem_take(&encerrado, K_FOREVER);
}
