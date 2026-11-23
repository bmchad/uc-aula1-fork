/*
 * TI-02: dublê do módulo que fica fora deste teste.
 *
 *   - modo noturno: nunca pedido e nunca ativo (testado no TS-01). Assim as
 *     duas placas ficam sempre no ciclo normal, e o bit NOTURNO do enlace é
 *     ignorado.
 *
 * O enlace e os semáforos de carros e de pedestres são os de verdade.
 */

#include <zephyr/kernel.h>

#include "modo_noturno.h"

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

/* ---- modo ALERTA: o enlace nunca falha neste teste (testado nos TA-02 a TA-04) ---- */

bool modo_alerta_pedido(void)
{
	return false;
}

bool modo_alerta_ativo(void)
{
	return false;
}

void modo_alerta_aplicar(bool ligado)
{
	ARG_UNUSED(ligado);
}

bool modo_pisca(void)
{
	return false;
}
