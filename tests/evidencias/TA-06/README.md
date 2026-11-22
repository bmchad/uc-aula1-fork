# TA-06: botão do modo noturno

Teste descrito no [plano de testes](../../plano-testes.md#ta-06-botão-do-modo-noturno). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-12 |
| **Nível** | Unitário. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | `-DPAPEL=placa_unica`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- Uma placa montada como na seção 1.5 do 03, com o fio do botão do modo noturno no D9.
- Vídeo do fio no D9, com áudio: os toques são contados em voz alta.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-06/etapa1.log unica=/dev/ttyACM0
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-06/etapa1.log --relatorio tests/evidencias/TA-06/etapa1_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `etapa1.log` | Etapa 1: 10 toques curtos, separados por pelo menos 0,5 s | `etapa1_analise.md` | _preencher_ |
| `etapa1.mp4` | Vídeo da etapa 1, com a contagem em voz alta | — | _preencher_ |
| `etapa2.log` | Etapa 2: 5 toques longos, de 2 s cada | `etapa2_analise.md` | _preencher_ |
| `etapa2.mp4` | Vídeo da etapa 2 | — | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-12 | Etapa 1: exatamente 10 linhas `botao noturno: pedido ...`, alternando entre `NOTURNO` e `NORMAL` | _preencher_ | _preencher_ |
| RF-12 | Etapa 2: exatamente 5 linhas, alternando | _preencher_ | _preencher_ |
| RF-12 | O número de toques no vídeo é igual ao número de linhas no log | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-06 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-06 na seção 5 do [plano de testes](../../plano-testes.md).
