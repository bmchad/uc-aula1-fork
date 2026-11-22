# TA-09: operação com duas placas

Teste descrito no [plano de testes](../../plano-testes.md#ta-09-operação-com-duas-placas). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-19, RF-20, RS-06 |
| **Nível** | Integração. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | `-DPAPEL=mestre` numa placa e `-DPAPEL=escrava` na outra. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- Etapa 1: cada placa sozinha, com o D12 ligado no D11 da própria placa por um resistor de 1 kΩ.
- Etapas 2 e 3: cada placa sozinha, sem nenhum fio no D11 nem no D12.
- Etapas 4 e 5: as duas placas com os três fios da seção 1.4 do 03: GND primeiro, TX (D12) do mestre no RX (D11) da escrava e TX da escrava no RX do mestre, com 1 kΩ em série nos fios de dados.

## 2. Capturas

Para cada log, gravar com a captura e analisar. Por exemplo, para o primeiro:

```bash
python3 tests/ferramentas/captura_serial.py tests/evidencias/TA-09/etapa1_laco_mestre.log mestre=/dev/ttyACM0
```

```bash
python3 tests/ferramentas/analisa_log.py tests/evidencias/TA-09/etapa1_laco_mestre.log --relatorio tests/evidencias/TA-09/etapa1_laco_mestre_analise.md
```

Confira as portas com `ls -l /dev/serial/by-id/` e a linha `Semaforo iniciado como ...` de cada placa.

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `etapa1_laco_mestre.log` | Etapa 1: laço no mestre, 20 s | `etapa1_laco_mestre_analise.md` | _preencher_ |
| `etapa1_laco_escrava.log` | Etapa 1: laço na escrava, 20 s | `etapa1_laco_escrava_analise.md` | _preencher_ |
| `etapa2_escrava_sozinha.log` | Etapa 2: escrava sozinha, 25 s desde o boot | `etapa2_escrava_sozinha_analise.md` | _preencher_ |
| `etapa3_mestre_sozinho.log` | Etapa 3: mestre sozinho, 25 s desde o boot | `etapa3_mestre_sozinho_analise.md` | _preencher_ |
| `etapa4_revezamento.log` | Etapa 4: revezamento, pelo menos 6 ciclos, cerca de 50 s | `etapa4_revezamento_analise.md` | _preencher_ |
| `etapa5_noturno.log` | Etapa 5: modo noturno ligado e desligado 5 vezes, cerca de 1 min e meio | `etapa5_noturno_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RS-06 | Etapa 1, nas duas placas: `enlace: quadro com o mesmo papel desta placa` aparece, e `enlace conectado` nunca aparece | _preencher_ | _preencher_ |
| RS-06 | Etapa 2: a escrava fica no vermelho, com os pedestres no verde, avisa `aguardando a vez do mestre` a cada 10 s e nunca registra `enlace conectado` | _preencher_ | _preencher_ |
| RS-06 | Etapa 3: o mestre fica no vermelho, com os pedestres no verde, e avisa `aguardando a escrava parar` a cada 10 s | _preencher_ | _preencher_ |
| RF-19 | Etapa 4: as duas placas registram `enlace conectado` logo depois do boot | _preencher_ | _preencher_ |
| RF-19 | Etapa 4: pelo tempo do PC, as vias abrem alternadamente (A, B, A, B), e uma só abre depois de a outra fechar | _preencher_ | _preencher_ |
| RF-20 | Etapa 5: as duas placas entram e saem do pisca no mesmo ciclo, nas 5 vezes | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-09 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-09 na seção 5 do [plano de testes](../../plano-testes.md).
