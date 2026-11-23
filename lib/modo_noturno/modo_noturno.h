/*
 * Modo noturno do semáforo.
 *
 * Separa o PEDIDO do modo efetivo:
 *   - pedido: no mestre, vem do botão no D9 (cada toque alterna); na escrava,
 *     vem do bit NOTURNO do enlace com o mestre;
 *   - modo efetivo: só muda quando o semáforo de veículos chega a um ponto
 *     seguro e chama modo_noturno_aplicar(). É ele que faz as luzes piscarem.
 */

#ifndef MODO_NOTURNO_H
#define MODO_NOTURNO_H

#include <stdbool.h>

/* usar_botao = true no mestre: configura o botão do D9 */
int modo_noturno_iniciar(bool usar_botao);

/* true se o modo noturno foi pedido (botão no mestre, enlace na escrava) */
bool modo_noturno_pedido(void);

/* Liga ou desliga o modo noturno efetivo. No mestre, também publica no bit NOTURNO do enlace */
void modo_noturno_aplicar(bool ligado);

/* true se o modo noturno está em vigor */
bool modo_noturno_ativo(void);

/*
 * Modo ALERTA (RF-04 a RF-06): o enlace com a outra placa falhou. As luzes
 * são as do modo noturno (amarelo dos carros e vermelho dos pedestres
 * piscando), mas o motivo é outro, e ele tem prioridade sobre o noturno.
 *   - pedido: o enlace está em falha agora (enlace_falhou);
 *   - modo efetivo: só muda quando o semáforo de veículos chama
 *     modo_alerta_aplicar().
 */
bool modo_alerta_pedido(void);
void modo_alerta_aplicar(bool ligado);
bool modo_alerta_ativo(void);

/* true se as luzes devem piscar: modo NOTURNO ou ALERTA em vigor */
bool modo_pisca(void);

#endif /* MODO_NOTURNO_H */
