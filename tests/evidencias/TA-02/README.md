# TA-02: tempos do ciclo normal

Teste descrito no [plano de testes](../../plano-testes.md#ta-02-tempos-do-ciclo-normal). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-03, RF-04, RNF-05 |
| **Nível** | Parte A: unitário. Parte B: integração. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | Parte A: `-DPAPEL=placa_unica`. Parte B: `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- Parte A: uma placa montada como na seção 1.5 do 03.
- Parte B: as duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados.
- Captura começada com o sistema já rodando, sem apertar o RESET, e sem tocar em nenhum botão.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-02/parteA.log unica=/dev/ttyACM0
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-02/parteA.log --relatorio tests/evidencias/TA-02/parteA_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `parteA.log` | Parte A: pelo menos 6 ciclos, cerca de 50 s | `parteA_analise.md` | _preencher_ |
| `parteB.log` | Parte B: pelo menos 6 ciclos das duas vias, cerca de 50 s | `parteB_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-03 | Parte A: verde dos veículos de 2,97 a 3,03 s | _preencher_ | _preencher_ |
| RF-03 | Parte A: amarelo dos veículos de 0,99 a 1,01 s | _preencher_ | _preencher_ |
| RF-03 | Parte A: vermelho dos veículos de 3,96 a 4,04 s | _preencher_ | _preencher_ |
| RF-04 | Parte A: verde dos pedestres de 3,96 a 4,04 s | _preencher_ | _preencher_ |
| RF-04 | Parte A: vermelho dos pedestres de 3,96 a 4,04 s | _preencher_ | _preencher_ |
| RF-03, RF-04 | Parte A: pelo menos 5 medidas de cada fase, sem fase pulada nem fora de ordem | _preencher_ | _preencher_ |
| RF-03 | Parte B, nas duas vias: verde de 2,97 a 3,03 s e amarelo de 0,99 a 1,01 s | _preencher_ | _preencher_ |
| RF-03 | Parte B, nas duas vias: vermelho dos veículos de 4,00 a 4,20 s | _preencher_ | _preencher_ |
| RF-04 | Parte B, nas duas vias: verde dos pedestres de 4,00 a 4,20 s e vermelho de 3,96 a 4,04 s | _preencher_ | _preencher_ |
| RF-03, RF-04 | Parte B: pelo menos 5 medidas de cada fase, sem fase pulada nem fora de ordem | _preencher_ | _preencher_ |
| RNF-05 | Os tempos acima foram medidos com o registro na serial ligado | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-02 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-02 na seção 5 do [plano de testes](../../plano-testes.md).
