/*
 * TS-01: dublê do módulo que fica fora deste teste.
 *
 *   - enlace: placa única, sem a outra placa. Com isso, o vermelho dos carros
 *     dura VERMELHO_MS (4 s) fixos, nenhuma função de handshake é chamada e o
 *     bit NOTURNO não vai para lugar nenhum (as duas placas no modo noturno
 *     são testadas no TA-01).
 *
 * O modo noturno (com o botão no D9) e os semáforos de carros e de pedestres
 * são os de verdade.
 */

#include <zephyr/kernel.h>

#include "enlace.h"

bool enlace_uma_placa(void)
{
	return true;
}

bool enlace_sou_mestre(void)
{
	return true;
}

/* As funções abaixo só são usadas com duas placas */

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

void enlace_publicar_noturno(bool ligado)
{
	ARG_UNUSED(ligado);
}

bool enlace_ler_noturno(void)
{
	/* Não é chamada: com o botão, o pedido vem dele, e não do enlace */
	return false;
}
