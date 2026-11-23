# Projeto dos componentes

Este documento detalha o hardware de cada placa (seção 1), a compilação (seção 2), cada módulo do firmware (seção 3), o registro na serial (seção 4) e os programas de teste (seção 5). A visão geral está no [02-arquitetura.md](02-arquitetura.md).

## 1. Hardware de cada placa

As duas placas são FRDM-KL25Z com a mesma fiação; só os botões mudam: o do modo noturno fica no mestre, e o de travessia, na escrava. Os pinos do código estão no [frdm_kl25z.overlay](../frdm_kl25z.overlay), que o Zephyr aplica sozinho porque tem o nome da placa. Este documento é a referência para a montagem.

### 1.1 Pinos

| Função | Pino | Conector | Nó no devicetree | Ativo em | Ligação |
|---|---|---|---|---|---|
| Veículos: vermelho | PTB18 | (LED RGB) | alias `led2` | baixo | Já ligado na placa |
| Veículos: verde | PTB19 | (LED RGB) | alias `led0` | baixo | Já ligado na placa |
| Veículos: amarelo | PTB18 + PTB19 | — | — | baixo | Os dois acesos juntos |
| Pedestres: vermelho | PTC8 | D6 | `ped_vermelho` | alto | D6 → resistor de 220 Ω → perna longa (anodo) do LED vermelho; perna curta (catodo) → GND |
| Pedestres: verde | PTC9 | D7 | `ped_verde` | alto | D7 → resistor de 220 Ω → perna longa do LED verde; perna curta → GND |
| Botão de travessia | PTD0 | D10 | `botao_travessia` | baixo | **Só na escrava** (e nos testes com o botão: TS-02 e placa única). Um fio no D10; para apertar, encostar a outra ponta no GND |
| Botão do modo noturno | PTD5 | D9 | `botao_noturno` | baixo | **Só no mestre** (e nos testes com o botão: TS-01 e placa única). Um fio no D9, encostado no GND |
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
- **Teste da fiação de cada placa, antes de ligar a outra:** ligue o D12 no D11 da própria placa, pelo resistor. Com o firmware de mestre ou de escrava, a serial deve mostrar `enlace: quadro com o mesmo papel desta placa`. Isso confirma o TX, o RX e os pinos certos. Como esse quadro não conta como a outra placa, a placa também registra `enlace FALHA` e fica no ALERTA, o que é esperado.
- O formato do quadro e o protocolo estão na seção 2 do [02-arquitetura.md](02-arquitetura.md).

### 1.5 Montagem dos testes com uma placa

Os testes TU-01, TU-02, TI-01, TS-01 e TS-02, e o papel de placa única, usam uma placa só, sem os fios da seção 1.4.

| O que | Ligação | Usado em |
|---|---|---|
| USB | Conector **SDA** da placa (gravação e serial) | Todos |
| LEDs dos pedestres | D6 e D7, como na seção 1.1 | TU-01, TI-01, TS-01, TS-02 |
| Botão do modo noturno | Fio no D9 | TS-01 |
| Botão de travessia | Fio no D10 | TS-02 |

- **GND:** o pino ao lado do D13, no conector do D8 ao D13, ou qualquer GND do conector de alimentação.
- **Monte com o USB desligado.**

## 2. Configuração do build

| Arquivo | Conteúdo |
|---|---|
| [CMakeLists.txt](../CMakeLists.txt) | Firmware principal. Escolhe o papel pela variável `PAPEL` (`mestre`, `escrava` ou `placa_unica`; padrão `mestre`), compila as quatro bibliotecas de `lib/` e, na placa única, define `UMA_PLACA=1`. Um valor inválido para a compilação com erro. |
| [prj.conf](../prj.conf) | Liga o GPIO, o console na UART0 (`CONFIG_GPIO`, `CONFIG_SERIAL`, `CONFIG_CONSOLE`, `CONFIG_UART_CONSOLE`) e a UART por interrupção, para o enlace (`CONFIG_UART_INTERRUPT_DRIVEN`). |
| [frdm_kl25z.overlay](../frdm_kl25z.overlay) | Pinos da seção 1.1, habilitação da porta C, nó da UART2 com os pinos PTD2 e PTD3, e o alias `enlace-uart`. |
| `tests/<teste>/CMakeLists.txt` | Programa de cada teste (seção 5). Usa o overlay da raiz, para os pinos serem os mesmos do firmware principal, e compila só os módulos daquele teste. Os testes de duas placas (TU-03 e TI-02) também têm a variável `PAPEL` (`mestre` ou `escrava`). |

Compilar e gravar o firmware principal, na raiz do projeto, com uma pasta de build por papel. O `-d` escolhe a pasta de build, o `-DPAPEL` escolhe o papel, e o `-r pyocd` grava com o pyOCD. A gravação apaga o firmware que estava na placa; deixe só a placa a ser gravada no USB.

```bash
west build -b frdm_kl25z -d build/mestre -- -DPAPEL=mestre
```

```bash
west flash -d build/mestre -r pyocd
```

A escrava é igual, com `escrava` no lugar de `mestre`. Um programa de teste é compilado pela pasta dele, por exemplo:

```bash
west build -b frdm_kl25z -d build/TU-01 tests/TU-01
```

## 3. Componentes de software

Convenções comuns aos quatro módulos:
- As threads são criadas com `K_THREAD_DEFINE`, com pilha de 768 bytes, e começam bloqueadas num `k_sem`. Nenhuma faz nada antes da função `_iniciar` do seu módulo.
- As funções `_iniciar` retornam 0 ou um código de erro negativo do Zephyr (`-ENODEV` se um dispositivo não está pronto). O arquivo principal para no primeiro erro e registra `Erro N ao iniciar o semaforo`.
- As interrupções não travam mutex nem esperam: as dos botões só dão `k_sem_give`, e a da UART2 só copia o byte para uma fila sem espera.

### 3.1 `lib/enlace`

**Responsabilidade:** trocar o estado com a outra placa pela UART2 e dizer se o enlace está em falha. Não sabe nada de semáforo, só de sinais lógicos: SINAL (VEZ ou PARADO, conforme o papel), NOTURNO, o papel de quem mandou e OUVE.

**Constantes:**

| Constante | Valor | Uso |
|---|---|---|
| `BYTE_INICIO` | `0xA5` | Primeiro byte de todo quadro |
| `PERIODO_MS` | 50 ms | Intervalo de reenvio do estado, mesmo sem mudança |
| `PRAZO_MS` | 200 ms | Sem quadro válido por esse tempo: enlace perdido |
| `INICIO_MS` | 500 ms | Tolerância depois do boot, antes de declarar falha |
| `TAMANHO_FILA` | 64 bytes | Bytes recebidos à espera da `t_enlace_rx` |
| `PRIORIDADE_ENLACE` | 5 | A mesma das luzes, para as linhas da serial não se misturarem |

**Quadro** (4 bytes, 115200 bps, 8 bits, sem paridade, 1 bit de parada):

| Byte | Conteúdo |
|---|---|
| 0 | `0xA5` |
| 1 | Bit 0: SINAL (VEZ no mestre, PARADO na escrava). Bit 1: NOTURNO (só o mestre). Bit 2: MESTRE (1 se quem mandou é o mestre). Bit 3: OUVE (1 se quem mandou está recebendo quadros válidos da outra placa). Bits 4 a 7: sempre 0. |
| 2 | Contador, que aumenta a cada quadro (só para depuração) |
| 3 | `~(byte0 + byte1 + byte2)`, em 8 bits |

**Dados internos:**

| Nome | Tipo | Uso |
|---|---|---|
| `mestre`, `sozinha` | `bool` | Papel da placa. A placa única age como mestre e não usa a UART. |
| `local` | `atomic_t` | Bits que esta placa manda (o OUVE é acrescentado na hora de mandar) |
| `remoto` | `atomic_t` | Bits do último quadro válido da outra placa; 0 sem enlace |
| `conectado` | `atomic_t` | 1 enquanto chegam quadros válidos da outra placa |
| `falha` | `atomic_t` | 1 com o enlace em falha (modo ALERTA) |
| `fila_rx` | `k_msgq` de 64 bytes | Interrupção da UART2 → `t_enlace_rx` |
| `sem_mudanca` | `k_sem`, limite 1 | `t_enlace_rx` → quem espera a outra placa |
| `sem_envio` | `k_sem`, limite 1 | Mudança local → `t_enlace_tx` manda na hora |

**Interrupção da UART2:** lê cada byte recebido e o coloca na `fila_rx` com `k_msgq_put` sem espera. Se a fila estiver cheia, o byte se perde, e o quadro dele falha na verificação.

**`t_enlace_rx`**, a cada byte recebido ou a cada `PERIODO_MS`:
1. Junta o byte aos três anteriores.
2. Se os quatro formam um quadro válido (começa com `0xA5`, só usa os bits 0 a 3, verificação certa):
   - com o papel oposto ao desta placa: guarda SINAL, NOTURNO e OUVE em `remoto` e anota a hora. Se não estava conectado, registra `enlace conectado`;
   - com o mesmo papel: descarta o quadro e registra, uma vez só, `enlace: quadro com o mesmo papel desta placa`.
3. Se estava conectado e o último quadro válido tem mais de `PRAZO_MS`, zera `remoto` e registra `enlace perdido`.
4. Calcula a falha: o enlace está OK se esta placa está conectada **e** o OUVE da outra é 1. Nos primeiros `INICIO_MS` depois do boot, a falha não é declarada. Quando a falha muda, registra `enlace FALHA` ou `enlace OK`.
5. Se algo mudou nos passos 2 a 4, dá `sem_mudanca`.

**`t_enlace_tx`:** manda o quadro com os bits de `local`, mais o OUVE se estiver conectado, e espera `sem_envio` por até `PERIODO_MS`. Assim manda logo que o estado muda e repete a cada 50 ms.

**Funções:**

| Função | Comportamento |
|---|---|
| `enlace_iniciar(papel)` | Na placa única, só guarda o papel. Nos outros papéis, começa no estado seguro (mestre com VEZ = 0, escrava com PARADO = 1, NOTURNO = 0), instala a interrupção da UART2 e começa as duas threads. |
| `enlace_sou_mestre()`, `enlace_uma_placa()` | Devolvem o papel. |
| `enlace_publicar(valor)` | Muda o SINAL desta placa (VEZ no mestre, PARADO na escrava) e pede o envio imediato. Não faz nada na placa única. |
| `enlace_ler()` | SINAL da outra placa: PARADO no mestre, VEZ na escrava. Vale 0 sem enlace. |
| `enlace_esperar(nivel, prazo)` | Espera o SINAL da outra placa chegar ao nível pedido. Depois de cada aviso, **lê o nível de novo**, então um aviso atrasado ou repetido não causa decisão errada. Retorna 0, ou `-EAGAIN` se o prazo acabou. |
| `enlace_aguardar_mudanca(prazo)` | Zera os avisos antigos e espera qualquer mudança na outra placa a partir de agora. Retorna `true` se houve mudança. |
| `enlace_publicar_noturno(ligado)`, `enlace_ler_noturno()` | Bit NOTURNO: o mestre manda, a escrava lê. Vale 0 sem enlace. |
| `enlace_falhou()` | `true` com o enlace em falha. Sempre `false` na placa única. |

### 3.2 `lib/modo_noturno`

**Responsabilidade:** guardar os dois modos que fazem as luzes piscarem, NOTURNO e ALERTA, separando o **pedido** do **modo efetivo**, e tratar o botão do modo noturno.

| Modo | Pedido | Quem aplica o modo efetivo |
|---|---|---|
| NOTURNO | No mestre, o botão (cada toque alterna). Na escrava, o bit NOTURNO do enlace. | O semáforo de veículos: liga no ponto seguro, desliga quando o pedido acaba |
| ALERTA | `enlace_falhou()` | O semáforo de veículos: liga em qualquer fase, desliga quando o enlace volta |

**Constantes:**

| Constante | Valor | Uso |
|---|---|---|
| `DEBOUNCE_MS` | 50 ms | Espera depois da borda, antes de conferir o botão, e depois de soltar |
| `LEITURA_SOLTO_MS` | 20 ms | Intervalo da consulta enquanto o botão continua apertado |
| `PRIORIDADE_BOTAO` | 4 | Acima das threads das luzes (5) |

**Dados internos:** `pedido_botao`, `ativo` (noturno efetivo), `alerta` (ALERTA efetivo) e `registrado` (último modo escrito na serial), protegidos por `mutex_modo`; `com_botao`, que é `true` só no mestre; `sem_toque`, dado pela interrupção do botão.

**`thread_botao`** (só age na placa com o botão):
1. Espera `sem_toque`, dado pela interrupção na borda de aperto.
2. Espera `DEBOUNCE_MS` e confere se o botão continua apertado. Se não continuar, foi repique, e o toque é descartado.
3. Inverte `pedido_botao`, com o mutex travado, e registra `botao noturno: pedido NOTURNO` ou `pedido NORMAL`.
4. Espera o botão ser solto, consultando a cada `LEITURA_SOLTO_MS`, para um toque longo não alternar de novo. Depois espera mais `DEBOUNCE_MS`.
5. Descarta os avisos que a interrupção deu durante o toque e volta ao passo 1.

**Registro do modo:** toda mudança de modo efetivo escreve o modo em vigor na serial (`modo ALERTA`, `modo NOTURNO` ou `modo NORMAL`), só quando ele muda. O ALERTA tem prioridade: desligar o noturno durante o ALERTA não escreve `modo NORMAL`.

**Funções:**

| Função | Comportamento |
|---|---|
| `modo_noturno_iniciar(usar_botao)` | No mestre (`true`), configura o botão e a interrupção. Na escrava (`false`), não configura nada, e a thread do botão fica bloqueada para sempre. |
| `modo_noturno_pedido()` | No mestre, devolve `pedido_botao`. Na escrava, lê o bit NOTURNO do enlace. |
| `modo_noturno_aplicar(ligado)` | Muda o noturno efetivo. No mestre, também manda o bit NOTURNO pelo enlace. |
| `modo_noturno_ativo()` | Devolve o noturno efetivo. |
| `modo_alerta_pedido()` | Devolve `enlace_falhou()`. |
| `modo_alerta_aplicar(ligado)` | Muda o ALERTA efetivo. |
| `modo_alerta_ativo()` | Devolve o ALERTA efetivo. |
| `modo_pisca()` | `true` com o NOTURNO ou o ALERTA em vigor. As threads das luzes usam para decidir se piscam. |

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
3. Espera `liberado` com prazo de `PISCA_MS` (1 s). A cada prazo vencido, se `modo_pisca()` for `true`, inverte o vermelho; senão, o mantém aceso. Cada troca é registrada como `pedestres VERMELHO aceso` ou `pedestres VERMELHO apagado`.
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

Cada `liberar` precisa ser seguido de um `encerrar` antes de os veículos abrirem. Esse contrato é o que garante que os pedestres nunca atravessam com os carros abertos (seção 1.3 do [02-arquitetura.md](02-arquitetura.md)).

### 3.4 `lib/semaforo_veiculo`

**Responsabilidade:** as três luzes dos veículos, o botão de travessia, o ponto seguro do modo noturno, o handshake com a outra placa e a entrada e a saída do ALERTA.

**Constantes:**

| Constante | Valor | Uso |
|---|---|---|
| `VERDE_MS` | 3000 ms | Duração do verde |
| `VERDE_MIN_MS` | 1000 ms | Verde mínimo, mesmo com travessia |
| `AMARELO_MS` | 1000 ms | Duração do amarelo |
| `VERMELHO_MS` | 4000 ms | Vermelho na placa única |
| `PISCA_MS` | 1000 ms | Meio período do pisca |
| `GUARDA_MS` | 50 ms | Folga para a outra placa reagir a um sinal |
| `RESPOSTA_MS` | 3000 ms | Prazo do mestre para a escrava pegar a vez |
| `ESPERA_CANCELADA_MS` | 5000 ms | Espera do mestre depois de cancelar a vez: verde + amarelo + 1 s de margem |
| `AVISO_MS` | 10000 ms | Intervalo dos avisos de espera longa na serial |
| `ESTAVEL_MS` | 1000 ms | Tempo que a escrava precisa ver VEZ = 0 no boot e depois de um ALERTA |
| `CONSULTA_MS` | 50 ms | Intervalo máximo para conferir o enlace e o pedido do modo noturno durante as esperas e o verde |
| `TOQUE_BLOQUEIO_MS` | 200 ms | Registro da travessia: toques mais próximos que isso contam como um |
| `TOQUE_SOLTO_MS` | 20 ms | Registro da travessia: intervalo para conferir se o botão foi solto |
| `TOQUE_REPIQUE_MS` | 50 ms | Registro da travessia: espera depois de soltar, para o repique passar |

**Primitivas:** `mutex_carros`; `vez_verde`, `vez_amarelo` e `vez_vermelho`, para a ordem entre as threads; `pedido_travessia` e `toque_travessia`, os dois dados pela interrupção do botão.

**Dados internos:**
- `mestre` e `uma_placa`, copiados do enlace na inicialização;
- `vez_consumida`, que impede a escrava de abrir duas vezes com o mesmo VEZ = 1;
- `recomecar`, `true` no boot e depois de cada ALERTA: no próximo vermelho, o mestre abre primeiro e a escrava espera VEZ = 0 estável.

Só as threads dos carros mexem nesses dados, uma de cada vez, porque cada uma trava o `mutex_carros`.

**Interrupção do botão de travessia:** na borda de aperto, dá `pedido_travessia`, para o controle, e `toque_travessia`, para o registro. O controle não tem debounce: vários avisos num semáforo que conta até 1 valem um só.

**`t_carro_verde`:**
1. Espera `vez_verde`.
2. Zera `pedido_travessia`, descartando toques antigos e repiques. Trava o mutex.
3. Acende o verde e registra `carros VERDE`.
4. Espera o fim do verde (`esperar_verde`), acordando no máximo a cada `CONSULTA_MS`:
   - com o enlace em falha, termina na hora e registra `carros enlace em falha: verde encerrado`;
   - depois de `VERDE_MIN_MS`, um `pedido_travessia` termina o verde, e a thread registra `carros travessia pedida: verde encurtado`;
   - sem nenhum dos dois, termina em `VERDE_MS`.
5. Apaga o verde, destrava o mutex e dá `vez_amarelo`.

**`t_carro_amarelo`:**
1. Espera `vez_amarelo` e trava o mutex.
2. **Vindo do verde, sem falha:** acende o amarelo, registra `carros AMARELO` e espera `AMARELO_MS`.
3. **Com um modo de pisca em vigor (noturno vindo do vermelho) ou com o enlace em falha:** registra `carros AMARELO PISCANDO` e pisca (`piscar`). A cada meio período:
   - se o enlace falhou e o ALERTA ainda não está em vigor, entra no ALERTA (`entra_alerta`); se o enlace voltou, desliga o ALERTA;
   - se o noturno está em vigor e o pedido acabou, desliga o noturno;
   - se nenhum modo de pisca continua, para de piscar;
   - senão, inverte o amarelo, registra `carros AMARELO aceso` ou `carros AMARELO apagado` e espera `PISCA_MS`.
4. Apaga o amarelo, destrava o mutex e dá `vez_vermelho`.

**`t_carro_vermelho`:**
1. Espera `vez_vermelho` e trava o mutex.
2. Acende o vermelho, registra `carros VERMELHO` e chama `semaforo_pedestres_liberar()`.
3. Espera a decisão: abrir, entrar no noturno ou entrar no ALERTA. Ela depende do papel:
   - **Placa única:** espera `VERMELHO_MS` e lê o pedido do modo noturno.
   - **Mestre:** fora do recomeço, dá a vez à escrava (`mestre_dar_a_vez`). Depois lê o pedido do modo noturno, no ponto seguro. Se não houver pedido, espera a liberação (`mestre_esperar_liberacao`) e abre. Se o enlace falhar em qualquer espera, a decisão é o ALERTA.
   - **Escrava:** levanta PARADO. No recomeço, espera VEZ = 0 estável (`escrava_esperar_vez_zerada`). Depois espera a vez, o pedido do modo noturno ou a falha do enlace (`escrava_esperar_a_vez`).
4. Zera `recomecar`. Chama `semaforo_pedestres_encerrar()`, que só retorna com o vermelho dos pedestres aceso, e apaga o vermelho dos veículos.
5. Conforme a decisão:
   - **noturno:** chama `modo_noturno_aplicar(true)` e zera `vez_consumida`;
   - **ALERTA:** registra `carros enlace em falha: vermelho encerrado` e chama `entra_alerta`.
6. Destrava o mutex e dá `vez_verde` (abrir) ou `vez_amarelo` (noturno ou ALERTA).

**`t_registro_travessia`** (prioridade 5, a mesma das luzes, para as linhas da serial não se misturarem):
1. Espera `toque_travessia` e registra `botao travessia: toque`, com o tempo em que acordou.
2. Espera `TOQUE_BLOQUEIO_MS`, ignorando novas bordas.
3. Espera o botão ser solto, consultando a cada `TOQUE_SOLTO_MS`, e mais `TOQUE_REPIQUE_MS`.
4. Descarta as bordas desse intervalo e volta ao passo 1.

Ela não muda o ciclo. Toques a menos de 0,2 s um do outro aparecem como um só na serial, mas o controle continua atendendo a todos.

**Funções internas:**

| Função | Papel | Comportamento |
|---|---|---|
| `entra_alerta()` | Os dois | Publica o sinal seguro (`enlace_publicar(false)`: VEZ = 0 no mestre, PARADO = 0 na escrava), zera `vez_consumida`, marca `recomecar` e liga o ALERTA. |
| `esperar_tempo(ms)` | Os dois | Espera `ms`, conferindo o enlace a cada `CONSULTA_MS`. Retorna 0, ou `-ECANCELED` se o enlace falhou. |
| `esperar_sinal(nivel, prazo, aviso)` | Os dois | Espera o SINAL da outra placa chegar a `nivel`, por até `prazo` (ou sem prazo), registrando `aviso` a cada `AVISO_MS`. Retorna 0, `-EAGAIN` se o prazo acabou, ou `-ECANCELED` se o enlace falhou. |
| `mestre_dar_a_vez()` | Mestre | Espera PARADO = 1 (`aguardando a escrava parar`), levanta VEZ e espera PARADO = 0 por até `RESPOSTA_MS`. Sem resposta, baixa VEZ, registra `escrava nao pegou a vez: esperando o ciclo dela` e espera `ESPERA_CANCELADA_MS`. Com resposta, espera PARADO = 1 (`aguardando a escrava fechar`) e baixa VEZ. Retorna `-ECANCELED` se o enlace falhou, sempre com VEZ = 0. |
| `mestre_esperar_liberacao()` | Mestre | Espera `GUARDA_MS` e então PARADO = 1 (`aguardando a escrava parar`). |
| `escrava_esperar_vez_zerada()` | Escrava | No boot e depois de um ALERTA. Espera VEZ = 0 (`aguardando VEZ = 0 do mestre`) e exige que VEZ fique em 0 por `ESTAVEL_MS`. |
| `escrava_esperar_a_vez()` | Escrava | Retorna abrir, noturno ou ALERTA. Numa subida de VEZ ainda não usada, baixa PARADO, espera `GUARDA_MS` e confere VEZ de novo: se VEZ caiu, levanta PARADO e continua esperando. Confere o pedido do modo noturno e o enlace a cada mudança no quadro do mestre, e no máximo a cada `CONSULTA_MS`. Se a espera passa de `AVISO_MS`, registra `aguardando a vez do mestre` e repete a cada `AVISO_MS`. |

As mensagens de espera longa só aparecem com o enlace OK e a outra placa parada num estado inesperado: com o enlace em falha, a placa entra no ALERTA em vez de esperar.

**`semaforo_veiculo_iniciar(usar_botao_travessia)`:** copia o papel do enlace, configura os LEDs apagados e, se `usar_botao_travessia` for `true`, o botão de travessia com interrupção; depois dá `vez_vermelho`. A escrava e a placa única passam `true`; o mestre passa `false` e não usa o D10. Sem o botão, a `t_carro_verde` só termina pelo tempo ou pela falha do enlace, e a `t_registro_travessia` fica bloqueada. Deve ser a última inicialização, porque começa o ciclo.

### 3.5 `src/mestre.c` e `src/escrava.c`

Iniciam os módulos na ordem `enlace` → `modo_noturno` → `semaforo_pedestres` → `semaforo_veiculo` e registram o papel.

| Arquivo | Enlace | Botão noturno | Botão de travessia | Mensagem de boot |
|---|---|---|---|---|
| `src/mestre.c` | `ENLACE_MESTRE` | Sim | Não | `Semaforo iniciado como MESTRE` |
| `src/escrava.c` | `ENLACE_ESCRAVA` | Não | Sim | `Semaforo iniciado como ESCRAVA` |
| `src/mestre.c` com `UMA_PLACA=1` | `ENLACE_UNICA` | Sim | Sim | `Semaforo iniciado como PLACA UNICA` |

## 4. Registro na serial

A serial (UART0, 115200 bps, pelo USB da OpenSDA) é a principal evidência dos testes, junto com os vídeos. Todas as linhas de evento têm o formato `tempo ms | evento`, com o tempo da placa desde o reset. Com o [`captura_serial.py`](../tests/ferramentas/captura_serial.py), cada linha ganha também o tempo do PC e o nome da placa.

| Evento | Quem registra | Quando |
|---|---|---|
| `carros VERMELHO`, `carros VERDE`, `carros AMARELO` | `semaforo_veiculo` | Ao acender cada luz dos veículos |
| `carros AMARELO PISCANDO` | `semaforo_veiculo` | Ao começar o pisca (noturno ou ALERTA) |
| `carros AMARELO aceso`, `carros AMARELO apagado` | `semaforo_veiculo` | A cada troca do pisca dos veículos |
| `carros travessia pedida: verde encurtado` | `semaforo_veiculo` | Quando um toque encerra o verde |
| `carros enlace em falha: verde encerrado`, `carros enlace em falha: vermelho encerrado` | `semaforo_veiculo` | Quando a falha do enlace interrompe o verde ou o vermelho |
| `pedestres VERMELHO`, `pedestres VERDE` | `semaforo_pedestres` | Ao acender cada luz dos pedestres |
| `pedestres VERMELHO aceso`, `pedestres VERMELHO apagado` | `semaforo_pedestres` | A cada troca do pisca dos pedestres |
| `botao travessia: toque` | `semaforo_veiculo` | A cada toque no botão de travessia, em qualquer fase e modo |
| `botao noturno: pedido NOTURNO` ou `NORMAL` | `modo_noturno` | A cada toque válido no botão do modo noturno |
| `modo NOTURNO`, `modo ALERTA`, `modo NORMAL` | `modo_noturno` | Quando o modo em vigor muda |
| `carros escrava nao pegou a vez: esperando o ciclo dela` | Mestre | A cada vez cancelada |
| `carros aguardando a escrava parar`, `carros aguardando a escrava fechar` | Mestre | A cada 10 s de espera, com o enlace OK |
| `carros aguardando VEZ = 0 do mestre`, `carros aguardando a vez do mestre` | Escrava | A cada 10 s de espera, com o enlace OK |
| `enlace conectado`, `enlace perdido` | `enlace` | Quando começam a chegar quadros válidos da outra placa, e quando eles param por mais de 200 ms |
| `enlace FALHA`, `enlace OK` | `enlace` | Quando a falha do enlace começa e termina (nos dois sentidos, pelo bit OUVE) |
| `enlace: quadro com o mesmo papel desta placa` | `enlace` | Uma vez, ao receber um quadro com o próprio papel (mesmo programa nas duas placas, ou TX ligado no próprio RX) |
| `Semaforo iniciado como ...`, `Erro N ao iniciar o semaforo` | Arquivo principal | Na inicialização, sem o tempo |

## 5. Programas de teste

Os testes unitários, de integração e de sistema rodam um programa próprio, na pasta de cada teste em [tests/](../tests/), e não o firmware principal. Cada programa compila só os módulos sob teste e troca os outros por **dublês**: versões simples das mesmas funções, com o comportamento mínimo (por exemplo, o enlace de placa única e o modo noturno sempre desligado). Os testes de aceitação (TA-01 a TA-04) usam o firmware principal.

| Teste | Módulos de verdade | Dublês | O que o programa acrescenta |
|---|---|---|---|
| TU-01 | `semaforo_pedestres` | `modo_pisca` (nunca pisca) | Faz o papel dos veículos: libera os pedestres depois de 4 s e os encerra depois de mais 4 s |
| TU-02 | `semaforo_veiculo` | `enlace` (placa única), `modo_noturno` (nunca noturno nem ALERTA), `semaforo_pedestres` (só registra as chamadas) | Só inicia o semáforo; o ciclo é o do módulo |
| TU-03 | `enlace` | — | Inverte o SINAL a cada 1 s e registra cada troca recebida da outra placa, com o intervalo; resumo a cada 10 s. Compilado como mestre ou escrava (`PAPEL`) |
| TI-01 | `semaforo_veiculo`, `semaforo_pedestres` | `enlace` (placa única), `modo_noturno` | Monitor: violações de sincronia e janelas de troca; resumo a cada 10 s |
| TI-02 | `enlace`, `semaforo_veiculo`, `semaforo_pedestres` | `modo_noturno` | O monitor do TI-01, mais os carros abertos sem a permissão da outra placa e as aberturas; resumo a cada 10 s. Compilado como mestre ou escrava (`PAPEL`) |
| TS-01 | `modo_noturno` (botão no D9), `semaforo_veiculo`, `semaforo_pedestres` | `enlace` (placa única, sem falha) | Monitor: atraso de entrada e de saída do noturno, meios períodos do pisca e luzes proibidas no pisca; resumo a cada 10 s |
| TS-02 | `semaforo_veiculo` (botão no D10), `semaforo_pedestres` | `enlace` (placa única), `modo_noturno` | Monitor: uma linha por fase dos carros, com a duração, os toques e o veredito; resumo a cada 10 s |

- **Monitor:** uma thread de prioridade 4, acima das luzes, que lê os LEDs (e, conforme o teste, o botão, o modo ou o enlace) e espera um tick (0,1 ms) entre uma leitura e a seguinte. Com o tempo da própria leitura, sai uma amostra a cada 0,2 ms, cerca de 50 000 a cada 10 s. Ele lê o nível dos pinos, ou seja, o que o firmware comanda, e não a luz em si: o vídeo continua sendo a evidência do que os LEDs mostram.
- **Arquivos de cada pasta:** `CMakeLists.txt`, `prj.conf`, `src/main.c` e, quando há dublês em arquivo separado, `src/dubles.c`. O procedimento e o resultado de cada teste estão no `README.md` da pasta e no [plano de testes](../tests/plano-testes.md).
