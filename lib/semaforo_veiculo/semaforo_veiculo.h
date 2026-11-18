/*
 * Semáforo de veículos (LED RGB da placa: vermelho PTB18, verde PTB19;
 * amarelo = os dois acesos) e handshake com a outra placa.
 */

#ifndef SEMAFORO_VEICULO_H
#define SEMAFORO_VEICULO_H

/*
 * Configura os LEDs e o botão de travessia e começa no vermelho. Chamar
 * depois de enlace_iniciar(), modo_noturno_iniciar() e
 * semaforo_pedestres_iniciar().
 */
int semaforo_veiculo_iniciar(void);

#endif /* SEMAFORO_VEICULO_H */
