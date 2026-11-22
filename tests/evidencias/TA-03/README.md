# TA-03: sincronismo e ordem das trocas

Teste descrito no [plano de testes](../../plano-testes.md#ta-03-sincronismo-e-ordem-das-trocas). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RF-05, RS-03 |
| **Nível** | Integração. |
| **Data** | _preencher_ |
| **Executado por** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | Os mesmos do TA-02. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Montagem e pré-condição

- Não precisa de captura nova: usa os logs do TA-02 e as análises deles.

## 2. Capturas

| Arquivo | O que é | Análise | O que foi feito |
|---|---|---|---|
| `../TA-02/parteA.log` | Log da parte A do TA-02 | `../TA-02/parteA_analise.md` | _preencher_ |
| `../TA-02/parteB.log` | Log da parte B do TA-02 | `../TA-02/parteB_analise.md` | _preencher_ |

Troque cada nome por um link (`[nome](nome)`) quando o arquivo estiver na pasta.

## 3. Critérios

| Requisito | Critério | Medido | Resultado |
|---|---|---|---|
| RF-05 | Parte A: em cada troca, o outro semáforo da via muda em até 50 ms | _preencher_ | _preencher_ |
| RF-05 | Parte B, nas duas vias: em cada troca, o outro semáforo da via muda em até 50 ms | _preencher_ | _preencher_ |
| RS-03 | Dentro de cada via: o verde dos pedestres só depois do vermelho dos veículos, e o verde dos veículos só depois do vermelho dos pedestres | _preencher_ | _preencher_ |
| RS-03 | Entre as vias (parte B, pelo tempo do PC): o `carros VERMELHO` de uma via sempre antes do `carros VERDE` da outra; anotar a menor folga | _preencher_ | _preencher_ |

## 4. Observações

_preencher_ (por exemplo: o que deu diferente do esperado, refações, problemas de montagem).

## 5. Conclusão

_preencher_: o TA-03 é aprovado se todos os critérios da seção 3 passarem. Depois, atualizar a linha do TA-03 na seção 5 do [plano de testes](../../plano-testes.md).
