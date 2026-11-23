/*
 * Enlace entre as duas placas do semáforo, pela UART2
 * (ver docs/02-arquitetura.md, seção 2, e docs/03-projeto-componentes.md, seção 3.1).
 *
 * Cada placa manda à outra, pela UART, um quadro com o próprio estado. O
 * quadro é reenviado a cada 50 ms e também logo que o estado muda. O
 * significado do sinal depende do papel de quem manda:
 *   - mestre: sinal = VEZ, e também o bit NOTURNO;
 *   - escrava: sinal = PARADO.
 * As duas mandam também o bit OUVE: "estou recebendo os seus quadros".
 * O papel vem do programa gravado: src/mestre.c (que também é a placa única
 * da etapa 1) ou src/escrava.c.
 *
 * Estado seguro: sem quadro válido da outra placa há mais de 200 ms, os
 * sinais dela valem 0 (sem vez, escrava não parada, modo normal). Um quadro
 * só é válido com a verificação certa e com o papel oposto ao desta placa.
 *
 * Falha do enlace (modo ALERTA, RF-04 e RF-05): o enlace só está OK quando
 * esta placa recebe a outra E a outra confirma, pelo bit OUVE, que recebe
 * esta. Assim, uma placa desligada ou um fio de dados solto, em qualquer
 * sentido, vira falha nas duas pontas.
 */

#ifndef ENLACE_H
#define ENLACE_H

#include <stdbool.h>
#include <zephyr/kernel.h>

enum enlace_papel {
	ENLACE_UNICA,     /* etapa 1: não existe a outra placa */
	ENLACE_MESTRE,    /* src/mestre.c na etapa 2 */
	ENLACE_ESCRAVA,   /* src/escrava.c */
};

/*
 * Começa no estado seguro: VEZ = 0 (mestre) ou PARADO = 1 (escrava), e
 * NOTURNO = 0. Liga a UART e as threads de envio e recepção.
 *
 * ENLACE_UNICA: a UART não é usada, a placa age como mestre, as funções de
 * publicar não fazem nada e o enlace nunca falha.
 */
int enlace_iniciar(enum enlace_papel papel);

bool enlace_sou_mestre(void);

/* true na etapa 1: a placa roda sozinha, sem handshake */
bool enlace_uma_placa(void);

/* Sinal desta placa: VEZ (mestre) ou PARADO (escrava). Manda na hora. */
void enlace_publicar(bool valor);

/* Sinal da outra placa: PARADO (no mestre) ou VEZ (na escrava). 0 sem enlace. */
bool enlace_ler(void);

/* Espera o sinal da outra placa chegar ao nível pedido. Retorna 0, ou -EAGAIN se o tempo acabou */
int enlace_esperar(bool nivel, k_timeout_t timeout);

/* Espera qualquer mudança na outra placa a partir de agora. Retorna true se mudou antes do tempo acabar */
bool enlace_aguardar_mudanca(k_timeout_t timeout);

/* Bit NOTURNO: o mestre publica, a escrava lê (0 sem enlace) */
void enlace_publicar_noturno(bool ligado);
bool enlace_ler_noturno(void);

/*
 * true se o enlace falhou: esta placa não recebe a outra, ou a outra avisa
 * que não recebe esta. Nos primeiros INICIO_MS depois do boot a falha ainda
 * não é declarada, para as placas trocarem os primeiros quadros. Sempre
 * false na placa única.
 */
bool enlace_falhou(void);

#endif /* ENLACE_H */
