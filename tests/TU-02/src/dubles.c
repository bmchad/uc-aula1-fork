/*
 * TU-02: dublês dos módulos de que lib/semaforo_veiculo depende.
 *
 * Cada função abaixo substitui a de verdade, com o comportamento mais simples
 * possível, para o teste ver só o semáforo dos carros:
 *   - enlace: placa única, sem a outra placa. Com isso, o vermelho dos carros
 *     dura VERMELHO_MS (4 s) fixos e nenhuma função de handshake é chamada;
 *   - modo noturno: nunca pedido e nunca ativo (testado no TS-01);
 *   - pedestres: liberar e encerrar não fazem nada (testados no TU-01, e a
 *     sincronia com os carros, no TI-01). Os LEDs dos pedestres ficam apagados.
 *
 * Cada chamada aos dublês de pedestres vai para a serial: ela mostra que os
 * carros pedem a travessia no vermelho e a encerram antes de abrir.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "enlace.h"
#include "modo_noturno.h"
#include "semaforo_pedestres.h"

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

/* ---- pedestres: só registram a chamada ---- */

void semaforo_pedestres_liberar(void)
{
	printk("%u ms | (duble) pedestres liberados\n", k_uptime_get_32());
}

void semaforo_pedestres_encerrar(void)
{
	printk("%u ms | (duble) pedestres encerrados\n", k_uptime_get_32());
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
