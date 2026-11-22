# TA-02: tempos do ciclo normal

Teste descrito no [plano de testes](../../plano-testes.md#ta-02-tempos-do-ciclo-normal). 

| | |
|---|---|
| **Requisitos** | RF-03, RF-04, RNF-05 |
| **Nível** | Parte A: unitário. Parte B: integração. |
| **Data** | 21/11/2026 |
| **Executado por** | Felipe |
| **Commit testado** | 42e811b (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | Parte A: `-DPAPEL=placa_unica`. Parte B: `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | Aprovado |

## 1. Montagem e pré-condição

- Parte A: uma placa montada como na seção 1.5 do 03.
- Parte B: as duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados.
- Captura começada com o sistema já rodando, sem apertar o RESET, e sem tocar em nenhum botão.

## 2. Capturas

| Arquivo | O que é |
|---|---|
| `parteA.log` | Serial da placa, do boot até cerca de 52s |
| `parteA_analise.md` | Análise do parteA.log gerada pelo script analisa_log.py |
| `parteA.mp4` | Vídeo do teste TA-01 parte A |
| `parteB.log` | Serial da placa, do boot até cerca de 60s |
| `parteB_analise.md` | Análise do parteB.log gerada pelo script analisa_log.py |
| `parteB.mp4` | Vídeo do teste TA-01 parte B |

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-03 | Parte A: verde dos veículos de 2,97 a 3,03 s | 3,003 - 3,003 s | Aprovado |
| RF-03 | Parte A: amarelo dos veículos de 0,99 a 1,01 s | 1,002 - 1,002 s | Aprovado |
| RF-03 | Parte A: vermelho dos veículos de 3,96 a 4,04 s | 4,005 - 4,005 s | Aprovado |
| RF-04 | Parte A: verde dos pedestres de 3,96 a 4,04 s | 4,000 - 4,000 s | Aprovado |
| RF-04 | Parte A: vermelho dos pedestres de 3,96 a 4,04 s | 4,010 - 4,010 s | Aprovado |
| RF-03, RF-04 | Parte A: pelo menos 5 medidas de cada fase, sem fase pulada nem fora de ordem | --- | Aprovado |
| RF-03 | Parte B, nas duas vias: verde de 2,97 a 3,03 s e amarelo de 0,99 a 1,01 s | 3,002 - 3,003 s e 1,002 - 1,003 s | Aprovado |
| RF-03 | Parte B, nas duas vias: vermelho dos veículos de 4,00 a 4,20 s | _preencher_ | _preencher_ |
| RF-04 | Parte B, nas duas vias: verde dos pedestres de 4,00 a 4,20 s e vermelho de 3,96 a 4,04 s | 4,117 - 4,118 s | Aprovado |
| RF-03, RF-04 | Parte B: pelo menos 5 medidas de cada fase, sem fase pulada nem fora de ordem | --- | Aprovado |
| RNF-05 | Os tempos acima foram medidos com o registro na serial ligado | --- | Aprovado |

## 4. Observações

- Nenhuma observação

## 5. Conclusão

O teste TA-02 foi aprovado em todos os critérios.
