# TA-08: entrada e saída do modo noturno

Teste descrito no [plano de testes](../../plano-testes.md#ta-08-entrada-e-saída-do-modo-noturno). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-16, RF-17 |
| **Nível** | Integração. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | Parte A: `-DPAPEL=placa_unica`. Parte B: `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- Parte A: uma placa montada como na seção 1.5 do 03, com o fio do botão do modo noturno no D9.
- Parte B: as duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados. Botão do modo noturno no mestre.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-08/A_entrada.log unica=/dev/ttyACM0
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-08/A_entrada.log --relatorio tests/evidencias/TA-08/A_entrada_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `A_entrada.log` | Parte A, etapa 1: pedido no verde, no amarelo e no vermelho, saindo entre um e outro | `A_entrada_analise.md` | _preencher_ |
| `A_saida.log` | Parte A, etapa 2: 5 saídas, cada uma depois de pelo menos 4 s de pisca | `A_saida_analise.md` | _preencher_ |
| `B_entrada.log` | Parte B, etapa 1 | `B_entrada_analise.md` | _preencher_ |
| `B_saida.log` | Parte B, etapa 2 | `B_saida_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-16 | Entrada: `modo NOTURNO` só no fim de um vermelho dos veículos, em até 8,4 s do pedido, nas 3 tentativas (partes A e B) | _preencher_ | _preencher_ |
| RF-16 | Parte B: as duas vias estão fechadas quando o pisca começa, e a escrava também começa a piscar em até 8,4 s | _preencher_ | _preencher_ |
| RF-17 | Saída: a primeira luz dos veículos depois do pisca é o vermelho, em até 1,05 s do `pedido NORMAL` na placa do botão, nas 5 saídas | _preencher_ | _preencher_ |
| RF-17 | Parte B: a escrava vai para o vermelho em até 2,1 s do pedido, pelo tempo do PC | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-08 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-08 na seção 5 do [plano de testes](../../plano-testes.md).
