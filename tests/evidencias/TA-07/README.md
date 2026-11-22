# TA-07: pisca do modo noturno

Teste descrito no [plano de testes](../../plano-testes.md#ta-07-pisca-do-modo-noturno). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-13, RF-14, RF-15, RF-18 |
| **Nível** | Sistema. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | Parte A: `-DPAPEL=placa_unica`. Parte B: `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- Parte A: uma placa montada como na seção 1.5 do 03, com os fios dos dois botões (D9 e D10).
- Parte B: as duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados. Botão do modo noturno no mestre, botão de travessia na escrava.
- Uma captura por parte, de cerca de 35 s: pedir o modo, pelo menos 10 s de pisca com 5 toques de travessia, sair e esperar o primeiro verde terminar.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-07/parteA.log unica=/dev/ttyACM0
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-07/parteA.log --relatorio tests/evidencias/TA-07/parteA_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `parteA.log` | Parte A | `parteA_analise.md` | _preencher_ |
| `parteB.log` | Parte B | `parteB_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-13 | Meio período do pisca do amarelo dos veículos de 0,99 a 1,01 s (partes A e B, nas duas vias) | _preencher_ | _preencher_ |
| RF-14 | Meio período do pisca do vermelho dos pedestres de 0,99 a 1,01 s | _preencher_ | _preencher_ |
| RF-13, RF-14 | Durante o pisca, nenhuma linha `carros VERDE`, `carros VERMELHO` ou `pedestres VERDE` | _preencher_ | _preencher_ |
| RF-15 | Em cada via, as trocas do pisca dos veículos e dos pedestres ficam a até 50 ms uma da outra | _preencher_ | _preencher_ |
| RF-18 | Os 5 toques aparecem como `botao travessia: toque`, sem `travessia pedida` | _preencher_ | _preencher_ |
| RF-18 | O primeiro verde depois da saída dura de 2,97 a 3,03 s | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-07 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-07 na seção 5 do [plano de testes](../../plano-testes.md).
