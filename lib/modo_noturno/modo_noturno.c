/*
 * Modo noturno do semáforo.
 *
 * Fluxo de um toque no botão (só no mestre):
 *   1. A interrupção do pino (borda para ativo) só dá k_sem_give: uma ISR não
 *      pode travar mutex.
 *   2. A thread do botão acorda, espera DEBOUNCE_MS e confere se o pino ainda
 *      está ativo. Um fio encostado no GND repica muito, e alternar o pedido
 *      não pode acontecer duas vezes por toque.
 *   3. Se o toque é válido, alterna o pedido com o mutex travado.
 *   4. Espera o fio ser solto, para um toque longo não alternar de novo, e
 *      descarta os k_sem_give gerados pelos repiques.
 */

#include "modo_noturno.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"

#define DEBOUNCE_MS      50
#define LEITURA_SOLTO_MS 20
#define TAMANHO_PILHA    768
#define PRIORIDADE_BOTAO 4   /* acima das threads dos LEDs: responde rápido */

static const struct gpio_dt_spec botao =
	GPIO_DT_SPEC_GET(DT_NODELABEL(botao_noturno), gpios);
static struct gpio_callback callback_botao;

K_SEM_DEFINE(sem_toque, 0, 1);   /* ISR do botão -> thread do botão */
K_MUTEX_DEFINE(mutex_modo);      /* protege pedido_botao, ativo, alerta e registrado */

static bool com_botao;
static bool pedido_botao;
static bool ativo;               /* modo noturno efetivo */
static bool alerta;              /* modo ALERTA efetivo */
static const char *registrado = "NORMAL";   /* último modo escrito na serial */

/*
 * Escreve o modo em vigor na serial, só quando ele muda. O ALERTA tem
 * prioridade: desligar o noturno durante o ALERTA não escreve "NORMAL".
 * Chamar com mutex_modo travado.
 */
static void registra_modo(void)
{
	const char *nome = alerta ? "ALERTA" : ativo ? "NOTURNO" : "NORMAL";

	if (nome != registrado) {
		registrado = nome;
		printk("%u ms | modo %s\n", k_uptime_get_32(), nome);
	}
}

static void isr_botao(const struct device *dev, struct gpio_callback *cb,
		      uint32_t pinos)
{
	k_sem_give(&sem_toque);
}

static void thread_botao(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&sem_toque, K_FOREVER);
		k_msleep(DEBOUNCE_MS);

		if (gpio_pin_get_dt(&botao) == 1) {
			k_mutex_lock(&mutex_modo, K_FOREVER);
			pedido_botao = !pedido_botao;
			bool pedido = pedido_botao;
			k_mutex_unlock(&mutex_modo);

			printk("%u ms | botao noturno: pedido %s\n", k_uptime_get_32(),
			       pedido ? "NOTURNO" : "NORMAL");

			while (gpio_pin_get_dt(&botao) == 1) {
				k_msleep(LEITURA_SOLTO_MS);
			}
			k_msleep(DEBOUNCE_MS);
		}

		k_sem_reset(&sem_toque);
	}
}

/* Na escrava fica bloqueada para sempre: a ISR do botão nunca é instalada */
K_THREAD_DEFINE(id_thread_botao_noturno, TAMANHO_PILHA, thread_botao,
		NULL, NULL, NULL, PRIORIDADE_BOTAO, 0, 0);

int modo_noturno_iniciar(bool usar_botao)
{
	int ret;

	com_botao = usar_botao;
	if (!com_botao) {
		return 0;
	}

	if (!gpio_is_ready_dt(&botao)) {
		return -ENODEV;
	}

	/* Pull-up e ativo em nível baixo vêm do devicetree */
	ret = gpio_pin_configure_dt(&botao, GPIO_INPUT);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&botao, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		return ret;
	}

	gpio_init_callback(&callback_botao, isr_botao, BIT(botao.pin));
	return gpio_add_callback(botao.port, &callback_botao);
}

bool modo_noturno_pedido(void)
{
	if (!com_botao) {
		return enlace_ler_noturno();
	}

	k_mutex_lock(&mutex_modo, K_FOREVER);
	bool pedido = pedido_botao;
	k_mutex_unlock(&mutex_modo);

	return pedido;
}

void modo_noturno_aplicar(bool ligado)
{
	k_mutex_lock(&mutex_modo, K_FOREVER);
	ativo = ligado;
	registra_modo();
	k_mutex_unlock(&mutex_modo);

	if (com_botao) {
		enlace_publicar_noturno(ligado);
	}
}

bool modo_noturno_ativo(void)
{
	k_mutex_lock(&mutex_modo, K_FOREVER);
	bool ligado = ativo;
	k_mutex_unlock(&mutex_modo);

	return ligado;
}

bool modo_alerta_pedido(void)
{
	return enlace_falhou();
}

void modo_alerta_aplicar(bool ligado)
{
	k_mutex_lock(&mutex_modo, K_FOREVER);
	alerta = ligado;
	registra_modo();
	k_mutex_unlock(&mutex_modo);
}

bool modo_alerta_ativo(void)
{
	k_mutex_lock(&mutex_modo, K_FOREVER);
	bool ligado = alerta;
	k_mutex_unlock(&mutex_modo);

	return ligado;
}

bool modo_pisca(void)
{
	k_mutex_lock(&mutex_modo, K_FOREVER);
	bool pisca = ativo || alerta;
	k_mutex_unlock(&mutex_modo);

	return pisca;
}
