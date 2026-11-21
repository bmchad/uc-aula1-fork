## 1. Hardware de cada placa

As duas placas são FRDM-KL25Z com a mesma fiação; só o botão do modo noturno é exclusivo do mestre. Os pinos do código estão no [frdm_kl25z.overlay](../frdm_kl25z.overlay), que o Zephyr aplica sozinho porque tem o nome da placa. Este documento é a referência para a montagem.

### 1.1 Pinos

| Função | Pino | Conector | Nó no devicetree | Ativo em | Ligação |
|---|---|---|---|---|---|
| Veículos: vermelho | PTB18 | (LED RGB) | alias `led2` | baixo | Já ligado na placa |
| Veículos: verde | PTB19 | (LED RGB) | alias `led0` | baixo | Já ligado na placa |
| Veículos: amarelo | PTB18 + PTB19 | — | — | baixo | Os dois acesos juntos |
| Pedestres: vermelho | PTC8 | D6 | `ped_vermelho` | alto | D6 → resistor de 220 Ω → perna longa (anodo) do LED vermelho; perna curta (catodo) → GND |
| Pedestres: verde | PTC9 | D7 | `ped_verde` | alto | D7 → resistor de 220 Ω → perna longa do LED verde; perna curta → GND |
| Botão de travessia | PTD0 | D10 | `botao_travessia` | baixo | Um fio no D10; para apertar, encostar a outra ponta no GND |
| Botão do modo noturno | PTD5 | D9 | `botao_noturno` | baixo | **Só no mestre.** Um fio no D9, encostado no GND |
| Enlace: UART2 TX | PTD3 (ALT3) | D12 (J2, pino 10) | `uart2` | — | Fio para o D11 da outra placa (seção 1.4) |
| Enlace: UART2 RX | PTD2 (ALT3) | D11 (J2, pino 8) | `uart2` | — | Fio do D12 da outra placa. Pull-up interno: com o fio solto, fica em repouso. |

- **Polaridade:** o código usa a polaridade do devicetree, então "1" sempre quer dizer aceso ou apertado, qualquer que seja a tensão no pino.
- **Porta C:** vem desabilitada na placa. O overlay a habilita para os LEDs de pedestres.
- **Entradas com interrupção:** os dois botões ficam na porta D (D9 e D10), porque na KL25Z só as portas A e D geram interrupção por pino.
- **UART2:** o Zephyr não descreve a UART1 nem a UART2 da KL25Z. O nó da UART2 está no overlay, a partir do manual do chip: base em `0x4006C000`, IRQ 14, relógio do barramento com a porta no bit 12 do `SIM_SCGC4`.
- **Pull-up:** os botões e o RX da UART2 usam o pull-up interno. A KL25Z não tem pull-down nesses pinos.

### 1.2 LEDs e resistores

- **Resistor de 220 Ω** em série com cada LED avulso. A corrente fica em até cerca de 6,4 mA no vermelho (queda de ~1,9 V) e 5,5 mA no verde comum (~2,1 V).
- O PTC8 e o PTC9 são pinos de corrente normal. O datasheet garante a tensão de saída até 5 mA, e o limite absoluto é de 25 mA por pino. Acima de 5 mA a tensão no pino cai alguns décimos de volt, o que também segura a corrente.
- **Não usar menos de 220 Ω.** Um verde do tipo de 3 V acende mais fraco, mas acende.
- Um LED invertido não acende, mas não queima.

### 1.3 Pinos que não devem ser usados

| Conector | Pino | Motivo |
|---|---|---|
| D0, D1 | PTA1, PTA2 | Console serial (UART0), usado pelo registro |
| D4 | PTA4 | É o NMI. Se estiver em 0 no reset, a placa trava. |
| D13 | PTD1 | LED azul da placa |

### 1.4 Fios entre as placas

| Fio | De | Para | Função |
|---|---|---|---|
| 1 | GND (J2, pino 14) | GND (J2, pino 14) | Referência comum |
| 2 | D12 do mestre (J2, pino 10; UART2 TX) | D11 da escrava (J2, pino 8; UART2 RX) | Quadros do mestre |
| 3 | D12 da escrava (J2, pino 10; UART2 TX) | D11 do mestre (J2, pino 8; UART2 RX) | Quadros da escrava |

- **Os sinais Arduino ficam nos pinos pares do J2,** e os nomes D11 e D12 geralmente não estão impressos na placa. Não ligue nada na fileira de pinos ímpares.
- **Resistor de 1 kΩ em série** em cada fio de dados (fios 2 e 3). Ele limita a corrente se dois TX forem ligados juntos por engano e a corrente que vai para uma placa desligada. Qualquer valor de 470 Ω a 2,2 kΩ serve.
- **O GND é o primeiro fio a ligar.** Sem ele, a UART não tem referência.
- **Fios 2 e 3** são a mesma ligação nos dois sentidos: TX (D12) de uma placa no RX (D11) da outra. Trocar o papel das placas não muda a fiação.
- **Teste da fiação de cada placa, antes de ligar a outra:** ligue o D12 no D11 da própria placa, pelo resistor. Com o firmware de mestre ou de escrava, a serial deve mostrar `enlace: quadro com o mesmo papel desta placa`. Isso confirma o TX, o RX e os pinos certos.
- O formato do quadro e o protocolo estão na seção 2 do [02-arquitetura.md](02-arquitetura.md).

### 1.5 Montagem da etapa 1 (placa única)

| O que | Ligação | Obrigatório? |
|---|---|---|
| USB | Conector **SDA** da placa (gravação e serial) | Sim |
| LED verde dos pedestres | D7, como na seção 1.1 | Sim |
| LED vermelho dos pedestres | D6, como na seção 1.1 | Não. Sem ele, o vermelho dos pedestres aparece só na serial. |
| Botão de travessia | Fio no D10 | Não |
| Botão do modo noturno | Fio no D9 | Não |

- **GND:** o pino ao lado do D13, no conector do D8 ao D13, ou qualquer GND do conector de alimentação.
- **Com um LED só, use o verde no D7.** Ele mostra direto a regra de segurança: o verde dos pedestres só pode acender com o LED RGB no vermelho.
- **Monte com o USB desligado.**

## 2. Configuração do build

| Arquivo | Conteúdo |
|---|---|
| [CMakeLists.txt](../CMakeLists.txt) | Escolhe o papel pela variável `PAPEL` (`mestre`, `escrava` ou `placa_unica`; padrão `mestre`), compila as quatro bibliotecas de `lib/` e, na placa única, define `UMA_PLACA=1`. Um valor inválido para a compilação com erro. |
| [prj.conf](../prj.conf) | Liga o GPIO, o console na UART0 (`CONFIG_GPIO`, `CONFIG_SERIAL`, `CONFIG_CONSOLE`, `CONFIG_UART_CONSOLE`) e a UART por interrupção, para o enlace (`CONFIG_UART_INTERRUPT_DRIVEN`). |
| [frdm_kl25z.overlay](../frdm_kl25z.overlay) | Pinos da seção 1.1, habilitação da porta C, nó da UART2 com os pinos PTD2 e PTD3, e o alias `enlace-uart`. |

Compilar e gravar, com uma pasta de build por papel:

```bash
west build -b frdm_kl25z -d build/mestre -- -DPAPEL=mestre
```

```bash
west flash -d build/mestre
```

## 3. Componentes de software

Convenções comuns aos quatro módulos:
- As threads são criadas com `K_THREAD_DEFINE`, com pilha de 768 bytes, e começam bloqueadas num `k_sem`. Nenhuma faz nada antes da função `_iniciar` do seu módulo.
- As funções `_iniciar` retornam 0 ou um código de erro negativo do Zephyr (`-ENODEV` se um GPIO não está pronto). O arquivo principal para no primeiro erro e registra `Erro N ao iniciar o semaforo`.
- As interrupções só fazem `k_sem_give` (RNF-03).

### 3.1 `lib/enlace`

**Responsabilidade:** trocar o estado com a outra placa pela UART2. Não sabe nada de semáforo, só de três sinais lógicos: SINAL (VEZ ou PARADO, conforme o papel), NOTURNO e o papel de quem mandou.

**Constantes:**

| Constante | Valor | Uso |
|---|---|---|
| `BYTE_INICIO` | `0xA5` | Primeiro byte de todo quadro |
| `PERIODO_MS` | 50 ms | Intervalo de reenvio do estado, mesmo sem mudança |
| `PRAZO_MS` | 200 ms | Sem quadro válido por esse tempo: enlace perdido |
| `TAMANHO_FILA` | 64 bytes | Bytes recebidos à espera da `t_enlace_rx` |
| `PRIORIDADE_ENLACE` | 5 | A mesma das luzes, para as linhas da serial não se misturarem |

**Quadro** (4 bytes, 115200 bps, 8 bits, sem paridade, 1 bit de parada):

| Byte | Conteúdo |
|---|---|
| 0 | `0xA5` |
| 1 | Bit 0: SINAL (VEZ no mestre, PARADO na escrava). Bit 1: NOTURNO (só o mestre). Bit 2: MESTRE (1 se quem mandou é o mestre). Bits 3 a 7: sempre 0. |
| 2 | Contador, que aumenta a cada quadro (só para depuração) |
| 3 | `~(byte0 + byte1 + byte2)`, em 8 bits |

**Dados internos:**

| Nome | Tipo | Uso |
|---|---|---|
| `mestre`, `sozinha` | `bool` | Papel da placa. A placa única age como mestre e não usa a UART. |
| `local` | `atomic_t` | Bits que esta placa manda |
| `remoto` | `atomic_t` | Bits do último quadro válido da outra placa; 0 sem enlace |
| `conectado` | `atomic_t` | 1 enquanto chegam quadros válidos |
| `fila_rx` | `k_msgq` de 64 bytes | Interrupção da UART2 → `t_enlace_rx` |
| `sem_mudanca` | `k_sem`, limite 1 | `t_enlace_rx` → quem espera a outra placa |
| `sem_envio` | `k_sem`, limite 1 | Mudança local → `t_enlace_tx` manda na hora |

**Interrupção da UART2:** lê cada byte recebido e o coloca na `fila_rx` com `k_msgq_put` sem espera. Se a fila estiver cheia, o byte se perde, e o quadro dele falha na verificação.

**`t_enlace_rx`:**
1. Espera um byte da fila por até `PERIODO_MS` e o junta aos três anteriores.
2. Se os quatro formam um quadro válido (começa com `0xA5`, só usa os bits 0 a 2, verificação certa):
   - com o papel oposto ao desta placa: guarda SINAL e NOTURNO em `remoto`, anota a hora e, se algo mudou, dá `sem_mudanca`. Na primeira vez, registra `enlace conectado`;
   - com o mesmo papel: descarta o quadro e registra, uma vez só, `enlace: quadro com o mesmo papel desta placa`.
3. Se o enlace estava conectado e o último quadro válido tem mais de `PRAZO_MS`, zera `remoto`, registra `enlace perdido` e dá `sem_mudanca`.

**`t_enlace_tx`:** manda o quadro com os bits de `local` e espera `sem_envio` por até `PERIODO_MS`. Assim manda logo que o estado muda e repete a cada 50 ms.

**Funções:**

| Função | Comportamento |
|---|---|
| `enlace_iniciar(papel)` | Na placa única, só guarda o papel. Nos outros papéis, começa no estado seguro (mestre com VEZ = 0, escrava com PARADO = 1, NOTURNO = 0), instala a interrupção da UART2 e começa as duas threads. |
| `enlace_sou_mestre()`, `enlace_uma_placa()` | Devolvem o papel. |
| `enlace_publicar(valor)` | Muda o SINAL desta placa (VEZ no mestre, PARADO na escrava) e pede o envio imediato. |
| `enlace_ler()` | SINAL da outra placa: PARADO no mestre, VEZ na escrava. Vale 0 sem enlace. |
| `enlace_esperar(nivel, prazo)` | Espera o SINAL da outra placa chegar ao nível pedido. Depois de cada aviso, **lê o nível de novo**, então um aviso atrasado ou repetido não causa decisão errada. Retorna 0, ou `-EAGAIN` se o prazo acabou. |
| `enlace_aguardar_mudanca(prazo)` | Zera os avisos antigos e espera qualquer mudança na outra placa a partir de agora. Retorna `true` se houve mudança. |
| `enlace_publicar_noturno(ligado)`, `enlace_ler_noturno()` | Bit NOTURNO: o mestre manda, a escrava lê. Vale 0 sem enlace. |

### 3.2 `lib/modo_noturno`

**Responsabilidade:** guardar o **pedido** do modo noturno e o **modo efetivo**, e tratar o botão.

**Constantes:**

| Constante | Valor | Uso |
|---|---|---|
| `DEBOUNCE_MS` | 50 ms | Espera depois da borda, antes de conferir o botão, e depois de soltar |
| `LEITURA_SOLTO_MS` | 20 ms | Intervalo da consulta enquanto o botão continua apertado |
| `PRIORIDADE_BOTAO` | 4 | Acima das threads das luzes (5) |

**Dados internos:** `pedido_botao` e `ativo`, protegidos por `mutex_modo`; `com_botao`, que é `true` só no mestre; `sem_toque`, dado pela interrupção do botão.

**Thread do botão** (só age no mestre):
1. Espera `sem_toque`, dado pela interrupção na borda de aperto.
2. Espera `DEBOUNCE_MS` e confere se o botão continua apertado. Se não continuar, foi repique, e o toque é descartado.
3. Inverte `pedido_botao`, com o mutex travado, e registra `botao noturno: pedido NOTURNO` ou `pedido NORMAL`.
4. Espera o botão ser solto, consultando a cada `LEITURA_SOLTO_MS`, para um toque longo não alternar de novo. Depois espera mais `DEBOUNCE_MS`.
5. Descarta os avisos que a interrupção deu durante o toque e volta ao passo 1.

**Funções:**

| Função | Comportamento |
|---|---|
| `modo_noturno_iniciar(usar_botao)` | No mestre (`true`), configura o botão e a interrupção. Na escrava (`false`), não configura nada, e a thread do botão fica bloqueada para sempre. |
| `modo_noturno_pedido()` | No mestre, devolve `pedido_botao`. Na escrava, lê o bit NOTURNO do enlace. |
| `modo_noturno_aplicar(ligado)` | Muda o modo efetivo e registra `modo NOTURNO` ou `modo NORMAL`. No mestre, também manda o bit NOTURNO pelo enlace. Só o semáforo de veículos chama, no ponto seguro. |
| `modo_noturno_ativo()` | Devolve o modo efetivo. As threads das luzes usam para decidir se piscam. |

### 3.3 `lib/semaforo_pedestres`

**Responsabilidade:** as duas luzes dos pedestres. O módulo não tem tempo próprio no modo normal: os pedestres abrem e fecham quando os veículos mandam.

**Primitivas:**

| Nome | Tipo | Uso |
|---|---|---|
| `mutex_pedestres` | `k_mutex` | Travado por cada thread enquanto a sua luz está acesa |
| `vez_ped_vermelho`, `vez_ped_verde` | `k_sem` | Ordem entre as duas threads |
| `liberado` | `k_sem` | Os veículos pararam: o verde pode abrir |
| `encerrar` | `k_sem` | Pedido para fechar o verde |
| `encerrado` | `k_sem` | O vermelho dos pedestres acendeu |

**`t_ped_vermelho`:**
1. Espera `vez_ped_vermelho` e trava o mutex.
2. Acende o vermelho, registra `pedestres VERMELHO` e dá `encerrado`.
3. Espera `liberado` com prazo de `PISCA_MS` (1 s). A cada prazo vencido, se o modo noturno está ativo, inverte o vermelho; senão, o mantém aceso. Cada troca é registrada como `pedestres VERMELHO aceso` ou `pedestres VERMELHO apagado`.
4. Ao receber `liberado`, apaga o vermelho, destrava o mutex e dá `vez_ped_verde`.

**`t_ped_verde`:**
1. Espera `vez_ped_verde` e trava o mutex.
2. Acende o verde e registra `pedestres VERDE`.
3. Espera `encerrar`, apaga o verde, destrava o mutex e dá `vez_ped_vermelho`.

**Funções:**

| Função | Comportamento |
|---|---|
| `semaforo_pedestres_iniciar()` | Configura os dois LEDs apagados e dá `vez_ped_vermelho`: os pedestres começam no vermelho. |
| `semaforo_pedestres_liberar()` | Dá `liberado`. Só pode ser chamada com o vermelho dos veículos aceso. |
| `semaforo_pedestres_encerrar()` | Zera `encerrado`, dá `encerrar` e **só retorna com o vermelho dos pedestres aceso**. Zerar antes garante que vale só o aviso que vier depois do pedido. |

Cada `liberar` precisa ser seguido de um `encerrar` antes de os veículos abrirem. Esse contrato é o que garante o RS-01 (seção 1.3 do [02-arquitetura.md](02-arquitetura.md)).

### 3.4 `lib/semaforo_veiculo`

**Responsabilidade:** as três luzes dos veículos, o botão de travessia, o ponto seguro do modo noturno e o handshake com a outra placa.

**Constantes:**

| Constante | Valor | Uso |
|---|---|---|
| `VERDE_MS` | 3000 ms | Duração do verde |
| `VERDE_MIN_MS` | 1000 ms | Verde mínimo, mesmo com travessia (RF-08) |
| `AMARELO_MS` | 1000 ms | Duração do amarelo |
| `VERMELHO_MS` | 4000 ms | Vermelho na placa única |
| `PISCA_MS` | 1000 ms | Meio período do pisca |
| `GUARDA_MS` | 50 ms | Folga para a outra placa reagir a um sinal |
| `RESPOSTA_MS` | 3000 ms | Prazo do mestre para a escrava pegar a vez |
| `ESPERA_CANCELADA_MS` | 5000 ms | Espera do mestre depois de cancelar a vez: verde + amarelo + 1 s de margem |
| `AVISO_MS` | 10000 ms | Intervalo dos avisos de espera longa na serial |
| `ESTAVEL_MS` | 1000 ms | Tempo que a escrava precisa ver VEZ = 0 depois do boot |
| `CONSULTA_MS` | 100 ms | Intervalo máximo em que a escrava confere o pedido do modo noturno enquanto espera a vez (ela também acorda a cada mudança no quadro do mestre) |
| `TOQUE_BLOQUEIO_MS` | 200 ms | Registro da travessia: toques mais próximos que isso contam como um |
| `TOQUE_SOLTO_MS` | 20 ms | Registro da travessia: intervalo para conferir se o botão foi solto |
| `TOQUE_REPIQUE_MS` | 50 ms | Registro da travessia: espera depois de soltar, para o repique passar |

**Primitivas:** `mutex_carros`; `vez_verde`, `vez_amarelo` e `vez_vermelho`, para a ordem entre as threads; `pedido_travessia` e `toque_travessia`, os dois dados pela interrupção do botão.

**Dados internos:** `mestre` e `uma_placa`, copiados do enlace na inicialização; `vez_consumida`, que impede a escrava de abrir duas vezes com o mesmo VEZ = 1.

**Interrupção do botão de travessia:** na borda de aperto, dá `pedido_travessia`, para o controle, e `toque_travessia`, para o registro. O controle não tem debounce: vários avisos num semáforo que conta até 1 valem um só.

**`t_carro_verde`:**
1. Espera `vez_verde`.
2. Zera `pedido_travessia`, descartando toques antigos e repiques (RF-10, RF-18). Trava o mutex.
3. Acende o verde e registra `carros VERDE`.
4. Espera `VERDE_MIN_MS`. Depois espera `pedido_travessia` por até `VERDE_MS − VERDE_MIN_MS`. Se o pedido chegar, registra `carros travessia pedida: verde encurtado`.
5. Apaga o verde, destrava o mutex e dá `vez_amarelo`.

**`t_carro_amarelo`:**
1. Espera `vez_amarelo` e trava o mutex.
2. **Modo normal:** acende o amarelo, registra `carros AMARELO` e espera `AMARELO_MS`.
3. **Modo noturno ativo:** registra `carros AMARELO PISCANDO`. Enquanto o pedido continuar ligado, inverte o amarelo, registra `carros AMARELO aceso` ou `carros AMARELO apagado` e espera `PISCA_MS`. Quando o pedido desliga, chama `modo_noturno_aplicar(false)`.
4. Apaga o amarelo, destrava o mutex e dá `vez_vermelho`.

**`t_carro_vermelho`:**
1. Espera `vez_vermelho` e trava o mutex.
2. Acende o vermelho, registra `carros VERMELHO` e chama `semaforo_pedestres_liberar()`.
3. Espera a vez de abrir. O que acontece aqui depende do papel:
   - **Placa única:** espera `VERMELHO_MS` e lê o pedido do modo noturno.
   - **Mestre:** a partir do segundo ciclo, dá a vez à escrava (`mestre_dar_a_vez`). Depois lê o pedido do modo noturno, no ponto seguro. Se não houver pedido, espera a liberação (`mestre_esperar_liberacao`).
   - **Escrava:** levanta PARADO. No primeiro ciclo, espera VEZ = 0 estável (`escrava_esperar_vez_zerada`). Depois espera a vez ou o pedido do modo noturno (`escrava_esperar_a_vez`).
4. Chama `semaforo_pedestres_encerrar()`, que só retorna com o vermelho dos pedestres aceso, e apaga o vermelho dos veículos.
5. Se o modo noturno foi pedido, chama `modo_noturno_aplicar(true)` e zera `vez_consumida`.
6. Destrava o mutex e dá `vez_amarelo`, se entrou no modo noturno, ou `vez_verde`.

**`t_registro_travessia`** (prioridade 5, a mesma das luzes, para as linhas da serial não se misturarem):
1. Espera `toque_travessia` e registra `botao travessia: toque`, com o tempo em que acordou.
2. Espera `TOQUE_BLOQUEIO_MS`, ignorando novas bordas.
3. Espera o botão ser solto, consultando a cada `TOQUE_SOLTO_MS`, e mais `TOQUE_REPIQUE_MS`.
4. Descarta as bordas desse intervalo e volta ao passo 1.

Ela não muda o ciclo. Toques a menos de 0,2 s um do outro aparecem como um só na serial, mas o controle continua atendendo a todos.

**Funções do handshake** (internas):

| Função | Papel | Comportamento |
|---|---|---|
| `mestre_dar_a_vez()` | Mestre | Levanta VEZ e espera PARADO = 0 por até `RESPOSTA_MS`. Sem resposta, baixa VEZ, registra `escrava nao pegou a vez: esperando o ciclo dela` e espera `ESPERA_CANCELADA_MS`. Com resposta, espera PARADO = 1, avisando `aguardando a escrava fechar` a cada `AVISO_MS`, e baixa VEZ. |
| `mestre_esperar_liberacao()` | Mestre | Espera `GUARDA_MS` e então espera PARADO = 1, avisando `aguardando a escrava parar` a cada `AVISO_MS`. |
| `escrava_esperar_vez_zerada()` | Escrava | Só no boot. Espera VEZ = 0, avisando `aguardando VEZ = 0 do mestre`, e exige que VEZ fique em 0 por `ESTAVEL_MS`. |
| `escrava_esperar_a_vez()` | Escrava | Retorna `true` para abrir ou `false` se o modo noturno foi pedido. Numa subida de VEZ ainda não usada, baixa PARADO, espera `GUARDA_MS` e confere VEZ de novo: se VEZ caiu, levanta PARADO e continua esperando. Confere o pedido do modo noturno a cada mudança no quadro do mestre, e no máximo a cada `CONSULTA_MS`. Se a espera passa de `AVISO_MS`, registra `aguardando a vez do mestre` e repete a cada `AVISO_MS`. |

**`semaforo_veiculo_iniciar()`:** copia o papel do enlace, configura os LEDs apagados e o botão de travessia com interrupção, e dá `vez_vermelho`. Deve ser a última inicialização, porque começa o ciclo.

### 3.5 `src/mestre.c` e `src/escrava.c`

Iniciam os módulos na ordem `enlace` → `modo_noturno` → `semaforo_pedestres` → `semaforo_veiculo` e registram o papel.

| Arquivo | Enlace | Botão noturno | Mensagem de boot |
|---|---|---|---|
| `src/mestre.c` com `UMA_PLACA=1` | `ENLACE_UNICA` | Sim | `Semaforo iniciado como PLACA UNICA (etapa 1)` |
| `src/mestre.c` | `ENLACE_MESTRE` | Sim | `Semaforo iniciado como MESTRE` |
| `src/escrava.c` | `ENLACE_ESCRAVA` | Não | `Semaforo iniciado como ESCRAVA` |

## 4. Registro na serial

A serial (UART0, 115200 bps, pelo USB da OpenSDA) é a principal evidência dos testes. Todas as linhas de evento têm o formato `tempo ms | evento`, com o tempo da placa desde o reset.

| Evento | Quem registra | Quando |
|---|---|---|
| `carros VERMELHO`, `carros VERDE`, `carros AMARELO` | `semaforo_veiculo` | Ao acender cada luz dos veículos |
| `carros AMARELO PISCANDO` | `semaforo_veiculo` | Ao começar o pisca |
| `carros AMARELO aceso`, `carros AMARELO apagado` | `semaforo_veiculo` | A cada troca do pisca dos veículos |
| `carros travessia pedida: verde encurtado` | `semaforo_veiculo` | Quando um toque encerra o verde |
| `pedestres VERMELHO`, `pedestres VERDE` | `semaforo_pedestres` | Ao acender cada luz dos pedestres |
| `pedestres VERMELHO aceso`, `pedestres VERMELHO apagado` | `semaforo_pedestres` | A cada troca do pisca dos pedestres |
| `botao travessia: toque` | `semaforo_veiculo` | A cada toque no botão de travessia, em qualquer fase e modo |
| `botao noturno: pedido NOTURNO` ou `NORMAL` | `modo_noturno` | A cada toque válido no botão do modo noturno |
| `modo NOTURNO`, `modo NORMAL` | `modo_noturno` | Quando o modo efetivo muda, no ponto seguro |
| `carros escrava nao pegou a vez: esperando o ciclo dela` | Mestre | A cada vez cancelada |
| `carros aguardando a escrava fechar`, `carros aguardando a escrava parar` | Mestre | A cada 10 s de espera |
| `carros aguardando VEZ = 0 do mestre` | Escrava | A cada 10 s de espera, no boot |
| `carros aguardando a vez do mestre` | Escrava | A cada 10 s de espera além do normal |
| `enlace conectado`, `enlace perdido` | `enlace` | Quando começam a chegar quadros válidos da outra placa, e quando eles param por mais de 200 ms |
| `enlace: quadro com o mesmo papel desta placa` | `enlace` | Uma vez, ao receber um quadro com o próprio papel (mesmo programa nas duas placas, ou TX ligado no próprio RX) |
| `Semaforo iniciado como ...`, `Erro N ao iniciar o semaforo` | Arquivo principal | Na inicialização, sem o tempo |

## 5. Desvios conhecidos

Nenhum no momento. A primeira versão deste documento tinha dois desvios do RNF-04, já corrigidos:
1. **O pisca não saía na serial.** Agora cada troca do pisca é registrada, pela `t_carro_amarelo` e pela `t_ped_vermelho` (seções 3.3 e 3.4).
2. **Toques de travessia sem efeito não saíam na serial.** Agora a `t_registro_travessia` registra todos os toques (seção 3.4), sem mudar o critério do RNF-03: a interrupção continua só com `k_sem_give`.
