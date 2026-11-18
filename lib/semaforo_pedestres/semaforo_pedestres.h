/*
 * Semáforo de pedestres (LEDs avulsos: vermelho no D6, verde no D7).
 *
 * Uma thread por LED, com mutex entre elas. Quem decide quando os pedestres
 * atravessam é o semáforo de veículos da mesma placa:
 *   - semaforo_pedestres_liberar(): os carros pararam, pode abrir o verde;
 *   - semaforo_pedestres_encerrar(): fecha o verde e só retorna com o
 *     vermelho dos pedestres aceso.
 * Cada liberar() precisa ser seguido de um encerrar() antes de os carros abrirem.
 */

#ifndef SEMAFORO_PEDESTRES_H
#define SEMAFORO_PEDESTRES_H

int semaforo_pedestres_iniciar(void);
void semaforo_pedestres_liberar(void);
void semaforo_pedestres_encerrar(void);

#endif /* SEMAFORO_PEDESTRES_H */
