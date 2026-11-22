# Plano de testes

Este plano detalha os testes de aceitação (TA) da tabela de rastreabilidade do [01-requisitos.md](../docs/01-requisitos.md). Cada teste tem pré-condição, etapas, resultado esperado (pós-condição) e a evidência que deve ficar no repositório. Os requisitos são citados pelo ID; o texto completo e o critério de cada um estão no 01.

## 1. Estratégia (modelo V)

Cada nível de teste confere um documento da descida do V:

| Nível | Confere | O que testa | Testes |
|---|---|---|---|
| Unitário | [03-projeto-componentes.md](../docs/03-projeto-componentes.md) | Cada semáforo e cada módulo, numa placa só (`-DPAPEL=placa_unica`) | TA-01, TA-02 parte A, TA-06, TA-12, TA-13 |
| Integração | [02-arquitetura.md](../docs/02-arquitetura.md) | As interfaces: veículos e pedestres da mesma placa, veículos e modo noturno, placa e placa | TA-02 parte B, TA-03, TA-08, TA-09 |
| Sistema | [01-requisitos.md](../docs/01-requisitos.md) | Os modos e os botões com o sistema completo, a segurança e as falhas | TA-04, TA-05, TA-07, TA-10, TA-11 |
| Processo | Enunciado, Req. de Projeto 4, 6 e 7 | Commits e evidências no repositório | TA-14 |

**Ordem de execução:** primeiro os unitários, com uma placa; depois a integração, com as duas placas; por fim, o sistema. Um teste de um nível só começa com os testes do nível anterior aprovados. Vários testes têm uma **parte A**, com uma placa, e uma **parte B**, com as duas.

## 2. Ambiente de teste

### 2.1 Hardware

- Duas FRDM-KL25Z montadas como na seção 1 do [03-projeto-componentes.md](../docs/03-projeto-componentes.md). Para as partes A, a montagem da etapa 1 (seção 1.5).
- Os botões são fios encostados no GND.
- As duas placas ligadas pelo USB (conector SDA) ao mesmo PC, que grava as seriais.

### 2.2 Firmware

Uma pasta de build por papel:

```bash
west build -b frdm_kl25z -d build/placa_unica -- -DPAPEL=placa_unica
```

```bash
west build -b frdm_kl25z -d build/mestre -- -DPAPEL=mestre
```

```bash
west build -b frdm_kl25z -d build/escrava -- -DPAPEL=escrava
```

Para gravar, deixar só a placa certa no USB e usar `west flash -d build/<papel>`. **Cada resultado registra o commit testado** (`git rev-parse --short HEAD`), e o firmware não pode ter mudanças sem commit.

### 2.3 Captura da serial

A [captura_serial.py](ferramentas/captura_serial.py) grava a serial de uma ou duas placas no mesmo arquivo, com o tempo do PC em cada linha:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-09/captura.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
```

- **Duração:** cada teste diz quanto capturar; vai de 20 s a 2 min. O tempo foi escolhido para ter o número de repetições do critério e nada mais. Uma captura por etapa, cada uma no próprio arquivo (`etapa1.log`, `etapa2.log`...), deixa a análise e a evidência mais fáceis de ler.
- **Formato:** `tempo do PC (s)`, TAB, `nome da placa`, TAB, `tempo da placa (ms) | evento`. O formato do evento está na seção 4 do [03-projeto-componentes.md](../docs/03-projeto-componentes.md).
- **Um relógio só:** as duas portas são lidas pelo mesmo programa, então o tempo do PC é o mesmo para as duas placas. O tempo da placa mede as durações dentro dela, e o tempo do PC decide a ordem entre as placas (RNF-04).
- **Notas:** cada linha digitada no terminal durante a captura entra no log como `nota`, com o tempo do PC. Use para marcar o que foi feito, por exemplo `tirei o fio VEZ`.
- **Porta que cai:** se uma placa for desligada, a captura registra a queda e reabre a porta sozinha quando a placa voltar.
- **No Windows,** as portas se chamam `COM3`, `COM4` etc. No WSL, a placa só aparece em `/dev/ttyACM*` depois de passada ao Linux pelo `usbipd`; o mais simples é rodar a captura direto no Windows.
- **Ctrl+C** encerra a captura.
- **A serial mostra o que o firmware comanda, e não o que o LED mostra.** Por isso o TA-01 confere as luzes em vídeo.

### 2.4 Análise

A [analisa_log.py](ferramentas/analisa_log.py) lê o log de uma ou de duas placas, mede tudo e confere os critérios do 01:

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-09/captura.log --relatorio tests/evidencias/TA-09/analise.md --figura tests/evidencias/TA-09/linha_do_tempo.png
```

| Seção do relatório | Requisitos | Usada em |
|---|---|---|
| Tempos do ciclo normal | RF-03, RF-04 | TA-02 |
| Sincronismo e segurança dentro de cada via | RF-05, RS-01, RS-03 | TA-03, TA-10 |
| Segurança entre as vias (só com duas placas) | RS-03, RS-04 | TA-03, TA-09, TA-10, TA-11 |
| Botão de travessia | RF-07 a RF-11 | TA-05 |
| Modo noturno | RF-13 a RF-17 | TA-07, TA-08 |
| Avisos de espera | RS-05 | TA-11 |

- **Faixas:** com uma placa no log, valem as da parte A; com duas, as da parte B. Os limites ficam no começo do arquivo, junto do requisito de cada um.
- **Fases excluídas:** fases afetadas por travessia, modo noturno, partida ou falha não entram na conferência dos tempos do ciclo normal. O relatório mostra quantas foram excluídas e por quê.
- **Resultado:** o relatório começa pela lista das conferências que falharam. O programa termina com código 0 se tudo passou e 1 se algo falhou.
- **Figura:** precisa do matplotlib. Sem ele, só o relatório é gerado.
- **Logs da etapa 1:** a análise também lê o formato antigo, sem o nome da placa. No [captura_ciclos.log](https://github.com/bmchad/uc-aula1-fork/blob/5890a32/docs/semaforo/captura_ciclos.log) da etapa 1 (commit `5890a32`), ela reproduz os números do relatório daquela etapa.

### 2.5 Evidências

- Cada teste guarda os anexos em `tests/evidencias/TA-xx/`: os logs (`.log`), as figuras e os vídeos. Vídeos grandes podem ficar fora do git, com o link no resultado.
- Cada execução gera um `tests/evidencias/TA-xx/README.md` com a data, o commit testado, quem executou, o resultado de cada etapa e a lista de anexos.
- **Quem executa faz o commit da evidência** (RNF-07).

## 3. Pré-requisitos

- **Python 3 com o pyserial** para a captura, e o matplotlib se quiser a figura. As versões estão em [ferramentas/requirements.txt](ferramentas/requirements.txt).
- **Firmware do commit em teste,** compilado sem avisos (TA-13).

Três bloqueios da primeira versão deste plano já foram resolvidos:

| # | Bloqueio | Solução |
|---|---|---|
| B1 | O pisca não saía na serial. | O firmware registra cada troca do pisca: `carros AMARELO aceso` ou `apagado`, e `pedestres VERMELHO aceso` ou `apagado`. |
| B2 | Toques de travessia sem efeito não saíam na serial. | O firmware registra todos os toques: `botao travessia: toque`. |
| B3 | Faltava gravar as duas seriais com o tempo do PC e analisar os dois logs juntos. | [captura_serial.py](ferramentas/captura_serial.py) e [analisa_log.py](ferramentas/analisa_log.py) (seções 2.3 e 2.4). |

## 4. Testes de aceitação

### TA-01: inspeção das luzes

- **Nível:** unitário.
- **Requisitos:** RF-01, RF-02.
- **Pré-condição:** cada placa montada como na seção 1.1 do 03-projeto-componentes.md, com `-DPAPEL=placa_unica`, e a captura da serial ligada.
- **Etapas:**
  1. Ligar a placa e filmar os LEDs durante um ciclo completo (8 s), com a captura da serial ligada.
  2. Repetir com a outra placa.
- **Resultado esperado:**
  - Veículos: verde, amarelo e vermelho distinguíveis a olho nu. O amarelo (vermelho e verde do RGB juntos) se distingue do vermelho e do verde.
  - Pedestres: verde e vermelho distinguíveis.
- **Evidência:** um vídeo e um log por placa.

### TA-02: tempos do ciclo normal

- **Nível:** parte A, unitário; parte B, integração.
- **Requisitos:** RF-03, RF-04, RNF-05.
- **Pré-condição:** captura da serial ligada, sem tocar em nenhum botão. Parte A: uma placa com `-DPAPEL=placa_unica`. Parte B: as duas placas ligadas pelos fios, com `-DPAPEL=mestre` e `-DPAPEL=escrava`.
- **Etapas:**
  1. Ligar e esperar o primeiro ciclo completo.
  2. Capturar pelo menos 6 ciclos seguidos: cerca de 50 s, com uma placa ou com duas. **Comece a captura com o sistema já rodando, sem apertar o RESET:** as fases da partida não entram na conta, e começar no boot gasta um ciclo à toa.
  3. Calcular a duração de cada fase completa pelo tempo da placa.
- **Resultado esperado (parte A),** em pelo menos 5 ciclos:

  | Fase | Faixa aceita |
  |---|---|
  | Veículos: verde | 2,97 a 3,03 s |
  | Veículos: amarelo | 0,99 a 1,01 s |
  | Veículos: vermelho | 3,96 a 4,04 s |
  | Pedestres: verde | 3,96 a 4,04 s |
  | Pedestres: vermelho | 3,96 a 4,04 s |

  Sequência esperada desde o reset:

  | Tempo | Veículos | Pedestres | Serial |
  |---|---|---|---|
  | 0 a 4 s | vermelho | verde | `carros VERMELHO`, `pedestres VERDE` |
  | 4 a 7 s | verde | vermelho | `pedestres VERMELHO`, `carros VERDE` |
  | 7 a 8 s | amarelo | vermelho | `carros AMARELO` |

- **Resultado esperado (parte B),** nas duas vias: verde e amarelo dos veículos e vermelho dos pedestres como na parte A; vermelho dos veículos e verde dos pedestres de **4,00 a 4,20 s**, por causa do handshake. Nenhuma fase pulada e a ordem não muda.
- **Evidência:** os logs e a tabela de durações.

### TA-03: sincronismo e ordem das trocas

- **Nível:** integração.
- **Requisitos:** RF-05, RS-03.
- **Pré-condição:** os logs do TA-02 (partes A e B).
- **Etapas:**
  1. Em cada troca de um semáforo, medir pelo tempo da placa o atraso até a troca do outro semáforo da mesma via.
  2. Conferir a ordem dentro da via: o verde dos pedestres só depois do vermelho dos veículos, e o verde dos veículos só depois do vermelho dos pedestres.
  3. Na parte B, conferir a ordem entre as vias pelo tempo do PC: o `carros VERMELHO` de uma via antes do `carros VERDE` da outra.
- **Resultado esperado:** atraso de até 50 ms em todas as trocas da mesma via, e nenhuma troca fora de ordem. Entre as vias, a folga esperada é de pelo menos 50 ms (`GUARDA_MS`), bem acima da variação do USB, então a ordem pelo tempo do PC é confiável.
- **Evidência:** a tabela de atrasos e a linha do tempo.

### TA-04: partida e reinício

- **Nível:** sistema.
- **Requisito:** RF-06.
- **Pré-condição:** as duas placas gravadas e ligadas pelos fios, com a captura das duas seriais.
- **Etapas:**
  1. **Partida:** ligar as duas placas ao mesmo tempo, ou uma logo depois da outra. Repetir 5 vezes.
  2. **Reinício:** com o sistema rodando, apertar o botão RESET de uma placa. Fazer 5 reinícios em cada placa, em fases diferentes: verde, amarelo e vermelho da própria via.
- **Resultado esperado:**
  - Partida: a primeira linha `carros` das duas placas é `VERMELHO`, e o ciclo começa sem nenhum comando.
  - Reinício: a primeira linha `carros` da placa reiniciada é `VERMELHO`, e o sistema volta a se revezar sem nenhum comando. Em nenhum momento as duas vias ficam abertas (RS-04).
- **Observação:** o RESET da placa não derruba a porta serial, mas tirar o USB derruba. A captura reabre a porta sozinha quando a placa volta.
- **Evidência:** os logs das duas placas.

### TA-05: botão de travessia

- **Nível:** sistema.
- **Requisitos:** RF-07 a RF-11, RF-21.
- **Pré-condição:** parte A com `-DPAPEL=placa_unica`; parte B com as duas placas, com o botão na escrava. Captura da serial ligada.
- **Etapas:**
  1. **RF-07:** dar um toque em 5 instantes diferentes do verde, entre 1 s e 3 s depois de ele acender.
  2. **RF-08:** dar um toque logo no começo do verde, nos primeiros 0,5 s. Repetir 5 vezes.
  3. **RF-09:** usar as medidas das etapas 1 e 2.
  4. **RF-10:** dar 5 toques durante o vermelho e 5 durante o amarelo.
  5. **RF-11:** dar 5 toques rápidos no mesmo verde.
  6. **RF-21 (parte B):** encostar 5 vezes no GND o D10 do mestre, que não tem botão, em fases diferentes.
- **Resultado esperado:**
  - **RF-07:** em todos os toques, `carros travessia pedida: verde encurtado`, depois `carros AMARELO` (0,99 a 1,01 s) e então `pedestres VERDE`.
  - **RF-08:** o verde dura de 1,00 a 1,05 s.
  - **RF-09:** atraso da linha `botao travessia: toque` até o `pedestres VERDE` de no máximo 2,05 s.
  - **RF-10:** cada toque aparece como `botao travessia: toque`, sem nenhuma linha `travessia pedida`, e as durações ficam dentro das faixas do TA-02.
  - **RF-11:** uma única linha `travessia pedida` e uma única passagem pelo amarelo. O verde seguinte dura 2,97 a 3,03 s.
  - **RF-21:** nenhuma linha `botao travessia: toque` na serial do mestre, e as durações do mestre dentro das faixas do TA-02.
- **Observação (parte B):** um toque na via B encurta o vermelho da via A. Isso é esperado e não reprova o TA-02.
- **Observação (RF-11):** na serial, toques a menos de 0,2 s um do outro aparecem como um só. Para contar os 5 toques, espaçe-os um pouco mais que isso.
- **Capturas:** uma por etapa. Cada verde aceita só uma travessia, e um ciclo leva cerca de 8 s, então as etapas 1 e 2 levam cerca de 45 s cada; as outras, cerca de 20 s.
- **Evidência:** os logs e o relatório da análise.

### TA-06: botão do modo noturno

- **Nível:** unitário.
- **Requisito:** RF-12.
- **Pré-condição:** `-DPAPEL=placa_unica`, captura ligada e vídeo do fio no D9.
- **Etapas:**
  1. Dar 10 toques curtos no D9, separados por pelo menos 0,5 s, contando em voz alta.
  2. Em outra captura, dar 5 toques longos, segurando o fio no GND por 2 s.
- **Resultado esperado:** exatamente 10 linhas `botao noturno: pedido ...` na etapa 1 e 5 na etapa 2, alternando entre `NOTURNO` e `NORMAL`. Se aparecerem alternâncias a mais, é repique, e o `DEBOUNCE_MS` precisa subir.
- **Evidência:** o log e o vídeo.
- **Etapa 1:** T5, inconclusivo, porque não foram contados os toques.

### TA-07: pisca do modo noturno

- **Nível:** sistema.
- **Requisitos:** RF-13, RF-14, RF-15, RF-18.
- **Pré-condição:** parte A com `-DPAPEL=placa_unica`; parte B com as duas placas. Captura da serial ligada.
- **Etapas:**
  1. Começar a captura e pedir o modo noturno. O pisca começa em até 8,4 s.
  2. Manter o pisca por pelo menos 10 s, dando 5 toques no botão de travessia durante ele (na escrava, na parte B).
  3. Sair do modo noturno e esperar o primeiro verde dos veículos terminar.

  Tudo numa captura só, de cerca de 35 s.
- **Resultado esperado:**
  - **RF-13 e RF-14:** cada meio período do amarelo dos veículos e do vermelho dos pedestres dura de 0,99 a 1,01 s. Nenhuma linha `carros VERDE`, `carros VERMELHO` ou `pedestres VERDE` durante o pisca.
  - **RF-15:** em cada via, as trocas do pisca dos veículos e dos pedestres ficam a até 50 ms uma da outra.
  - **RF-18:** os toques aparecem como `botao travessia: toque`, sem nenhuma linha `travessia pedida`, e o primeiro verde depois da saída dura de 2,97 a 3,03 s.
- **Evidência:** os logs e o relatório da análise.

### TA-08: entrada e saída do modo noturno

- **Nível:** integração.
- **Requisitos:** RF-16, RF-17.
- **Pré-condição:** parte A com `-DPAPEL=placa_unica`; parte B com as duas placas. Captura ligada.
- **Etapas:**
  1. **Entrada:** pedir o modo noturno uma vez em cada fase da via do botão: verde, amarelo e vermelho. Sair do modo entre uma tentativa e a outra.
  2. **Saída:** sair do modo noturno 5 vezes, depois de pelo menos 4 s de pisca. Cada etapa cabe numa captura de cerca de 1 min.
- **Resultado esperado:**
  - **Entrada:** a linha `modo NOTURNO` aparece só no fim de um vermelho dos veículos, até 8,4 s depois da linha `botao noturno: pedido NOTURNO`. Na parte B, as duas vias estão fechadas quando o pisca começa, e a escrava começa a piscar também em até 8,4 s.
  - **Saída:** depois de `botao noturno: pedido NORMAL`, a primeira luz dos veículos é o vermelho: em até 1,05 s na placa do botão e, na parte B, em até 2,1 s na escrava, comparando pelo tempo do PC.
- **Evidência:** os logs, com uma tabela dos atrasos.
- **Etapa 1:** T4, aprovado com uma placa.

### TA-09: operação com duas placas

- **Nível:** integração.
- **Requisitos:** RF-19, RF-20, RS-06.
- **Pré-condição:** uma placa gravada com `-DPAPEL=mestre` e a outra com `-DPAPEL=escrava`, com a captura das seriais.
- **Etapas:**
  1. **Laço na própria placa:** sem a outra placa, ligar o D12 no D11 da mesma placa, pelo resistor de 1 kΩ, e capturar 20 s. Fazer nas duas placas. Isso confere o TX, o RX e os pinos de cada uma antes de ligar as duas.
  2. **Escrava sozinha:** sem nenhum fio entre as placas, ligar só a escrava e capturar 25 s desde o boot, para pegar dois avisos.
  3. **Mestre sozinho:** sem nenhum fio entre as placas, ligar só o mestre e capturar 25 s desde o boot, para pegar dois avisos.
  4. **Revezamento:** ligar os três fios (seção 1.4 do 03), na ordem GND primeiro, espere alguns segundos e capture pelo menos 6 ciclos das duas vias, cerca de 50 s.
  5. **Modo noturno nas duas:** ligar e desligar o modo noturno 5 vezes pelo botão do mestre, numa captura de cerca de 1 min e meio.
- **Resultado esperado:**
  1. A placa registra `enlace: quadro com o mesmo papel desta placa` e nunca `enlace conectado`. Os veículos ficam no vermelho. Se essa linha não aparecer, o TX, o RX ou o fio estão errados, e não adianta ligar as duas placas.
  2. A escrava fica com os veículos no vermelho e os pedestres no verde, registra `aguardando a vez do mestre` a cada 10 s e nunca `enlace conectado`.
  3. O mestre fica com os veículos no vermelho e os pedestres no verde e registra `aguardando a escrava parar` a cada 10 s.
  4. As duas placas registram `enlace conectado` logo depois do boot. Pelo tempo do PC, as vias abrem alternadamente: A, B, A, B. Uma via só abre depois de a outra fechar.
  5. As duas placas entram e saem do pisca no mesmo ciclo, todas as 5 vezes.
- **Evidência:** os logs de cada etapa.

### TA-10: ensaio de segurança

- **Nível:** sistema.
- **Requisitos:** RS-01, RS-02, RS-04.
- **Pré-condição:** as duas placas ligadas pelos fios, com a captura das duas seriais.
- **Etapas:**
  1. Capturar 2 minutos de operação, cobrindo: o modo normal; toques de travessia na escrava, em todas as fases; e pelo menos duas entradas e saídas do modo noturno. O critério mínimo do 01 é 30 s; 2 minutos dão uns 14 ciclos, o bastante para passar por todas as combinações.
  2. Rodar a análise e procurar violações.
- **Resultado esperado:** zero instantes com:
  - o verde dos pedestres aceso com os veículos da mesma via no verde ou no amarelo (RS-01);
  - as duas vias abertas, comparando pelo tempo do PC (RS-04).
- **Limite da evidência para o RS-02:** a serial registra quando cada luz acende, mas não quando apaga. Ela mostra a ordem, mas não prova que a luz anterior apagou. O RS-02 é conferido pelo vídeo do TA-01 e pela revisão do mutex no TA-12.
- **Evidência:** os logs, a linha do tempo das duas vias e a contagem de violações.

### TA-11: falhas do enlace e placa desligada

- **Nível:** sistema.
- **Requisitos:** RS-05, RS-06.
- **Pré-condição:** as duas placas rodando no modo normal, com a captura das duas seriais.
- **Etapas:** provocar cada falha abaixo **no meio do ciclo**, uma de cada vez. Fazer cada uma duas vezes: com a via A aberta e com a via B aberta. Numa captura só: provocar a falha, mantê-la por 25 s (para pegar dois avisos), desfazê-la e esperar o sistema voltar a se revezar. Cada caso leva cerca de 45 s. Digitar uma nota na captura no instante de cada falha, por exemplo `tirei o fio 2`.

  | Falha | Como provocar | Comportamento esperado (02-arquitetura, seção 2.5) |
  |---|---|---|
  | Fio TX do mestre solto | Tirar o fio 2 | A escrava registra `enlace perdido`, fica no vermelho e avisa; o mestre cancela a vez e segue sozinho |
  | Fio TX da escrava solto | Tirar o fio 3 | O mestre registra `enlace perdido`; as duas vias ficam no vermelho, e as duas placas avisam |
  | Ruído no fio de dados | Encostar e soltar o fio 2 do GND várias vezes, por 5 s | Nenhuma via abre fora de hora. Pode aparecer `enlace perdido` e depois `enlace conectado` |
  | Escrava desligada | Tirar o USB da escrava | O mestre registra `enlace perdido` e para no vermelho, avisando |
  | Mestre desligado | Tirar o USB do mestre | A escrava registra `enlace perdido`, termina a fase e fica no vermelho, avisando |

- **Resultado esperado:** em todos os casos, nenhuma violação de RS-01 a RS-04 na placa que continua ligada. Ela só para com o vermelho dos veículos aceso fixo ou no pisca, nunca no verde. Parada no vermelho, ela registra um aviso pelo menos a cada 10 s. Depois de desfeita a falha, o sistema volta a se revezar. O caso de quadro com o mesmo papel (RS-06) é a etapa 1 do TA-09.
- **Depois do teste:** se algum caso se comportar diferente da seção 2.5 do [02-arquitetura.md](../docs/02-arquitetura.md), atualizar a tabela com o comportamento real.
- **Evidência:** os logs de cada caso, com as notas.

### TA-12: revisão de código

- **Nível:** unitário.
- **Requisitos:** RNF-01, RNF-02, RNF-03 e a revisão de código do RF-21.
- **Pré-condição:** o commit a revisar. **Quem revisa é o integrante que não escreveu o trecho.**
- **Etapas:** preencher a lista abaixo, citando arquivo e linha de cada item.

  | Item | Confere |
  |---|---|
  | 1 | Há uma thread por luz: três dos veículos e duas dos pedestres (RNF-01) |
  | 2 | Cada thread só escreve no pino da própria luz. A do amarelo escreve nos dois pinos do RGB, e só ela faz isso. As threads auxiliares (botão noturno e registro da travessia) não escrevem em nenhum LED. |
  | 3 | Há um mutex por semáforo, travado antes de acender a luz e destravado depois de apagar, em todos os caminhos, inclusive no pisca (RNF-02) |
  | 4 | As interrupções dos dois botões só chamam `k_sem_give`, e a da UART2 só copia o byte recebido para a fila com `k_msgq_put` sem espera (RNF-03) |
  | 5 | Nenhuma rotina de interrupção usa mutex, `printk` ou espera |
  | 6 | O mestre não configura o botão de travessia: `semaforo_veiculo_iniciar(false)` (RF-21) |

- **Resultado esperado:** todos os itens conferidos.

### TA-13: registro na serial e modo de placa única

- **Nível:** unitário.
- **Requisitos:** RNF-04, RNF-06.
- **Pré-condição:** o commit a testar.
- **Etapas:**
  1. Compilar os três papéis (seção 2.2) e conferir que não há erros nem avisos.
  2. Gravar `placa_unica` e conferir a mensagem `Semaforo iniciado como PLACA UNICA (etapa 1)`.
  3. Num ciclo completo, conferir que cada troca de luz aparece uma vez, no formato `tempo ms | evento`.
  4. Na parte B de qualquer teste, conferir que cada linha das duas seriais tem o tempo do PC.
- **Resultado esperado:** todas as etapas conferidas. O RNF-06 também exige que a parte A de TA-01 a TA-08 passe com `placa_unica`.
- **Evidência:** a saída dos três builds e o log do boot.

### TA-14: inspeção do repositório

- **Nível:** processo.
- **Requisitos:** RNF-07, RNF-08.
- **Pré-condição:** o repositório no fim do trabalho.
- **Etapas:**
  1. Listar os autores dos commits com `git shortlog -sn`.
  2. Conferir que cada integrante tem commits de desenvolvimento e de testes.
  3. Conferir que todos os anexos citados neste plano e nos `README.md` de `tests/evidencias/` existem no repositório, ou têm um link que funciona.
- **Resultado esperado:** os dois autores aparecem nos dois tipos de commit, e nenhum anexo falta.
- **Evidência:** a saída do `git shortlog` e a lista conferida.

## 5. Situação atual

| Teste | Nível | Situação | Evidência existente |
|---|---|---|---|
| TA-01 | Unitário | Aprovado | - |
| TA-02 | Unitário e integração | Pendente | - |
| TA-03 | Integração | Pendente | - |
| TA-04 | Sistema | Pendente | - |
| TA-05 | Sistema | Pendente | — |
| TA-06 | Unitário | Pendente | - |
| TA-07 | Sistema | Pendente | — |
| TA-08 | Integração | Pendente | - |
| TA-09 | Integração | Pendente | — |
| TA-10 | Sistema | Pendente | - |
| TA-11 | Sistema | Pendente | — |
| TA-12 | Unitário | Pendente | — |
| TA-13 | Unitário | Pendente | — |
| TA-14 | Processo | Pendente | — |

Os testes T0 a T8 são os da etapa 1, registrados no [relatório da etapa 1](../docs/enunciado.md#relatório-de-testes-etapa-1-do-semáforo-uma-placa). Eles foram feitos no commit `8ae444e`, antes das mudanças da etapa 2. Antes da entrega, as partes A aprovadas devem ser refeitas no commit final.
