#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define TIME_1  1000 /* verde: alterna a cada 1 s */
#define TIME_2  350  /* azul: alterna a cada 350 ms (desafio) */
#define TIME_3 500 /* apenas por definir */

// Define o LED usando Device Tree
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/*
led0	verde	PTB19
led1	azul	PTD1
led2	vermelho	PTB18
*/

// Verifica se o LED está definido no Device Tree
#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
static const struct gpio_dt_spec led_verde = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#else
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
static const struct gpio_dt_spec led_azul = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#else
#error "Unsupported board: led1 devicetree alias is not defined"
#endif

#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
static const struct gpio_dt_spec led_vermelho = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
#else
#error "Unsupported board: led2 devicetree alias is not defined"
#endif

// Verifica se o device está pronto e configura o pino como saída, apagado
static int led_init(const struct gpio_dt_spec *led)
{
	int ret;

	if (!gpio_is_ready_dt(led)) {
		printk("Error: LED device %s is not ready\n", led->port->name);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Error %d: failed to configure LED pin %d\n", ret, led->pin);
		return ret;
	}

	return 0;
}

// Cada thread configura o próprio LED antes do laço: as threads do
// K_THREAD_DEFINE com atraso 0 ficam prontas no boot, antes da main, então
// não dá para depender da main ter configurado os pinos.
static void pisca_verde_thread(void *arg1, void *arg2, void *arg3)
{
	if (led_init(&led_verde) < 0) {
		return;
	}

	while (1) {
		gpio_pin_toggle_dt(&led_verde);
		k_msleep(TIME_1);
	}
}

static void pisca_azul_thread(void *arg1, void *arg2, void *arg3)
{
	if (led_init(&led_azul) < 0) {
		return;
	}

	while (1) {
		gpio_pin_toggle_dt(&led_azul);
		k_msleep(TIME_2);
	}
}

static void pisca_vermelho_thread(void *arg1, void *arg2, void *arg3)
{
	if (led_init(&led_vermelho) < 0) {
		return;
	}

	while (1) {
		gpio_pin_toggle_dt(&led_vermelho);
		k_msleep(TIME_3);
	}
}

#define TAMANHO_PILHA 512
#define PRIORIDADE_1 1
#define PRIORIDADE_2 2
#define PRIORIDADE_3 3

K_THREAD_DEFINE(id_thread_verde, TAMANHO_PILHA, 
	pisca_verde_thread, NULL, NULL, NULL, PRIORIDADE_1, 0, 0);

K_THREAD_DEFINE(id_thread_azul, TAMANHO_PILHA, 
	pisca_azul_thread, NULL, NULL, NULL, PRIORIDADE_2, 0, 0);

K_THREAD_DEFINE(id_thread_vermelho, TAMANHO_PILHA, 
	pisca_vermelho_thread, NULL, NULL, NULL, PRIORIDADE_3, 0, 0);
	


int main(void)
{
	// Os LEDs são configurados pelas próprias threads; a main só informa os pinos
	printk("LED blinking:\nGreen on %s pin %d\nBlue on %s pin %d\nRed on %s pin %d\n",
	       led_verde.port->name, led_verde.pin,
	       led_azul.port->name, led_azul.pin,
	       led_vermelho.port->name, led_vermelho.pin);

	return 0;
}
