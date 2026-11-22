# TA-11: falhas do enlace e placa desligada

Teste descrito no [plano de testes](../../plano-testes.md#ta-11-falhas-do-enlace-e-placa-desligada). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RS-05, RS-06 |
| **Nível** | Sistema. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | `-DPAPEL=mestre` e `-DPAPEL=escrava`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- As duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados.
- Cada caso numa captura: provocar a falha no meio do ciclo, mantê-la por 25 s, desfazê-la e esperar o revezamento voltar. Digitar uma nota no instante da falha e no de desfazê-la.
- Cada caso é feito duas vezes: com a via A aberta e com a via B aberta.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-11/tx_mestre_viaA.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-11/tx_mestre_viaA.log --relatorio tests/evidencias/TA-11/tx_mestre_viaA_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `tx_mestre_viaA.log` | Fio TX do mestre solto (fio 2), com a via A aberta | `tx_mestre_viaA_analise.md` | _preencher_ |
| `tx_mestre_viaB.log` | Fio TX do mestre solto (fio 2), com a via B aberta | `tx_mestre_viaB_analise.md` | _preencher_ |
| `tx_escrava_viaA.log` | Fio TX da escrava solto (fio 3), com a via A aberta | `tx_escrava_viaA_analise.md` | _preencher_ |
| `tx_escrava_viaB.log` | Fio TX da escrava solto (fio 3), com a via B aberta | `tx_escrava_viaB_analise.md` | _preencher_ |
| `ruido_viaA.log` | Ruído no fio 2 (encostar e soltar do GND por 5 s), com a via A aberta | `ruido_viaA_analise.md` | _preencher_ |
| `ruido_viaB.log` | Ruído no fio 2 (encostar e soltar do GND por 5 s), com a via B aberta | `ruido_viaB_analise.md` | _preencher_ |
| `escrava_desligada_viaA.log` | Escrava desligada (USB tirado), com a via A aberta | `escrava_desligada_viaA_analise.md` | _preencher_ |
| `escrava_desligada_viaB.log` | Escrava desligada (USB tirado), com a via B aberta | `escrava_desligada_viaB_analise.md` | _preencher_ |
| `mestre_desligado_viaA.log` | Mestre desligado (USB tirado), com a via A aberta | `mestre_desligado_viaA_analise.md` | _preencher_ |
| `mestre_desligado_viaB.log` | Mestre desligado (USB tirado), com a via B aberta | `mestre_desligado_viaB_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RS-05, RS-06 | Fio TX do mestre solto: a escrava registra `enlace perdido`, fica no vermelho e avisa; o mestre cancela a vez e segue sozinho | _preencher_ | _preencher_ |
| RS-05, RS-06 | Fio TX da escrava solto: o mestre registra `enlace perdido`; as duas vias ficam no vermelho, e as duas placas avisam | _preencher_ | _preencher_ |
| RS-06 | Ruído: nenhuma via abre fora de hora | _preencher_ | _preencher_ |
| RS-05 | Escrava desligada: o mestre registra `enlace perdido` e para no vermelho, avisando | _preencher_ | _preencher_ |
| RS-05 | Mestre desligado: a escrava registra `enlace perdido`, termina a fase e fica no vermelho, avisando | _preencher_ | _preencher_ |
| RS-05 | Em todos os casos: nenhuma violação de RS-01 a RS-04 na placa ligada, que só para no vermelho fixo ou no pisca | _preencher_ | _preencher_ |
| RS-05 | Em todos os casos: parada no vermelho, a placa avisa pelo menos a cada 10 s | _preencher_ | _preencher_ |
| RS-05 | Em todos os casos: depois de desfeita a falha, o revezamento volta | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-11 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-11 na seção 5 do [plano de testes](../../plano-testes.md).
