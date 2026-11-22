# TA-10: ensaio de segurança

Teste descrito no [plano de testes](../../plano-testes.md#ta-10-ensaio-de-segurança). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RS-01, RS-02, RS-04 |
| **Nível** | Sistema. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- As duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados.
- Botão do modo noturno no mestre, botão de travessia na escrava.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-10/ensaio.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-10/ensaio.log --relatorio tests/evidencias/TA-10/ensaio_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `ensaio.log` | 2 minutos: modo normal, toques de travessia na escrava em todas as fases, e pelo menos duas entradas e saídas do modo noturno | `ensaio_analise.md` | _preencher_ |
| `linha_do_tempo.png` | Figura gerada pela análise com `--figura` | — | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RS-01 | Zero instantes com o verde dos pedestres aceso e os veículos da mesma via no verde ou no amarelo | _preencher_ | _preencher_ |
| RS-04 | Zero instantes com as duas vias abertas, pelo tempo do PC | _preencher_ | _preencher_ |
| RS-01, RS-04 | A captura cobriu: modo normal; toques de travessia no verde, no amarelo e no vermelho da escrava; duas entradas e duas saídas do modo noturno | _preencher_ | _preencher_ |
| RS-02 | Conferido pelo vídeo do TA-01 e pela revisão do mutex no TA-12, porque a serial não registra quando uma luz apaga | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-10 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-10 na seção 5 do [plano de testes](../../plano-testes.md).
