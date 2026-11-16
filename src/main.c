/*
 * Race condition: contador de vagas de um estacionamento.
 *
 * Duas portarias (entrada e saída) compartilham a variável `vagas_ocupadas`.
 * Cada carro que entra faz +1 e cada carro que sai faz -1. Como cada operação
 * é uma sequência ler -> modificar -> escrever, quando as duas threads
 * intercalam essas etapas uma atualização é perdida e o contador final
 * deixa de bater com o número real de carros.
 *
 * USAR_MUTEX 0: versão com a condição de corrida (o erro aparece).
 * USAR_MUTEX 1: versão corrigida, com k_mutex protegendo a seção crítica.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define USAR_MUTEX 0

#define OPERACOES_POR_PORTARIA 1000   /* carros que entram e que saem */
#define TAMANHO_PILHA          512
#define PRIORIDADE_PORTARIA    5      /* mesma prioridade: uma cede a outra */

/* Recurso compartilhado entre as duas threads */
static volatile int vagas_ocupadas = 0;

K_MUTEX_DEFINE(mutex_vagas);
K_SEM_DEFINE(fim_portarias, 0, 2);

/* Seção crítica: ler, modificar e escrever o contador */
static void atualiza_vagas(int delta)
{
#if USAR_MUTEX
	k_mutex_lock(&mutex_vagas, K_FOREVER);
#endif

	int atual = vagas_ocupadas;   /* 1. lê o valor */
	k_yield();                    /* 2. a outra portaria pode rodar aqui */
	vagas_ocupadas = atual + delta; /* 3. grava com base no valor lido antes */

#if USAR_MUTEX
	k_mutex_unlock(&mutex_vagas);
#endif
}

static void portaria_entrada(void *p1, void *p2, void *p3)
{
	for (int i = 0; i < OPERACOES_POR_PORTARIA; i++) {
		atualiza_vagas(+1);
	}
	k_sem_give(&fim_portarias);
}

static void portaria_saida(void *p1, void *p2, void *p3)
{
	for (int i = 0; i < OPERACOES_POR_PORTARIA; i++) {
		atualiza_vagas(-1);
	}
	k_sem_give(&fim_portarias);
}

K_THREAD_DEFINE(id_entrada, TAMANHO_PILHA, portaria_entrada,
		NULL, NULL, NULL, PRIORIDADE_PORTARIA, 0, 0);
K_THREAD_DEFINE(id_saida, TAMANHO_PILHA, portaria_saida,
		NULL, NULL, NULL, PRIORIDADE_PORTARIA, 0, 0);

int main(void)
{
	printk("=== Race condition: contador de vagas ===\n");
	printk("Modo: %s\n", USAR_MUTEX ? "COM mutex (correto)" : "SEM mutex (race condition)");
	printk("Entradas e saidas: %d cada. Esperado no fim: 0 vagas ocupadas\n",
	       OPERACOES_POR_PORTARIA);

	k_sem_take(&fim_portarias, K_FOREVER);
	k_sem_take(&fim_portarias, K_FOREVER);

	printk("Vagas ocupadas no fim: %d\n", vagas_ocupadas);
	if (vagas_ocupadas == 0) {
		printk("Resultado: OK\n");
	} else {
		printk("Resultado: ERRO, %d atualizacoes perdidas\n", vagas_ocupadas);
	}

	return 0;
}
