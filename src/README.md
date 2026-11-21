# Projeto: semáforos de carros e pedestres em duas placas (Aula 07)

Documento de projeto, escrito antes do código. É a referência para a fiação e a lógica. Substitui o esboço de 16/11, em que cada placa tinha um só semáforo, e o plano de 17/11, com um fio de PTD3 para PTD3. O enunciado da atividade está em [../docs/enunciado.md](../docs/enunciado.md).

> **Atualização:** os fios VEZ, PARADO e NOTURNO descritos aqui foram trocados por uma UART entre as placas (UART2, nos mesmos pinos D11 e D12). As regras do handshake da seção 5.2 continuam valendo: VEZ, PARADO e NOTURNO viraram bits de um quadro. A referência atual é o [02-arquitetura.md](../docs/02-arquitetura.md), seção 2, e o [03-projeto-componentes.md](../docs/03-projeto-componentes.md). As seções 2, 3, 6 e 7 deste documento ficam como registro do projeto com fios.

## 1. O plano

- **Mesmo código nas duas placas:** as duas FRDM-KL25Z rodam o mesmo programa ao mesmo tempo.
- **Cada placa é um cruzamento completo:**
  - semáforo de **carros** no LED RGB da própria placa: vermelho, verde e amarelo (vermelho e verde acesos juntos);
  - semáforo de **pedestres** em dois LEDs avulsos na protoboard, um verde e um vermelho. São 2 verdes e 2 vermelhos no total, um par por placa.
- **A restrição pedida:** "se o PTB18 da outra placa estiver ligado, o meu PTB18 tem que estar desligado; o mesmo para o PTB19". Em termos de luz, os carros das duas placas nunca mostram a mesma cor. Ou seja, **as duas placas trabalham em fases opostas, como duas ruas que se cruzam**: quando uma rua está aberta, a outra está fechada.

> **Atenção ao enunciado:** essa leitura foge do enunciado em dois pontos. Ele pede "uma única via" e coloca o semáforo de pedestres numa placa e o de veículos na outra. Vale confirmar com o professor antes de implementar.

## 2. Problemas encontrados no plano e as correções

| # | Problema | Por que é um problema | Correção |
|---|---|---|---|
| 1 | Ler o PTB18 e o PTB19 da outra placa | Pelo pinout da FRDM-KL25Z, esses pinos **não estão em nenhum conector**: ligam só no LED RGB. Não há onde ligar o fio. | Cada placa **publica o próprio estado** num pino livre (saída) e **lê o da outra** em outro pino (entrada). |
| 2 | Ligar um pino da placa A direto no mesmo pino da placa B | Se os dois forem saídas, quando um estiver em 1 e o outro em 0 há **curto entre as duas saídas**, e os pinos podem queimar. | Todo fio entre placas liga **uma saída a uma entrada**. Um resistor de 1 kΩ em série em cada fio limita a corrente se alguém configurar errado. |
| 3 | "PTB18 mandando tensão" como sinal de "vermelho aceso" | O LED RGB acende em **nível baixo**: PTB18 em 3,3 V significa vermelho **apagado**. A regra ao pé da letra ficaria invertida. | Os sinais entre placas são definidos pelo **estado do semáforo** ("carros parados = 1"), não pela tensão do pino do LED. |
| 4 | Regra por pino com o amarelo | O amarelo é o PTB18 e o PTB19 acesos juntos. Durante o amarelo de uma placa, a outra não poderia ter nem vermelho nem verde, e teria de **ficar apagada**. | A regra passa a valer para o **estado** (aberta ou fechada), não para cada pino. |
| 5 | "Nunca vermelho nas duas ao mesmo tempo" | A segurança só exige que as duas ruas **nunca fiquem abertas** (verde ou amarelo) ao mesmo tempo. Na troca, uma rua precisa fechar antes de a outra abrir, então há um instante com as duas no vermelho, e **esse é o comportamento seguro**. Proibir isso obrigaria as duas placas a trocarem no mesmo instante exato, o que placas com relógios separados não garantem: qualquer atraso violaria uma das regras. | A única regra entre placas é: **nunca as duas abertas**. |
| 6 | Cada placa decidir sozinha, olhando a outra | É uma **race condition entre placas**: cada uma lê "a outra está fechada" e abre; se as duas lerem no mesmo instante, as duas abrem. É o erro da Aula 4, só que agora `k_mutex` e `k_sem` não ajudam, porque não atravessam placas. Além disso, os relógios derivam, e os ciclos se desalinham com o tempo. | Uma placa é **mestre** e decide de quem é a vez; a outra é **escrava** e segue. As bibliotecas são as mesmas nas duas, e o papel vem do programa gravado: `mestre.c` ou `escrava.c`. A escrava confirma que fechou antes de o mestre abrir. |
| 7 | Um botão de modo noturno em cada placa | As placas poderiam discordar: uma piscando o amarelo e a outra com o verde aceso. | **Só o mestre** tem o botão do modo noturno e repassa o modo à escrava por um fio. |

## 3. Arquitetura

- **Dentro de cada placa:** é a mesma lógica da Aula 6.
  - Uma thread por LED.
  - Um `k_mutex` por semáforo, para a exclusão mútua entre as luzes.
  - Uma cadeia de `k_sem` para a ordem entre as luzes.
  - As interrupções só fazem `k_sem_give`, porque não podem usar mutex.
- **Entre as placas:** um handshake de dois fios. O mestre dá a vez (sinal **VEZ**), e a escrava confirma quando fechou (sinal **PARADO**). Nenhuma placa abre sem a confirmação da outra.
- **Mesmas bibliotecas nas duas placas:** o papel vem do arquivo gravado, `src/mestre.c` ou `src/escrava.c` (seção 5.5).

## 4. Conexões

A fiação passou para a seção 1 do [docs/03-projeto-componentes.md](../docs/03-projeto-componentes.md), que é a referência atual dos pinos:
- pinos de cada placa, LEDs e resistores: seções 1.1 e 1.2;
- pinos que não devem ser usados: seção 1.3;
- fios entre as placas: seção 1.4;
- montagem da etapa 1 (placa única): seção 1.5.

O comportamento esperado em cada etapa está no [plano de testes](../tests/plano-testes.md).

## 5. Lógica

### 5.1 Ciclo normal (8 s)

| Tempo | Carros do mestre | Pedestres do mestre | Carros da escrava | Pedestres da escrava | VEZ | PARADO |
|---|---|---|---|---|---|---|
| 0 a 3 s | verde | vermelho | vermelho | verde | 0 | 1 |
| 3 a 4 s | amarelo | vermelho | vermelho | verde | 0 | 1 |
| 4 a 7 s | vermelho | verde | verde | vermelho | 1 | 0 |
| 7 a 8 s | vermelho | verde | amarelo | vermelho | 1 | 0 |

Os tempos do enunciado fecham sozinhos. O vermelho de cada placa (4 s) dura o verde mais o amarelo da outra (3 + 1 s). Os pedestres de cada placa ficam no verde durante o vermelho dos próprios carros (4 s), e no vermelho durante o verde e o amarelo deles (4 s).

### 5.2 Regras do handshake

**Mestre:**
1. Só abre (verde) com **VEZ = 0 e PARADO = 1**.
2. Só levanta VEZ depois que os próprios carros estão no vermelho.
3. Baixa VEZ quando vê PARADO voltar para 1, ou seja, quando a escrava fechou.

**Escrava:**
1. Só abre numa **subida de VEZ**, e só depois de ter visto VEZ em 0 por pelo menos 1 s desde o boot.
2. Baixa PARADO **antes** de abrir.
3. Levanta PARADO só **depois** de voltar ao vermelho.

**Por que bordas e não níveis:** se a escrava olhasse só o nível, um VEZ ainda em 1 de um ciclo anterior a faria abrir de novo. A exigência de ver VEZ em 0 depois do boot protege contra a configuração errada de duas escravas (seção 6).

**Casos que só apareceram ao detalhar o código:**

| Caso | Problema | Solução no código |
|---|---|---|
| A escrava reinicia enquanto o mestre dá a vez | **Impasse.** A escrava espera ver VEZ em 0, e o mestre espera ela abrir. | Se a escrava não abrir em **3 s** (`RESPOSTA_MS`), o mestre **cancela** (VEZ = 0), espera **5 s** (`ESPERA_CANCELADA_MS`, linha abaixo) e segue o próprio ciclo. A escrava completa o boot durante o verde do mestre. |
| Fio PARADO em curto com o GND (com sinais ativos em nível baixo, isso é PARADO = 1 para sempre) | **As duas ruas abrem juntas.** O mestre dá a vez, a escrava abre, e o mestre não vê PARADO = 0. Ele cancela depois de 3 s, lê PARADO = 1 e reabre com a escrava ainda aberta. | Depois de cancelar, o mestre espera o ciclo inteiro da escrava, verde mais amarelo mais 1 s de margem (`ESPERA_CANCELADA_MS` = 5 s), antes de seguir. Se ela abriu sem o mestre ver, já fechou. O revezamento continua, só que por tempo. |
| O mestre cancela no mesmo instante em que a escrava decide abrir | **Race condition entre placas:** as duas poderiam abrir. | A escrava baixa PARADO, espera **50 ms** (`GUARDA_MS`) e **confere de novo** que VEZ continua em 1 antes de abrir; se não continuar, volta a PARADO = 1. O mestre, depois de baixar VEZ, também espera 50 ms antes de conferir PARADO. |
| Boot | No primeiro ciclo não há vez para dar. | O mestre começa abrindo o próprio verde, assim que vê PARADO = 1. A escrava espera o VEZ = 0 estável e pega a vez no fim do primeiro ciclo do mestre. |

### 5.3 Threads e primitivas em cada placa

| Thread | Controla | Observação |
|---|---|---|
| `t_carro_verde` | PTB19 | 3 s. O botão de travessia encurta o verde, respeitando 1 s mínimo. |
| `t_carro_amarelo` | PTB18 + PTB19 | 1 s. No modo noturno, pisca. |
| `t_carro_vermelho` | PTB18 | Faz o handshake com a outra placa e libera os pedestres da própria placa. Na etapa 1, sem a outra placa, dura 4 s fixos. |
| `t_ped_verde` | D7 | Só acende com os carros da própria placa no vermelho. |
| `t_ped_vermelho` | D6 | No modo noturno, pisca. |
| (sem thread) | — | A interrupção do botão de travessia (D10) avisa `t_carro_verde` por `k_sem`. **Não precisa de debounce:** encurtar o verde atual pode acontecer várias vezes sem efeito extra, e o verde descarta os toques antigos ao começar. |
| `t_botao_noturno` | — | Só no mestre. Já existe em `lib/modo_noturno/`. |

- **`mutex_carros`:** exclusão mútua entre as três luzes dos carros.
- **`mutex_pedestres`:** exclusão mútua entre as duas luzes dos pedestres.
- **Cadeia de `k_sem`:** a ordem verde → amarelo → vermelho, e a passagem para os pedestres dentro da placa.
- **Interrupções** dos botões e das entradas VEZ, PARADO e NOTURNO: só `k_sem_give`.

### 5.4 Travessia e modo noturno

- **Travessia:** o botão de cada placa encurta o verde dos **próprios** carros, que passam pelo amarelo e fecham. Isso só antecipa a troca de vez e **não precisa de fio extra**.
- **Modo noturno:**
  - **Entrada:** o botão do mestre alterna o modo. O mestre leva o próprio ciclo até o vermelho, que é o ponto seguro, e só então levanta NOTURNO. As duas placas passam a piscar o amarelo dos carros e o vermelho dos pedestres, 1 s aceso e 1 s apagado.
  - **Saída:** o mestre baixa NOTURNO, e as duas placas vão para o vermelho. O mestre volta dando a vez, então **a escrava abre primeiro**. Durante o modo noturno o mestre mantém VEZ = 0, por isso o primeiro VEZ = 1 depois dele é uma vez nova para a escrava.

### 5.5 Organização do código

| Biblioteca | Responsabilidade | Funções públicas |
|---|---|---|
| `lib/enlace` | Pinos entre as placas. O papel é passado pelo `mestre.c` ou pelo `escrava.c`. A interrupção da entrada só acorda quem espera, e quem espera sempre confere o nível de novo. | `enlace_iniciar`, `enlace_sou_mestre`, `enlace_uma_placa`, `enlace_publicar`, `enlace_ler`, `enlace_esperar`, `enlace_aguardar_mudanca`, `enlace_publicar_noturno`, `enlace_ler_noturno` |
| `lib/modo_noturno` | Separa o **pedido** (botão no mestre, fio NOTURNO na escrava) do **modo efetivo**, que só muda no ponto seguro | `modo_noturno_iniciar`, `modo_noturno_pedido`, `modo_noturno_aplicar`, `modo_noturno_ativo` |
| `lib/semaforo_pedestres` | Threads do vermelho e do verde dos pedestres, com `mutex_pedestres` | `semaforo_pedestres_iniciar`, `semaforo_pedestres_liberar`, `semaforo_pedestres_encerrar` |
| `lib/semaforo_veiculo` | Threads do verde, do amarelo e do vermelho dos carros, com `mutex_carros`; handshake e botão de travessia | `semaforo_veiculo_iniciar` |
| `src/mestre.c` | Inicia as quatro bibliotecas, nesta ordem, como mestre. Com `-DPAPEL=mestre` é a etapa 2; com `-DPAPEL=placa_unica`, o `CMakeLists.txt` define `UMA_PLACA=1` para a etapa 1 (seção 1.5 do [03-projeto-componentes.md](../docs/03-projeto-componentes.md)). | — |
| `src/escrava.c` | Inicia as quatro bibliotecas como escrava. A lógica da escrava fica em `lib/semaforo_veiculo`. | — |

- **Gravação:** o papel é escolhido pela variável `PAPEL` do `CMakeLists.txt`, com uma pasta de build por papel: `west build -b frdm_kl25z -d build/mestre -- -DPAPEL=mestre` e `west build -b frdm_kl25z -d build/escrava -- -DPAPEL=escrava`, e depois `west flash -d build/mestre` ou `west flash -d build/escrava`. Para a etapa 1, `-DPAPEL=placa_unica`. Sem `-DPAPEL`, o papel é o mestre. O upload não escolhe a placa, então, na hora de gravar, deixe só uma placa no USB do PC. Os fios entre as placas (seção 1.4 do [03-projeto-componentes.md](../docs/03-projeto-componentes.md)) podem ficar ligados.

- **Pinos:** estão todos em `frdm_kl25z.overlay`, na raiz do projeto. O Zephyr aplica esse arquivo sozinho porque o nome dele é o da placa. A porta C, dos LEDs de pedestres, é habilitada lá.
- **Serial (115200 bps):** cada troca de luz sai como `tempo ms | carros VERDE` ou `tempo ms | pedestres VERMELHO`. Com a serial das duas placas gravada, dá para conferir que as ruas nunca abriram juntas.

## 6. Falha segura

| Situação | Resultado |
|---|---|
| Fio VEZ solto | A escrava lê 0 e nunca abre. O mestre não recebe a resposta, cancela a vez depois de 3 s, espera 5 s e segue o próprio ciclo. **A escrava fica no vermelho; nunca há as duas abertas.** |
| Fio PARADO solto | O mestre lê 0 e nunca abre. **As duas ficam no vermelho.** |
| Fio PARADO em curto com o GND | O mestre lê PARADO = 1 para sempre e não vê a escrava abrir. Ele cancela a vez depois de 3 s e espera 5 s, o ciclo inteiro dela, antes de reabrir. **O revezamento continua por tempo, e nunca há as duas abertas.** Sem essa espera, as duas abriam juntas. |
| Fio NOTURNO solto | A escrava não entra no modo noturno. O mestre continua respeitando o handshake, então nunca há as duas abertas. |
| Uma placa reseta no meio do ciclo | Ela volta fechada. Pelas regras do handshake, a outra fecha ao fim da sua fase e espera. |
| `mestre` gravado nas duas placas | Cada uma lê o VEZ da outra, que é 0, como PARADO = 0. **Nenhuma abre.** |
| `escrava` gravada nas duas placas | Se as duas ligarem juntas, cada uma vê o PARADO da outra sempre em 1, nunca vê VEZ em 0, e **nenhuma abre**. Se uma ligar bem depois da outra, elas podem se revezar, porque cada uma só abre quando a outra acabou de fechar. **Em nenhum caso as duas abrem juntas.** |
| Mestre gravado como `placa_unica` e ligado à escrava | Ele não configura os pinos do enlace, e o D11 da escrava lê 0 lógico pelo pull-up. **A escrava nunca abre**, e o mestre roda o próprio ciclo. A escrava avisa `aguardando a vez do mestre` na serial a cada 10 s: se ela nunca abrir, confira primeiro se o mestre foi compilado com `-DPAPEL=mestre`. |
| Fio GND solto | Os níveis ficam indefinidos. Por isso o fio de GND é obrigatório e deve ser o primeiro a ser ligado. |

Proposta: se a confirmação não chegar em 10 s, a placa sinaliza a falha piscando o vermelho dos carros.

## 7. Pontos de teste que esta arquitetura cria (modelo V)

- **Unitário (uma placa):**
  - **Etapa 1 (placa única):** `-DPAPEL=placa_unica` e a montagem da seção 1.5 do [03-projeto-componentes.md](../docs/03-projeto-componentes.md). Testa as cinco threads, os dois mutexes e a sincronia entre carros e pedestres da mesma placa, sem o handshake.
  - **Escrava sem mestre:** a `escrava` gravada numa placa sozinha. Os carros devem ficar no vermelho para sempre, com os pedestres no verde.
  - **Mestre sem escrava, ou fio PARADO em curto:** `-DPAPEL=mestre` e o D11 ligado ao **GND** por um resistor de 1 kΩ. Como os sinais são ativos em nível baixo, isso simula uma escrava sempre parada. Testa o cancelamento da vez e a espera de segurança: o vermelho dura os 3 s do prazo mais os 5 s da espera, com o aviso "escrava nao pegou a vez: esperando o ciclo dela".
  - Tempos de cada luz.
  - Nunca duas luzes do mesmo semáforo acesas, contando o amarelo como uma luz só.
  - Debounce dos botões.
- **Integração (duas placas):**
  - **Nunca as duas ruas abertas,** registrando a serial das duas placas com o tempo de cada linha.
  - Soltar cada fio no meio do ciclo.
  - O mesmo programa nas duas placas: `mestre` nas duas, depois `escrava` nas duas.
  - Resetar uma placa no meio do ciclo.
  - Medir o atraso do handshake.
- **Sistema:**
  - Travessia em cada fase.
  - Entrada e saída do modo noturno em cada fase.
  - Botões apertados várias vezes seguidas.

## 8. Pontos em aberto

1. **Leitura do plano:** confirmar com o professor a ideia de duas ruas, por causa do enunciado (seção 1).
2. **Pinos PTB18 e PTB19:** conferir na placa que eles não estão em nenhum conector. Não muda a solução, porque mesmo que estivessem, ligar saída com saída continuaria proibido (problema 2).
3. **Resistor dos LEDs:** resolvido. A montagem usa 220 Ω (seção 1.2 do [03-projeto-componentes.md](../docs/03-projeto-componentes.md)).
