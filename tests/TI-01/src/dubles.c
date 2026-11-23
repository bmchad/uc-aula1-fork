/*
 * TI-01: dublês dos módulos que ficam fora deste teste.
 *
 *   - enlace: placa única, sem a outra placa. Com isso, o vermelho dos carros
 *     dura VERMELHO_MS (4 s) fixos e nenhuma função de handshake é chamada
 *     (o enlace é testado no TU-03, e as duas vias, no TI-02);
 *   - modo noturno: nunca pedido e nunca ativo (testado no TS-01).
 *
 * Os semáforos de carros e de pedestres são os de verdade: é a ligação entre
 * eles (semaforo_pedestres_liberar e semaforo_pedestres_encerrar) que este
 * teste verifica.
 */

#include <zephyr/kernel.h>

#include "enlace.h"
#include "modo_noturno.h"

/* ---- enlace: placa única ---- */

bool enlace_uma_placa(void)
{
	return true;
}

bool enlace_sou_mestre(void)
{
	return true;
}

/* As funções abaixo só são usadas no handshake, que não existe na placa única */

void enlace_publicar(bool valor)
{
	ARG_UNUSED(valor);
}

bool enlace_ler(void)
{
	return false;
}

int enlace_esperar(bool nivel, k_timeout_t timeout)
{
	ARG_UNUSED(nivel);
	ARG_UNUSED(timeout);
	return 0;
}

bool enlace_aguardar_mudanca(k_timeout_t timeout)
{
	ARG_UNUSED(timeout);
	return false;
}

/* ---- modo noturno: sempre desligado ---- */

bool modo_noturno_pedido(void)
{
	return false;
}

bool modo_noturno_ativo(void)
{
	return false;
}

void modo_noturno_aplicar(bool ligado)
{
	ARG_UNUSED(ligado);
}
