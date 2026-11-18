/*
 * Enlace entre as duas placas do semáforo (ver src/README.md, seção 4.2).
 *
 * Cada placa tem uma saída (D12) e uma entrada (D11) de handshake, ligadas em
 * cruz com a outra placa. O significado depende do papel:
 *   - mestre: saída = VEZ, entrada = PARADO da escrava;
 *   - escrava: saída = PARADO, entrada = VEZ do mestre.
 * O papel vem do programa gravado: src/mestre.c (que também é a placa única
 * da etapa 1) ou src/escrava.c.
 * O fio NOTURNO vai do D2 do mestre para o D3 da escrava.
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
 * Configura os pinos no estado seguro: VEZ = 0 (mestre) ou PARADO = 1 (escrava).
 *
 * ENLACE_UNICA: nenhum pino do enlace é configurado, a placa age como mestre
 * e as funções de publicar não fazem nada.
 */
int enlace_iniciar(enum enlace_papel papel);

bool enlace_sou_mestre(void);

/* true na etapa 1: a placa roda sozinha, sem handshake */
bool enlace_uma_placa(void);

/* Saída de handshake: VEZ (mestre) ou PARADO (escrava) */
void enlace_publicar(bool valor);

/* Entrada de handshake: PARADO (no mestre) ou VEZ (na escrava) */
bool enlace_ler(void);

/* Espera a entrada chegar ao nível pedido. Retorna 0, ou -EAGAIN se o tempo acabou */
int enlace_esperar(bool nivel, k_timeout_t timeout);

/* Espera a entrada mudar a partir de agora. Retorna true se mudou antes do tempo acabar */
bool enlace_aguardar_mudanca(k_timeout_t timeout);

/* Fio NOTURNO: o mestre publica, a escrava lê */
void enlace_publicar_noturno(bool ligado);
bool enlace_ler_noturno(void);

#endif /* ENLACE_H */
