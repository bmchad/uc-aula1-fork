# Aula 6: solução da race condition no contador de vagas (FRDM-KL25Z)

| | |
|---|---|
| **Aluno** | Bernardo Machado |
| **Data dos testes** | 17/11/2026 |
| **Branch** | `solution-race-condition`, criada a partir de [`race-condition`](https://github.com/bmchad/uc-aula1-fork/tree/race-condition) (Aula 4) |
| **Código** | [`src/main.c`](src/main.c) |
| **Evidências** | [pasta "Evidências - solução da race condition"](https://drive.google.com/drive/folders/1x_IIHKzwv92ZDo6PJHojIje00_E02LAR) no Drive |
| **Situação** | Race condition reproduzida e corrigida com `k_mutex` e com `k_sem`. Todos os testes aprovados. |

## 1. Objetivo

Na Aula 4 (branch `race-condition`), o contador de vagas demonstrou uma race condition: duas threads atualizavam a mesma variável sem sincronização, e o resultado final saía errado. Nesta aula, o objetivo é **corrigir a race condition com um mutex ou um semáforo** e comprovar a correção na placa. As duas primitivas foram implementadas e comparadas.

## 2. O problema

Duas portarias de um estacionamento são duas threads com a mesma prioridade. Elas compartilham a variável `vagas_ocupadas`: a entrada soma 1 por carro e a saída subtrai 1. Cada portaria processa 1000 carros, então o valor esperado no fim é **0**.

A atualização é uma sequência de três passos, **ler, modificar e escrever**, e é essa sequência que forma a **seção crítica**:

```c
int atual = vagas_ocupadas;     /* 1. lê o valor */
k_yield();                      /* 2. a outra portaria pode rodar aqui */
vagas_ocupadas = atual + delta; /* 3. grava com base no valor lido antes */
```

O `k_yield()` entre a leitura e a escrita força a troca de thread exatamente no pior momento. Num programa real, essa troca aconteceria raramente, por uma interrupção ou fatia de tempo, e o erro seria intermitente. Aqui ele é forçado para ficar reproduzível.

**Por que `volatile` não resolve:** `vagas_ocupadas` já é `volatile`. Isso só obriga o compilador a ler e escrever a variável na memória a cada acesso, sem guardar o valor num registrador. Não impede que outra thread rode entre a leitura e a escrita, que é onde a atualização se perde.

## 3. Solução

A seção crítica passou a ser envolvida por duas funções, que travam e liberam conforme o modo:

```c
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
```

- **`k_mutex`** (`K_MUTEX_DEFINE(mutex_vagas)`): só uma thread por vez fica com o mutex. Se a portaria A trava e cede a vez no `k_yield()`, a portaria B bloqueia no `k_mutex_lock` até A terminar a escrita e destravar.
- **`k_sem` binário** (`K_SEM_DEFINE(sem_vagas, 1, 1)`): um semáforo com contagem inicial 1 e limite 1 funciona como uma "ficha" única. Quem pega a ficha (`k_sem_take`) entra, e a outra thread espera até a ficha ser devolvida (`k_sem_give`).

O `k_yield()` continua dentro da seção crítica de propósito. Assim, o teste prova que, mesmo com a troca de thread forçada no pior ponto, a outra portaria não consegue entrar.

## 4. Como o teste roda

Para comparar as três situações numa única gravação, o `main` roda o mesmo cenário em três modos, um depois do outro:

1. **sem proteção** (reproduz a Aula 4);
2. **com `k_mutex`**;
3. **com `k_sem` binário**.

Cada modo roda 3 rodadas. Em cada rodada, o `main` zera o contador, cria as duas portarias com `k_thread_create`, espera as duas terminarem com `k_thread_join` e imprime o valor final e o tempo da rodada, medido com `k_cycle_get_32()`.

Na Aula 4, as threads eram criadas com `K_THREAD_DEFINE`, que as inicia no boot uma única vez. Com `k_thread_create` e `k_thread_join`, dá para rodar o cenário várias vezes, e o `main` não precisa mais de um `k_sem` só para saber quando as portarias terminaram.

## 5. Ambiente de teste

| Item | Versão / valor |
|---|---|
| Placa | FRDM-KL25Z (MKL25Z128VLK4, 48 MHz) |
| RTOS | Zephyr 4.2.0 (PlatformIO, framework-zephyr 3.40200) |
| Compilador | arm-none-eabi-gcc 8.2.1 |
| Gravação | LinkServer (MCUXpresso) pelo OpenSDA/DAPLink |
| Serial | COM15, 115200 bps |
| Uso de memória | Flash 12.696 B (9,7%); RAM 5.223 B (31,9%) |

## 6. Plano de testes

| ID | Teste | Resultado esperado | Situação |
|---|---|---|---|
| R0 | Build e gravação | Compila sem erros e sem avisos em `main.c`; gravação concluída | **Aprovado** |
| R1 | Sem proteção | Valor final diferente de 0 nas 3 rodadas (reproduz a race condition) | **Aprovado** |
| R2 | Com `k_mutex` | Valor final 0 nas 3 rodadas | **Aprovado** |
| R3 | Com `k_sem` binário | Valor final 0 nas 3 rodadas | **Aprovado** |
| R4 | Custo da proteção | Tempo por rodada medido nos três modos | **Medido** (seção 7) |

## 7. Resultados

Saída do programa na serial (tempo do PC em segundos):

```text
31.468  *** Booting Zephyr OS build zephyr-v40200 ***
31.468  === Race condition: contador de vagas ===
31.468  Entradas e saidas: 1000 cada. Esperado no fim: 0 vagas ocupadas
31.562  [sem protecao] rodada 1: vagas no fim = -1000, tempo = 73057 us -> ERRO
31.640  [sem protecao] rodada 2: vagas no fim = -1000, tempo = 74163 us -> ERRO
31.734  [sem protecao] rodada 3: vagas no fim = -1000, tempo = 74163 us -> ERRO
31.843  [com k_mutex] rodada 1: vagas no fim = 0, tempo = 105400 us -> OK
31.953  [com k_mutex] rodada 2: vagas no fim = 0, tempo = 107470 us -> OK
32.078  [com k_mutex] rodada 3: vagas no fim = 0, tempo = 108660 us -> OK
32.187  [com k_sem (semaforo binario)] rodada 1: vagas no fim = 0, tempo = 104786 us -> OK
32.312  [com k_sem (semaforo binario)] rodada 2: vagas no fim = 0, tempo = 104786 us -> OK
32.422  [com k_sem (semaforo binario)] rodada 3: vagas no fim = 0, tempo = 104786 us -> OK
32.422  === Fim ===
```

| Modo | Vagas no fim | Tempo médio por rodada | Tempo por atualização | Em relação a "sem proteção" |
|---|---|---|---|---|
| Sem proteção | −1000 nas 3 rodadas (ERRO) | 73,8 ms | 36,9 µs | — |
| `k_mutex` | 0 nas 3 rodadas (OK) | 107,2 ms | 53,6 µs | +45% |
| `k_sem` binário | 0 nas 3 rodadas (OK) | 104,8 ms | 52,4 µs | +42% |

O tempo por atualização é o tempo da rodada dividido pelas 2000 atualizações (1000 de cada portaria).

## 8. Análise

**Por que o erro é sempre −1000.** Com as duas threads na mesma prioridade e o `k_yield()` entre a leitura e a escrita, o escalonador alterna as portarias sempre na mesma ordem:

1. A entrada lê o valor e cede a vez.
2. A saída lê o mesmo valor e cede a vez.
3. A entrada escreve (lido + 1) e já lê de novo, ou seja, lê o próprio valor que acabou de escrever.
4. A saída escreve (o valor que ela tinha lido − 1), por cima da escrita da entrada.

Cada portaria acaba contando só os próprios carros, e uma escrita apaga a da outra. No fim vale a última escrita, que é a da saída: −1000. **Todas as 1000 entradas se perdem.** Por isso o erro é o mesmo nas três rodadas. O "resultado esperado" do prompt da Aula 4 falava em valor variável, mas, com o `k_yield()` forçando a troca, o escalonamento é determinístico. Num sistema real, em que a troca depende de interrupções e do momento exato, o valor errado variaria de execução para execução, e às vezes o erro nem apareceria. É isso que torna race conditions difíceis de encontrar em testes.

**As duas correções funcionam.** Com `k_mutex` ou com `k_sem`, a portaria que encontra a seção crítica ocupada espera. O `k_yield()` dentro da seção não deixa mais a outra entrar, e o valor final é 0 em todas as rodadas.

**Custo.** A proteção deixou cada rodada cerca de 42% a 45% mais lenta. Com a seção ocupada, a outra portaria bloqueia e o kernel precisa de trocas de contexto extras para acordá-la depois. Esses números estão inflados pelo `k_yield()` dentro da seção crítica, que força uma troca de contexto a cada atualização. Num código real, a seção crítica seria curta e sem `k_yield()`, e o custo seria proporcionalmente menor. A pequena diferença entre mutex e semáforo (2%) não é significativa com 3 rodadas.

**Mutex ou semáforo?** Neste teste os dois dão o mesmo resultado, mas não são equivalentes:

| | `k_mutex` | `k_sem` binário |
|---|---|---|
| Dono | Tem dono: só a thread que travou pode destravar | Não tem dono: qualquer thread (ou ISR) pode dar `k_sem_give` |
| Inversão de prioridade | Tem herança de prioridade, que evita a inversão | Não tem; uma thread de baixa prioridade pode atrasar uma de alta indefinidamente |
| Uso em ISR | Não pode ser usado em interrupção | `k_sem_give` pode ser chamado em interrupção |
| Uso típico | Exclusão mútua sobre um recurso compartilhado | Sinalização entre threads ou de uma ISR para uma thread |

Aqui as duas portarias têm a mesma prioridade, então a herança de prioridade não faz diferença. Mesmo assim, **o mutex é a escolha recomendada para proteger o contador**, porque o problema é de exclusão mútua. A falta de dono do semáforo é um risco: um `k_sem_give` a mais, por engano, deixaria duas threads entrarem na seção crítica, e esse erro não aparece em compilação.

## 9. Observações

- **Saída do programa:** o programa roda uma vez no boot e termina. Para ver a saída de novo, basta resetar a placa com o monitor serial aberto (`pio device monitor`, 115200 bps).
- **Captura bruta:** a captura começou antes da gravação. Por isso, as primeiras 586 linhas do arquivo `captura_serial_bruta.txt` são do firmware anterior (o do acelerômetro). A saída deste programa começa em "Booting Zephyr OS" e também está separada em `saida_do_programa.txt`.
- **Falha passageira no build:** a primeira tentativa de build falhou na geração do Kconfig (`couldn't parse 'efconfig"'` num arquivo gerado em `.pio/build`). A segunda tentativa, sem nenhuma alteração, passou. Não foi um erro do código.

## Como reproduzir

```text
pio run -e frdm_kl25z -t upload
pio device monitor -b 115200
```

Depois, aperte o reset da placa para ver a saída do início.

## Evidências

Todas na [pasta "Evidências - solução da race condition"](https://drive.google.com/drive/folders/1x_IIHKzwv92ZDo6PJHojIje00_E02LAR):

| Arquivo | Conteúdo |
|---|---|
| [saida_do_programa.txt](https://drive.google.com/file/d/1zS8pgSZ3aipsmmxBGckhJsByiE0cYAbW/view) | Saída deste programa na serial, do boot ao "Fim" |
| [captura_serial_bruta.txt](https://drive.google.com/file/d/1QgRjtd4-mGICk_C2EYDKCid4WWkj5fVo/view) | Captura completa da serial, com o tempo do PC em cada linha. As primeiras 586 linhas são do firmware anterior. |
| [build_platformio.txt](https://drive.google.com/file/d/1Mu2k19ofqg0VaRcFEqANxxbjfYz18_P9/view) | Log completo do build |
| [gravacao_linkserver.txt](https://drive.google.com/file/d/1nCagls8wuJYaL-8qGkPRd9azF3aTD_L7/view) | Log da gravação pelo LinkServer |
| [main.c.txt](https://drive.google.com/file/d/1FZ7YlWzV10yYE1BAr91_GjdjcIiLcTdd/view) | Cópia do `src/main.c` testado |
