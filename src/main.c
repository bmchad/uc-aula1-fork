/*
 * Race condition: contador de vagas de um estacionamento.
 *
 * Duas portarias (entrada e saída) compartilham a variável `vagas_ocupadas`.
 * Cada carro que entra faz +1 e cada carro que sai faz -1. Como cada operação
 * é uma sequência ler -> modificar -> escrever, quando as duas threads
 * intercalam essas etapas uma atualização é perdida e o contador final
 * deixa de bater com o número real de carros.
 *
 * O programa roda o mesmo cenário em três modos, um depois do outro:
 *   1. sem proteção: a race condition aparece;
 *   2. com k_mutex: a seção crítica fica com uma portaria por vez;
 *   3. com k_sem binário (semáforo iniciado em 1): mesmo efeito do mutex.
 * Cada modo roda RODADAS vezes e imprime o valor final e o tempo gasto.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define OPERACOES_POR_PORTARIA 1000   /* carros que entram e que saem */
#define RODADAS                3
#define TAMANHO_PILHA          512
#define PRIORIDADE_PORTARIA    5      /* mesma prioridade: uma cede a outra */

enum modo {
	SEM_PROTECAO,
	COM_MUTEX,
	COM_SEMAFORO,
};

static const char *const nome_modo[] = {
	[SEM_PROTECAO] = "sem protecao",
	[COM_MUTEX]    = "com k_mutex",
	[COM_SEMAFORO] = "com k_sem (semaforo binario)",
};

/* Recurso compartilhado entre as duas threads */
static volatile int vagas_ocupadas;
static enum modo modo_atual;

K_MUTEX_DEFINE(mutex_vagas);
K_SEM_DEFINE(sem_vagas, 1, 1);   /* 1 = a seção crítica começa livre */

K_THREAD_STACK_DEFINE(pilha_entrada, TAMANHO_PILHA);
K_THREAD_STACK_DEFINE(pilha_saida, TAMANHO_PILHA);
static struct k_thread thread_entrada;
static struct k_thread thread_saida;

static void entra_secao_critica(void)
{
	if (modo_atual == COM_MUTEX) {
		k_mutex_lock(&mutex_vagas, K_FOREVER);
	} else if (modo_atual == COM_SEMAFORO) {
		k_sem_take(&sem_vagas, K_FOREVER);
	}
}

static void sai_secao_critica(void)
{
	if (modo_atual == COM_MUTEX) {
		k_mutex_unlock(&mutex_vagas);
	} else if (modo_atual == COM_SEMAFORO) {
		k_sem_give(&sem_vagas);
	}
}

/* Seção crítica: ler, modificar e escrever o contador */
static void atualiza_vagas(int delta)
{
	entra_secao_critica();

	int atual = vagas_ocupadas;     /* 1. lê o valor */
	k_yield();                      /* 2. a outra portaria pode rodar aqui */
	vagas_ocupadas = atual + delta; /* 3. grava com base no valor lido antes */

	sai_secao_critica();
}

/* p1 traz o delta da portaria: +1 na entrada, -1 na saída */
static void portaria(void *p1, void *p2, void *p3)
{
	int delta = (int)(intptr_t)p1;

	for (int i = 0; i < OPERACOES_POR_PORTARIA; i++) {
		atualiza_vagas(delta);
	}
}

static void executa_rodada(enum modo modo, int rodada)
{
	vagas_ocupadas = 0;
	modo_atual = modo;

	uint32_t inicio = k_cycle_get_32();

	k_thread_create(&thread_entrada, pilha_entrada,
			K_THREAD_STACK_SIZEOF(pilha_entrada), portaria,
			(void *)(intptr_t)+1, NULL, NULL,
			PRIORIDADE_PORTARIA, 0, K_NO_WAIT);
	k_thread_create(&thread_saida, pilha_saida,
			K_THREAD_STACK_SIZEOF(pilha_saida), portaria,
			(void *)(intptr_t)-1, NULL, NULL,
			PRIORIDADE_PORTARIA, 0, K_NO_WAIT);

	k_thread_join(&thread_entrada, K_FOREVER);
	k_thread_join(&thread_saida, K_FOREVER);

	uint32_t us = k_cyc_to_us_floor32(k_cycle_get_32() - inicio);

	printk("[%s] rodada %d: vagas no fim = %d, tempo = %u us -> %s\n",
	       nome_modo[modo], rodada, vagas_ocupadas, us,
	       vagas_ocupadas == 0 ? "OK" : "ERRO");
}

int main(void)
{
	printk("=== Race condition: contador de vagas ===\n");
	printk("Entradas e saidas: %d cada. Esperado no fim: 0 vagas ocupadas\n",
	       OPERACOES_POR_PORTARIA);

	for (enum modo modo = SEM_PROTECAO; modo <= COM_SEMAFORO; modo++) {
		for (int rodada = 1; rodada <= RODADAS; rodada++) {
			executa_rodada(modo, rodada);
		}
	}

	printk("=== Fim ===\n");
	return 0;
}
