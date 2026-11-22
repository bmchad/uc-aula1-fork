# TA-04: partida e reinício

Teste descrito no [plano de testes](../../plano-testes.md#ta-04-partida-e-reinício). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-06 (e RS-04 durante os reinícios) |
| **Nível** | Sistema. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- As duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados.
- O RESET da placa não derruba a porta serial; tirar o USB derruba, e a captura reabre a porta sozinha.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-04/partida.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-04/partida.log --relatorio tests/evidencias/TA-04/partida_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `partida.log` | Etapa 1: 5 partidas das duas placas | `partida_analise.md` | _preencher_ |
| `reinicio_mestre.log` | Etapa 2: 5 reinícios do mestre, no verde, no amarelo e no vermelho da via A | `reinicio_mestre_analise.md` | _preencher_ |
| `reinicio_escrava.log` | Etapa 2: 5 reinícios da escrava, no verde, no amarelo e no vermelho da via B | `reinicio_escrava_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-06 | Partida: a primeira linha `carros` das duas placas é `VERMELHO`, nas 5 partidas | _preencher_ | _preencher_ |
| RF-06 | Partida: o ciclo começa sem nenhum comando | _preencher_ | _preencher_ |
| RF-06 | Reinício do mestre: a primeira linha `carros` dele é `VERMELHO`, nos 5 reinícios, e o revezamento volta sozinho | _preencher_ | _preencher_ |
| RF-06 | Reinício da escrava: a primeira linha `carros` dela é `VERMELHO`, nos 5 reinícios, e o revezamento volta sozinho | _preencher_ | _preencher_ |
| RS-04 | Em nenhum momento as duas vias ficam abertas (seção "Segurança entre as vias" da análise) | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-04 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-04 na seção 5 do [plano de testes](../../plano-testes.md).
