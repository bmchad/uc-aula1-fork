## 1. Requisitos funcionais

O sistema controla **duas vias** que se cruzam. Cada placa controla uma via completa: o semáforo de veículos e o de pedestres dessa via. As duas vias trabalham em fases opostas: quando uma está aberta, a outra está fechada.

### 1.1 Modo normal

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RF-01 | O semáforo de veículos de cada via deve ter três luzes: verde, amarelo e vermelho. | Enunciado §2 | Inspeção: nas duas vias, as três luzes são distinguíveis a olho nu |
| RF-02 | O semáforo de pedestres de cada via deve ter duas luzes: verde e vermelho. | Enunciado §1 | Inspeção: nas duas vias, as duas luzes são distinguíveis a olho nu |
| RF-03 | No modo normal, o semáforo de veículos de cada via deve repetir verde por 3 s, amarelo por 1 s e vermelho por 4 s, nessa ordem. | Enunciado §2 | Em pelo menos 5 ciclos seguidos, sem toques nos botões, em cada via: o verde e o amarelo duram o valor pedido ± 1%; o vermelho dura 4 s ± 1% com uma placa e de 4,00 a 4,10 s com duas, pela troca de sinais entre elas; nenhuma fase é pulada e a ordem não muda |
| RF-04 | No modo normal, o semáforo de pedestres de cada via deve repetir verde por 4 s e vermelho por 4 s. | Enunciado §1 | Em pelo menos 5 ciclos seguidos, sem toques nos botões, em cada via: o vermelho dura 4 s ± 1%; o verde dura 4 s ± 1% com uma placa e de 4,00 a 4,10 s com duas, porque acompanha o vermelho dos veículos (RF-03) |
| RF-05 | Em cada via, os dois semáforos devem andar sincronizados: o verde dos pedestres coincide com o vermelho dos veículos, e o vermelho dos pedestres, com o verde e o amarelo dos veículos. | Enunciado §2 ("sincronismo") | Em cada troca, a luz do outro semáforo da mesma via muda em até 50 ms |
| RF-06 | Ao ligar as duas placas, o sistema deve começar com as duas vias fechadas e entrar no modo normal. Uma placa que reinicia sozinha deve voltar com a sua via fechada. | (dupla) | 5 partidas das duas placas: a primeira luz registrada dos veículos é o vermelho nas duas vias, e o ciclo normal começa sem nenhum comando. 5 reinícios de cada placa no meio do ciclo: a primeira luz registrada dos veículos da placa reiniciada é o vermelho |

### 1.2 Botão de travessia

Cada via tem o seu botão de travessia, que age só nos veículos da própria via.

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RF-07 | Um toque no botão de travessia, com a via aberta no verde, deve encerrar o verde dos veículos, passar pelo amarelo e fechar a via, e então acender o verde dos pedestres. | Enunciado §4 | Toque em 5 instantes diferentes do verde: em todos, os veículos passam pelo amarelo (1 s ± 1%) e o verde dos pedestres acende em seguida |
| RF-08 | O verde dos veículos deve durar pelo menos 1 s, mesmo com o botão de travessia apertado. | (dupla) | Toque logo no início do verde: o verde dura de 1,00 a 1,05 s |
| RF-09 | O tempo entre o toque e o verde dos pedestres não deve passar de 2 s. | (dupla) | O maior atraso medido nos testes de RF-07 é ≤ 2,05 s |
| RF-10 | Um toque com a via fechada, ou durante o amarelo, não deve mudar a sequência nem os tempos do ciclo. | (dupla) | Toques nessas fases: as durações continuam dentro de RF-03 e RF-04 |
| RF-11 | Vários toques durante o mesmo verde devem ter o mesmo efeito de um toque só. | (dupla) | 5 toques rápidos no mesmo verde: uma única passagem pelo amarelo, e o verde seguinte tem a duração normal |

### 1.3 Modo noturno

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RF-12 | O operador deve poder ligar e desligar o modo noturno por um botão. Cada toque alterna o modo pedido. | Enunciado §3 e Req. de Projeto 3 | 10 toques separados por pelo menos 0,5 s geram exatamente 10 alternâncias registradas na serial |
| RF-13 | No modo noturno, o amarelo dos veículos deve piscar com 1 s aceso e 1 s apagado, e as outras luzes dos veículos devem ficar apagadas. | Enunciado §3 | Com o modo ativo por pelo menos 10 s, nas duas vias: cada meio período dura 1 s ± 1%, e nem o verde nem o vermelho dos veículos acendem |
| RF-14 | No modo noturno, o vermelho dos pedestres deve piscar com 1 s aceso e 1 s apagado, e o verde dos pedestres deve ficar apagado. | Enunciado §3 | Mesmo ensaio de RF-13, nas luzes dos pedestres |
| RF-15 | Em cada via, o pisca dos veículos e o dos pedestres devem estar em fase: os dois acendem e apagam juntos. | (dupla) | Nas trocas do pisca, a diferença entre os dois semáforos da mesma via é ≤ 50 ms |
| RF-16 | O sistema deve entrar no modo noturno só no ponto seguro, sem interromper nenhuma fase do ciclo normal. | (dupla) | Pedido em cada fase do ciclo: o pisca começa no fim do vermelho dos veículos, nas duas vias, em até 8,2 s após o pedido. Os 0,2 s cobrem a troca de sinais entre as placas |
| RF-17 | Ao sair do modo noturno, o sistema deve voltar ao modo normal com as duas vias fechadas: em até 1 s após o pedido na placa que tem o botão, e em até 2 s na outra, que só fica sabendo quando a primeira sai do pisca. | (dupla) | 5 saídas: a primeira luz dos veículos depois do pisca é o vermelho nas duas vias, em até 1,05 s após o toque na placa do botão e em até 2,1 s na outra |
| RF-18 | No modo noturno, o botão de travessia não deve ter efeito, nem durante o pisca nem depois dele. | (dupla) | Toques no botão de travessia das duas vias durante o pisca: o pisca continua dentro de RF-13 e RF-14, e, ao sair do modo, o primeiro verde dos veículos dura 3 s ± 1% |

### 1.4 Duas placas

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RF-19 | O sistema deve usar dois microcontroladores, um de cada integrante, cada um controlando uma via. As placas trocam sinais para que as vias se revezem: uma só abre depois de a outra fechar. | Enunciado §1 e §2 | Inspeção: duas placas ligadas, e o sistema completo só funciona com as duas. Na serial das duas, as vias abrem alternadamente |
| RF-20 | Só uma das placas deve ter o botão do modo noturno, e a outra deve seguir o modo dela. | (dupla) | Com o modo ligado e desligado 5 vezes, as duas placas entram e saem do pisca no mesmo ciclo |

## 2. Requisitos de segurança

São as regras que nunca podem ser quebradas, em nenhum modo e com qualquer falha das listadas. Um único instante de violação reprova o teste.

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RS-01 | Em cada via, o verde dos pedestres nunca deve estar aceso com a via aberta (veículos no verde ou no amarelo). | Enunciado §4 ("evitando conflito de LEDs") | Zero instantes de sobreposição na serial, em pelo menos 30 s de operação, cobrindo todos os modos e toques nos dois botões |
| RS-02 | Em cada semáforo, só uma luz deve estar acesa por vez. O amarelo conta como uma luz só, mesmo sendo feito com o vermelho e o verde do LED RGB. | Enunciado §1, §2 e Req. de Projeto 2 | Zero instantes com duas luzes do mesmo semáforo acesas no ensaio de RS-01 |
| RS-03 | Nas trocas, o semáforo que fecha deve fechar antes de o outro abrir. Um instante com os dois no vermelho é permitido; o contrário, não. Vale dentro de cada via e entre as duas vias. | (dupla) | Em todas as trocas do ensaio de RS-01, o vermelho de um aparece na serial antes do verde do outro. Entre as duas vias, a ordem é decidida pelo tempo do PC (RNF-04) |
| RS-04 | As duas vias nunca devem estar abertas (veículos no verde ou no amarelo) ao mesmo tempo. | (dupla) | Zero instantes com as duas vias abertas no ensaio de RS-01, comparando as seriais das duas placas pelo tempo do PC (RNF-04) |
| RS-05 | Com o fio VEZ, PARADO ou NOTURNO solto, ou com uma placa desligada, a placa que continua ligada deve respeitar RS-01 a RS-04. Ela pode parar o ciclo, mas só com o vermelho dos veículos aceso fixo ou no pisca do modo noturno, nunca com o verde. Parada no vermelho, ela deve avisar a espera na serial pelo menos a cada 10 s. O fio GND fica fora deste requisito: sem ele, os níveis entre as placas ficam indefinidos, e ele é condição de montagem. | (dupla) | Cada um dos fios VEZ, PARADO e NOTURNO solto no meio do ciclo, e cada placa desligada no meio do ciclo, uma falha de cada vez: nenhuma violação na placa ligada, e, se ela parar no vermelho, o aviso aparece na serial enquanto a falha durar |

## 3. Requisitos não funcionais

### 3.1 Implementação imposta pelo enunciado

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RNF-01 | Cada luz deve ser controlada por uma thread própria: três threads para os veículos e duas para os pedestres. | Enunciado §1, §2 e Req. de Projeto 1 | Revisão de código: uma thread por luz, e nenhuma thread acende a luz de outra |
| RNF-02 | A exclusão mútua entre as luzes de um mesmo semáforo deve usar mutex. | Enunciado §1, §2 e Req. de Projeto 2 | Revisão de código: um mutex por semáforo, travado por toda thread antes de acender sua luz |
| RNF-03 | Rotinas de interrupção não devem usar mutex. | Zephyr (mutex não pode ser usado em ISR) | Revisão de código: as ISRs só sinalizam semáforos (`k_sem_give`) |

### 3.2 Testabilidade

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RNF-04 | Cada placa deve registrar na serial (115200 bps) toda troca de luz, toda mudança de modo e todo toque de botão, com o tempo da placa em ms. Nos ensaios com duas placas, as duas seriais devem ser gravadas no mesmo PC, que acrescenta o próprio tempo a cada linha. O tempo da placa mede as durações dentro dela; o tempo do PC decide a ordem entre as placas, porque os relógios delas partem de instantes diferentes e se afastam. | Enunciado, Req. de Projeto 4 (evidências) | Num ciclo completo, cada troca de luz aparece uma vez na serial, no formato `tempo ms \| evento`. Com duas placas, cada linha das duas seriais tem também o tempo do PC |
| RNF-05 | O registro na serial não deve atrasar o ciclo além da tolerância de RF-03 e RF-04. | (dupla) | Mesmo ensaio de RF-03 e RF-04, com o registro ligado |
| RNF-06 | Deve haver um modo de placa única, que roda uma via (os dois semáforos dela) numa só placa, para testar antes de ligar a segunda. | (dupla) | O firmware compila com `-DPAPEL=placa_unica` e cumpre RF-01 a RF-18 para uma via |

### 3.3 Processo

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RNF-07 | Cada integrante deve fazer os próprios commits no repositório. | Enunciado, Req. de Projeto 7 | `git log` mostra commits dos dois autores, no desenvolvimento e nos testes |
| RNF-08 | Os planos e as evidências de teste (logs, figuras, vídeos) devem ficar no repositório, nos níveis unitário, de integração e de sistema. | Enunciado, Req. de Projeto 4 e 6 | Inspeção: [tests/plano-testes.md](../tests/plano-testes.md) e os anexos citados nele existem no repositório |

## 4. Rastreabilidade

Os testes de aceitação (TA) serão detalhados em [tests/plano-testes.md](../tests/plano-testes.md), com pré-condição, etapas e pós-condição. A última coluna aponta os testes da etapa 1 já registrados no [relatório da etapa 1](enunciado.md#relatório-de-testes-etapa-1-do-semáforo-uma-placa).

| Requisitos | Teste de aceitação | Evidência existente (etapa 1) |
|---|---|---|
| RF-01, RF-02 | TA-01: inspeção das luzes | T8 (pendente) |
| RF-03, RF-04, RNF-05 | TA-02: tempos do ciclo normal | T2 (aprovado, uma placa) |
| RF-05, RS-03 | TA-03: sincronismo e ordem das trocas | T3 (aprovado, uma placa) |
| RF-06 | TA-04: partida e reinício | T1 (aprovado, um boot) |
| RF-07 a RF-11 | TA-05: botão de travessia | T7 (pendente) |
| RF-12 | TA-06: botão do modo noturno | T5 (inconclusivo) |
| RF-13 a RF-15, RF-18 | TA-07: pisca do modo noturno | T6 (pendente) |
| RF-16, RF-17 | TA-08: entrada e saída do modo noturno | T4 (aprovado, uma placa) |
| RF-19, RF-20 | TA-09: operação com duas placas | — |
| RS-01, RS-02, RS-04 | TA-10: ensaio longo de segurança | T3 (parcial: 45 s, uma placa; RS-04 exige as duas) |
| RS-05 | TA-11: falhas de fiação e placa desligada | — |
| RNF-01 a RNF-03 | TA-12: revisão de código | — |
| RNF-04, RNF-06 | TA-13: registro na serial e modo de placa única | T0 (build aprovado) e T1 a T4 (usam o registro) |
| RNF-07, RNF-08 | TA-14: inspeção do repositório | — |
