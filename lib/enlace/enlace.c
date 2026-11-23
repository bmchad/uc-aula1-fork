/*
 * Enlace entre as duas placas do semáforo, pela UART2.
 *
 * Quadro de 4 bytes, de cada placa para a outra:
 *   [0] 0xA5                      início
 *   [1] bits                      SINAL, NOTURNO, MESTRE (papel de quem manda) e OUVE
 *   [2] contador                  aumenta a cada quadro (só para depuração)
 *   [3] ~(b0 + b1 + b2)           verificação
 *
 * Fluxo:
 *   - a interrupção da UART só copia cada byte recebido para uma fila
 *     (k_msgq_put sem espera), sem mutex e sem decidir nada;
 *   - t_enlace_rx monta os quadros, descarta os inválidos, guarda os bits da
 *     outra placa, calcula a falha do enlace e avisa quem espera (sem_mudanca);
 *   - t_enlace_tx manda o estado desta placa a cada PERIODO_MS e logo que
 *     ele muda (sem_envio), com o bit OUVE ligado enquanto recebe a outra.
 * Quem espera sempre confere o nível de novo depois de acordar, então um
 * aviso atrasado ou repetido não causa decisão errada.
 */

#include "enlace.h"

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>

#define BYTE_INICIO       0xA5
#define BIT_SINAL         BIT(0)   /* VEZ (mestre) ou PARADO (escrava) */
#define BIT_NOTURNO       BIT(1)   /* só o mestre manda */
#define BIT_MESTRE        BIT(2)   /* papel de quem mandou o quadro */
#define BIT_OUVE          BIT(3)   /* quem mandou está recebendo quadros válidos */
#define BITS_VALIDOS      (BIT_SINAL | BIT_NOTURNO | BIT_MESTRE | BIT_OUVE)
#define PERIODO_MS        50       /* reenvio do estado, mesmo sem mudança */
#define PRAZO_MS          200      /* sem quadro válido por esse tempo: enlace perdido */
#define INICIO_MS         500      /* tolerância do boot antes de declarar falha */
#define TAMANHO_FILA      64       /* bytes recebidos à espera da t_enlace_rx */
#define TAMANHO_PILHA     768
/*
 * Mesma prioridade das luzes: nenhuma interrompe a outra no meio de um printk,
 * e as linhas da serial não se misturam. Um quadro pode esperar alguns ms uma
 * luz terminar de imprimir; a fila guarda os bytes enquanto isso, e a guarda
 * de 50 ms do handshake cobre o atraso.
 */
#define PRIORIDADE_ENLACE 5

static const struct device *const uart = DEVICE_DT_GET(DT_ALIAS(enlace_uart));

K_SEM_DEFINE(sem_mudanca, 0, 1);   /* t_enlace_rx -> quem espera a outra placa */
K_SEM_DEFINE(sem_envio, 0, 1);     /* mudança local -> t_enlace_tx manda na hora */
K_MSGQ_DEFINE(fila_rx, 1, TAMANHO_FILA, 1);   /* ISR da UART -> t_enlace_rx */

static bool mestre;
static bool sozinha;        /* placa única: sem a outra placa */
static atomic_t local;      /* bits que esta placa manda (sem o OUVE) */
static atomic_t remoto;     /* bits da outra placa (0 sem enlace) */
static atomic_t conectado;  /* 1 enquanto chegam quadros válidos */
static atomic_t falha;      /* 1 com o enlace em falha: modo ALERTA */

static uint8_t verificacao(const uint8_t *q)
{
	return (uint8_t)~(q[0] + q[1] + q[2]);
}

static void isr_uart(const struct device *dev, void *dados)
{
	uint8_t byte;

	uart_irq_update(dev);
	while (uart_irq_rx_ready(dev) && uart_fifo_read(dev, &byte, 1) == 1) {
		/* Fila cheia: o byte se perde, e o quadro falha na verificação */
		(void)k_msgq_put(&fila_rx, &byte, K_NO_WAIT);
	}
}

/* Guarda os bits da outra placa; devolve true se algo mudou */
static bool atualiza(atomic_val_t bits, bool ok)
{
	bool mudou = atomic_set(&remoto, bits) != bits;
	bool estava = atomic_set(&conectado, ok) != 0;

	if (estava != ok) {
		printk("%u ms | enlace %s\n", k_uptime_get_32(), ok ? "conectado" : "perdido");
		mudou = true;
	}
	return mudou;
}

/*
 * O enlace está OK quando esta placa recebe a outra e a outra confirma (OUVE)
 * que recebe esta. Nos primeiros INICIO_MS depois do boot a falha ainda não é
 * declarada. Devolve true se a falha mudou.
 */
static bool atualiza_falha(int64_t inicio)
{
	bool ok = atomic_get(&conectado) && (atomic_get(&remoto) & BIT_OUVE);
	bool nova = !ok && (k_uptime_get() - inicio > INICIO_MS);

	if ((atomic_set(&falha, nova) != 0) == nova) {
		return false;
	}
	printk("%u ms | enlace %s\n", k_uptime_get_32(), nova ? "FALHA" : "OK");
	return true;
}

static void t_enlace_rx(void *p1, void *p2, void *p3)
{
	uint8_t q[4] = {0};
	int64_t inicio = k_uptime_get();
	int64_t ultimo_valido = 0;
	bool avisou_papel = false;

	while (1) {
		uint8_t byte;
		bool mudou = false;

		if (k_msgq_get(&fila_rx, &byte, K_MSEC(PERIODO_MS)) == 0) {
			memmove(q, q + 1, 3);
			q[3] = byte;

			if (q[0] == BYTE_INICIO && (q[1] & ~BITS_VALIDOS) == 0 &&
			    q[3] == verificacao(q)) {
				bool dele_mestre = (q[1] & BIT_MESTRE) != 0;

				if (dele_mestre != mestre) {
					ultimo_valido = k_uptime_get();
					mudou = atualiza(q[1] & (BIT_SINAL | BIT_NOTURNO | BIT_OUVE),
							 true);
				} else if (!avisou_papel) {
					/* Mesmo programa nas duas placas, ou TX ligado no próprio RX */
					printk("%u ms | enlace: quadro com o mesmo papel desta placa\n",
					       k_uptime_get_32());
					avisou_papel = true;
				}
				memset(q, 0, sizeof(q));   /* o próximo quadro começa do zero */
			}
		}

		if (atomic_get(&conectado) && k_uptime_get() - ultimo_valido > PRAZO_MS) {
			mudou |= atualiza(0, false);   /* estado seguro: os sinais da outra placa valem 0 */
		}

		mudou |= atualiza_falha(inicio);
		if (mudou) {
			k_sem_give(&sem_mudanca);
		}
	}
}

static void t_enlace_tx(void *p1, void *p2, void *p3)
{
	uint8_t contador = 0;

	while (1) {
		uint8_t bits = (uint8_t)atomic_get(&local);

		if (atomic_get(&conectado)) {
			bits |= BIT_OUVE;   /* avisa a outra placa que os quadros dela chegam */
		}

		uint8_t q[4] = {BYTE_INICIO, bits, contador++, 0};

		q[3] = verificacao(q);
		for (size_t i = 0; i < sizeof(q); i++) {
			uart_poll_out(uart, q[i]);
		}
		(void)k_sem_take(&sem_envio, K_MSEC(PERIODO_MS));
	}
}

/* As duas só começam em enlace_iniciar(), e nunca na placa única */
K_THREAD_DEFINE(id_t_enlace_rx, TAMANHO_PILHA, t_enlace_rx,
		NULL, NULL, NULL, PRIORIDADE_ENLACE, 0, SYS_FOREVER_MS);
K_THREAD_DEFINE(id_t_enlace_tx, TAMANHO_PILHA, t_enlace_tx,
		NULL, NULL, NULL, PRIORIDADE_ENLACE, 0, SYS_FOREVER_MS);

int enlace_iniciar(enum enlace_papel papel)
{
	mestre = papel != ENLACE_ESCRAVA;

	if (papel == ENLACE_UNICA) {
		sozinha = true;
		return 0;
	}

	if (!device_is_ready(uart)) {
		return -ENODEV;
	}

	/* Estado inicial seguro: mestre sem dar a vez, escrava parada */
	atomic_set(&local, mestre ? BIT_MESTRE : BIT_SINAL);

	int ret = uart_irq_callback_user_data_set(uart, isr_uart, NULL);

	if (ret < 0) {
		return ret;
	}
	uart_irq_rx_enable(uart);

	k_thread_start(id_t_enlace_rx);
	k_thread_start(id_t_enlace_tx);
	return 0;
}

bool enlace_sou_mestre(void)
{
	return mestre;
}

bool enlace_uma_placa(void)
{
	return sozinha;
}

static void publica_bit(atomic_val_t bit, bool valor)
{
	if (sozinha) {
		return;
	}
	if (valor) {
		atomic_or(&local, bit);
	} else {
		atomic_and(&local, ~bit);
	}
	k_sem_give(&sem_envio);   /* t_enlace_tx manda assim que quem publicou esperar */
}

void enlace_publicar(bool valor)
{
	publica_bit(BIT_SINAL, valor);
}

bool enlace_ler(void)
{
	return (atomic_get(&remoto) & BIT_SINAL) != 0;
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
	publica_bit(BIT_NOTURNO, ligado);
}

bool enlace_ler_noturno(void)
{
	return (atomic_get(&remoto) & BIT_NOTURNO) != 0;
}

bool enlace_falhou(void)
{
	return atomic_get(&falha) != 0;
}
