#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define TIME_1 1000
#define TIME_2 350

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
static const struct gpio_dt_spec led_vermelho = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#else
#error "Unsupported board: led2 devicetree alias is not defined"
#endif

void main(void)
{
    int ret;

    // Verifica se o device está pronto
    if (!gpio_is_ready_dt(&led_verde)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return;
    }

	if (!gpio_is_ready_dt(&led_azul)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return;
    }

	if (!gpio_is_ready_dt(&led_vermelho)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return;
    }

    // Configura o pino como saída
    ret = gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin\n", ret);
        return;
    }

	ret = gpio_pin_configure_dt(&led_azul, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin\n", ret);
        return;
    }

	ret = gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin\n", ret);
        return;
    }

    printk("LED blinking:\nGreen on %s pin %d\nBlue on %s pin %d\nRed on %s pin %d\n", led_verde.port->name, led_verde.pin, 
		led_azul.port->name, led_azul.pin, led_vermelho.port->name, led_vermelho.pin);

    while (1) {
        gpio_pin_toggle_dt(&led_verde);
        k_msleep(TIME_1);
		
		gpio_pin_toggle_dt(&led_verde);
		gpio_pin_toggle_dt(&led_azul);
        k_msleep(TIME_2);
        
		gpio_pin_toggle_dt(&led_azul);
    }
}
