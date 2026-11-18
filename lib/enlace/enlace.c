/*
 * Enlace entre as duas placas do semáforo.
 *
 * A interrupção da entrada (as duas bordas) só dá k_sem_give. Quem espera
 * sempre confere o nível de novo depois de acordar, então um aviso atrasado
 * ou repetido não causa decisão errada.
 */

#include "enlace.h"

#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec jumper =
	GPIO_DT_SPEC_GET(DT_NODELABEL(jumper_mestre), gpios);
static const struct gpio_dt_spec saida =
	GPIO_DT_SPEC_GET(DT_NODELABEL(saida_enlace), gpios);
static const struct gpio_dt_spec entrada =
	GPIO_DT_SPEC_GET(DT_NODELABEL(entrada_enlace), gpios);
static const struct gpio_dt_spec saida_noturno =
	GPIO_DT_SPEC_GET(DT_NODELABEL(saida_noturno), gpios);
static const struct gpio_dt_spec entrada_noturno =
	GPIO_DT_SPEC_GET(DT_NODELABEL(entrada_noturno), gpios);

static struct gpio_callback callback_entrada;

K_SEM_DEFINE(sem_mudanca, 0, 1);   /* ISR da entrada -> thread que espera */

static bool mestre;
static bool sozinha;   /* etapa 1: sem a outra placa */

static void isr_entrada(const struct device *dev, struct gpio_callback *cb,
			uint32_t pinos)
{
	k_sem_give(&sem_mudanca);
}

int enlace_iniciar(bool uma_placa)
{
	const struct gpio_dt_spec *pinos[] = {
		&jumper, &saida, &entrada, &saida_noturno, &entrada_noturno,
	};
	int ret;

	if (uma_placa) {
		/* Etapa 1: os pinos do enlace ficam como vieram do reset (desligados) */
		sozinha = true;
		mestre = true;
		return 0;
	}

	for (size_t i = 0; i < ARRAY_SIZE(pinos); i++) {
		if (!gpio_is_ready_dt(pinos[i])) {
			return -ENODEV;
		}
	}

	ret = gpio_pin_configure_dt(&jumper, GPIO_INPUT);
	if (ret < 0) {
		return ret;
	}
	k_busy_wait(100);   /* deixa o pull-up estabilizar antes de ler */
	mestre = gpio_pin_get_dt(&jumper) == 1;

	/* Estado inicial seguro: mestre sem dar a vez, escrava parada */
	ret = gpio_pin_configure_dt(&saida, mestre ? GPIO_OUTPUT_INACTIVE
						   : GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&saida_noturno, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&entrada_noturno, GPIO_INPUT);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&entrada, GPIO_INPUT);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&entrada, GPIO_INT_EDGE_BOTH);
	if (ret < 0) {
		return ret;
	}

	gpio_init_callback(&callback_entrada, isr_entrada, BIT(entrada.pin));
	return gpio_add_callback(entrada.port, &callback_entrada);
}

bool enlace_sou_mestre(void)
{
	return mestre;
}

bool enlace_uma_placa(void)
{
	return sozinha;
}

void enlace_publicar(bool valor)
{
	if (sozinha) {
		return;
	}
	gpio_pin_set_dt(&saida, valor);
}

bool enlace_ler(void)
{
	return gpio_pin_get_dt(&entrada) == 1;
}

int enlace_esperar(bool nivel, k_timeout_t timeout)
{
	k_timepoint_t fim = sys_timepoint_calc(timeout);

	while (enlace_ler() != nivel) {
		if (k_sem_take(&sem_mudanca, sys_timepoint_timeout(fim)) != 0) {
			return enlace_ler() == nivel ? 0 : -EAGAIN;
		}
	}

	return 0;
}

bool enlace_aguardar_mudanca(k_timeout_t timeout)
{
	k_sem_reset(&sem_mudanca);
	return k_sem_take(&sem_mudanca, timeout) == 0;
}

void enlace_publicar_noturno(bool ligado)
{
	if (sozinha) {
		return;
	}
	gpio_pin_set_dt(&saida_noturno, ligado);
}

bool enlace_ler_noturno(void)
{
	return gpio_pin_get_dt(&entrada_noturno) == 1;
}
