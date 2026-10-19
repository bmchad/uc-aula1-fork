#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define TICK_MS 50   /* mdc(1000, 350) = 50 ms */
#define TIME_1  1000 /* verde: alterna a cada 1 s */
#define TIME_2  350  /* azul: alterna a cada 350 ms (desafio) */

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

int main(void)
{
	if (led_init(&led_verde) < 0 ||
	    led_init(&led_azul) < 0 ||
	    led_init(&led_vermelho) < 0) {
		return 0;
	}

	printk("LED blinking:\nGreen on %s pin %d\nBlue on %s pin %d\nRed on %s pin %d\n",
	       led_verde.port->name, led_verde.pin,
	       led_azul.port->name, led_azul.pin,
	       led_vermelho.port->name, led_vermelho.pin);

	// Prazos absolutos: cada LED alterna no seu próprio ritmo, sem deriva
	int64_t next_verde = k_uptime_get() + TIME_1;
	int64_t next_azul = k_uptime_get() + TIME_2;

	while (1) {
		int64_t now = k_uptime_get();

		if (now >= next_verde) {
			gpio_pin_toggle_dt(&led_verde);
			next_verde += TIME_1; //1000
		}

		if (now >= next_azul) {
			gpio_pin_toggle_dt(&led_azul);
			next_azul += TIME_2; //350
		}

		k_msleep(TICK_MS); //mdc(350,1000)
	}

	return 0;
}
