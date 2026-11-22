# TA-13: registro na serial e modo de placa única

Teste descrito no [plano de testes](../../plano-testes.md#ta-13-registro-na-serial-e-modo-de-placa-única). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RNF-04, RNF-06 |
| **Nível** | Unitário. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | Os três papéis: `mestre`, `escrava` e `placa_unica`. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Evidências

| Arquivo | O que é |
|---|---|
| `build_mestre.txt` | Saída do `west build -b frdm_kl25z -d build/mestre -- -DPAPEL=mestre` |
| `build_escrava.txt` | Saída do `west build -b frdm_kl25z -d build/escrava -- -DPAPEL=escrava` |
| `build_placa_unica.txt` | Saída do `west build -b frdm_kl25z -d build/placa_unica -- -DPAPEL=placa_unica` |
| `boot.log` | Captura do boot da placa única, com pelo menos um ciclo completo |

Para guardar a saída de um build, acrescente `> tests/evidencias/TA-13/build_mestre.txt 2>&1` ao fim do comando.

## 2. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RNF-06 | Os três papéis compilam sem erros nem avisos | _preencher_ | _preencher_ |
| RNF-06 | O boot da placa única mostra `Semaforo iniciado como PLACA UNICA (etapa 1)` | _preencher_ | _preencher_ |
| RNF-04 | Num ciclo completo, cada troca de luz aparece uma vez, no formato `tempo ms \| evento` | _preencher_ | _preencher_ |
| RNF-04 | Nas partes B dos outros testes, cada linha das duas seriais tem o tempo do PC | _preencher_ | _preencher_ |
| RNF-06 | A parte A do TA-01 ao TA-08 passou com `placa_unica` (ver os README desses testes) | _preencher_ | _preencher_ |

## 3. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 4. Conclusão

_preencher_: o TA-13 é aprovado se todos os critérios da seção 2 passarem. Depois, atualizar a linha do TA-13 na seção 5 do [plano de testes](../../plano-testes.md).
