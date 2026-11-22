/*
 * Semáforo de veículos (LED RGB da placa: vermelho PTB18, verde PTB19;
 * amarelo = os dois acesos) e handshake com a outra placa.
 */

#ifndef SEMAFORO_VEICULO_H
#define SEMAFORO_VEICULO_H

#include <stdbool.h>

/*
 * Configura os LEDs e começa no vermelho. Chamar depois de enlace_iniciar(),
 * modo_noturno_iniciar() e semaforo_pedestres_iniciar().
 *
 * usar_botao_travessia = true na escrava e na placa única: configura o botão
 * do D10. No mestre (false), o D10 não é usado, e o verde sempre dura 3 s.
 */
int semaforo_veiculo_iniciar(bool usar_botao_travessia);

#endif /* SEMAFORO_VEICULO_H */
