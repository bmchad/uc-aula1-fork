# TA-05: botão de travessia

Teste descrito no [plano de testes](../../plano-testes.md#ta-05-botão-de-travessia). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-07 a RF-11, RF-21 |
| **Nível** | Sistema. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | Parte A: `-DPAPEL=placa_unica`. Parte B: `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- Parte A: uma placa montada como na seção 1.5 do 03, com o fio do botão de travessia no D10.
- Parte B: as duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados. O botão de travessia fica só na escrava (D10).
- Uma captura por etapa. Toques a menos de 0,2 s um do outro aparecem como um só na serial.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-05/A_rf07.log unica=/dev/ttyACM0
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-05/A_rf07.log --relatorio tests/evidencias/TA-05/A_rf07_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `A_rf07.log` | Parte A, etapa 1: 5 toques entre 1 s e 3 s depois de o verde acender | `A_rf07_analise.md` | _preencher_ |
| `A_rf08.log` | Parte A, etapa 2: 5 toques nos primeiros 0,5 s do verde | `A_rf08_analise.md` | _preencher_ |
| `A_rf10.log` | Parte A, etapa 4: 5 toques no vermelho e 5 no amarelo | `A_rf10_analise.md` | _preencher_ |
| `A_rf11.log` | Parte A, etapa 5: 5 toques rápidos no mesmo verde | `A_rf11_analise.md` | _preencher_ |
| `B_rf07.log` | Parte B, etapa 1, no botão da escrava | `B_rf07_analise.md` | _preencher_ |
| `B_rf08.log` | Parte B, etapa 2, no botão da escrava | `B_rf08_analise.md` | _preencher_ |
| `B_rf10.log` | Parte B, etapa 4, no botão da escrava | `B_rf10_analise.md` | _preencher_ |
| `B_rf11.log` | Parte B, etapa 5, no botão da escrava | `B_rf11_analise.md` | _preencher_ |
| `B_rf21.log` | Parte B, etapa 6: encostar 5 vezes o D10 do mestre no GND, em fases diferentes | `B_rf21_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-07 | Em todos os toques: `travessia pedida`, depois amarelo de 0,99 a 1,01 s e então `pedestres VERDE` (partes A e B) | _preencher_ | _preencher_ |
| RF-08 | Toque no começo do verde: o verde dura de 1,00 a 1,05 s (partes A e B) | _preencher_ | _preencher_ |
| RF-09 | Do `botao travessia: toque` ao `pedestres VERDE`: no máximo 2,05 s; anotar o pior caso | _preencher_ | _preencher_ |
| RF-10 | Toques no vermelho e no amarelo aparecem como `botao travessia: toque`, sem `travessia pedida`, e as durações ficam nas faixas do TA-02 | _preencher_ | _preencher_ |
| RF-11 | Toques rápidos no mesmo verde: uma só `travessia pedida`, um só amarelo, e o verde seguinte de 2,97 a 3,03 s | _preencher_ | _preencher_ |
| RF-21 | Nenhuma linha `botao travessia: toque` na serial do mestre, e as durações do mestre nas faixas do TA-02 | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-05 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-05 na seção 5 do [plano de testes](../../plano-testes.md).
