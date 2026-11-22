# TA-14: inspeção do repositório

Teste descrito no [plano de testes](../../plano-testes.md#ta-14-inspeção-do-repositório). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RNF-07, RNF-08 |
| **Nível** | Processo. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | — |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Evidências

| Arquivo | O que é |
|---|---|
| `shortlog.txt` | Saída do `git shortlog -sn` no commit final |
| `anexos.txt` | Lista dos anexos citados no plano e nos README de `tests/evidencias/`, com a situação de cada um |

## 2. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RNF-07 | Os dois integrantes aparecem no `git shortlog -sn` | _preencher_ | _preencher_ |
| RNF-07 | Cada integrante tem commits de desenvolvimento e de testes | _preencher_ | _preencher_ |
| RNF-08 | Todos os anexos citados existem no repositório ou têm um link que funciona | _preencher_ | _preencher_ |

## 3. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 4. Conclusão

_preencher_: o TA-14 é aprovado se todos os critérios da seção 2 passarem. Depois, atualizar a linha do TA-14 na seção 5 do [plano de testes](../../plano-testes.md).
