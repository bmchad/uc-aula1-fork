# TA-01: inspeção das luzes

Teste descrito no [plano de testes](../../plano-testes.md#ta-01-inspeção-das-luzes).

| | |
|---|---|
| **Requisitos** | RF-01, RF-02 |
| **Nível** | Unitário |
| **Data** | 21/11/2026 |
| **Executado por** | Felipe |
| **Commit testado** | aac1702 (`git rev-parse --short HEAD` no momento da gravação) |
| **Firmware** | `west build -b frdm_kl25z -d build/placa_unica -- -DPAPEL=placa_unica`, Zephyr v4.5.0 |
| **Situação** | Aprovado |

## 1. Montagem

Cada placa sozinha, como na seção 1.5 do [03-projeto-componentes.md](../../../docs/03-projeto-componentes.md):

- **Veículos:** LED RGB da placa (vermelho no PTB18, verde no PTB19, amarelo com os dois acesos).
- **Pedestres:** LED vermelho no D6 e LED verde no D7, cada um com resistor de 220 Ω até o GND.
- **USB:** conector SDA, para a gravação e a serial.
- **Botões:** não usados neste teste.

## 2. Procedimento

1. Gravar o firmware de placa única.
2. Ligar a captura: `python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-01/placaN.log placaN=/dev/ttyACM0`.
3. Começar a filmar, apertar o RESET da placa e filmar pelo menos 2 ciclos completos, sem tocar nos botões.
4. Encerrar a captura com Ctrl+C.
5. Analisar se os resultados obtidos batem com os esperados.

**Como achar no vídeo o instante de cada linha do log:** escolha uma troca fácil de ver, como o primeiro verde dos veículos. Anote o tempo dela no vídeo e o tempo da placa no log. A diferença entre os dois vale para todas as outras linhas.

## 3. Resultado da placa 1

**Anexos:** [placa1.log](placa1.log), [placa1.mp4](placa1.mp4).

**Critérios de aprovação:**

| Critério | Resultado |
|---|---|
| As três luzes dos veículos são distinguíveis a olho nu (RF-01) | Aprovado |
| O amarelo (vermelho e verde do RGB juntos) não se confunde com o vermelho nem com o verde (RF-01) | Aprovado |
| As duas luzes dos pedestres são distinguíveis a olho nu (RF-02) | Aprovado |
| Em cada troca, o vídeo mostra a luz que o log registra | Aprovado |

**Observações:** o LED verde está meio fraco, mas dá para ver.

## 4. Resultado da placa 2

**Anexos:** [placa2.log](placa2.log), [placa2.mp4](placa2.mp4).

**Critérios de aprovação:**

| Critério | Resultado |
|---|---|
| As três luzes dos veículos são distinguíveis a olho nu (RF-01) | Aprovado |
| O amarelo (vermelho e verde do RGB juntos) não se confunde com o vermelho nem com o verde (RF-01) | Aprovado |
| As duas luzes dos pedestres são distinguíveis a olho nu (RF-02) | Aprovado |
| Em cada troca, o vídeo mostra a luz que o log registra | Aprovado |

**Observações:** mesma coisa que a placa 1 - o LED verde está meio fraco, mas dá para ver.

## 5. Conclusão

O TA-01 foi aprovado em todos os critérios.

## Anexos

| Arquivo | Conteúdo |
|---|---|
| [placa1.log](placa1.log) | Serial da placa 1, com o tempo do PC |
| [placa1_analise.md](placa1_analise.md) | Saída da `analisa_log.py` para o `placa1.log` |
| placa1.mp4 | _preencher_: vídeo da placa 1, comprimido em 720p |
| placa2.log | _preencher_ |
| placa2_analise.md | _preencher_ |
| placa2.mp4 | _preencher_ |
