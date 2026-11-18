/*
 * Modo noturno do semáforo.
 *
 * Separa o PEDIDO do modo efetivo:
 *   - pedido: no mestre, vem do botão no D9 (cada toque alterna); na escrava,
 *     vem do fio NOTURNO do mestre;
 *   - modo efetivo: só muda quando o semáforo de veículos chega a um ponto
 *     seguro e chama modo_noturno_aplicar(). É ele que faz as luzes piscarem.
 */

#ifndef MODO_NOTURNO_H
#define MODO_NOTURNO_H

#include <stdbool.h>

/* usar_botao = true no mestre: configura o botão do D9 */
int modo_noturno_iniciar(bool usar_botao);

/* true se o modo noturno foi pedido (botão no mestre, fio na escrava) */
bool modo_noturno_pedido(void);

/* Liga ou desliga o modo efetivo. No mestre, também publica no fio NOTURNO */
void modo_noturno_aplicar(bool ligado);

/* true se o modo noturno está em vigor */
bool modo_noturno_ativo(void);

#endif /* MODO_NOTURNO_H */
