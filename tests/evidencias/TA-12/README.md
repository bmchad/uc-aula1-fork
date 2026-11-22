# TA-12: revisão de código

Teste descrito no [plano de testes](../../plano-testes.md#ta-12-revisão-de-código). Os campos marcados com _preencher_ ainda faltam.

| | |
|---|---|
| **Requisitos** | RNF-01, RNF-02, RNF-03 e a revisão de código do RF-21 |
| **Nível** | Unitário. |
| **Data** | _preencher_ |
| **Revisado por (quem não escreveu o trecho)** | _preencher_ |
| **Commit testado** | _preencher_ (`git rev-parse --short HEAD`, sem mudanças pendentes no firmware) |
| **Firmware** | O commit revisado. |
| **Situação** | _preencher_: aprovado, reprovado ou pendente |

## 1. Lista de revisão

Para cada item, citar o arquivo e a linha que comprovam.

| Item | Requisito | Confere | Arquivo e linha | Resultado |
|---|---|---|---|---|
| 1 | RNF-01 | Há uma thread por luz: três dos veículos e duas dos pedestres | _preencher_ | _preencher_ |
| 2 | RNF-01 | Cada thread só escreve no pino da própria luz. A do amarelo escreve nos dois pinos do RGB, e só ela faz isso. As threads auxiliares (botão noturno, registro da travessia e enlace) não escrevem em nenhum LED | _preencher_ | _preencher_ |
| 3 | RNF-02 | Há um mutex por semáforo, travado antes de acender a luz e destravado depois de apagar, em todos os caminhos, inclusive no pisca | _preencher_ | _preencher_ |
| 4 | RNF-03 | As interrupções dos dois botões só chamam `k_sem_give`, e a da UART2 só copia o byte recebido para a fila com `k_msgq_put` sem espera | _preencher_ | _preencher_ |
| 5 | RNF-03 | Nenhuma rotina de interrupção usa mutex, `printk` ou espera | _preencher_ | _preencher_ |
| 6 | RF-21 | O mestre não configura o botão de travessia: `semaforo_veiculo_iniciar(false)` | _preencher_ | _preencher_ |

## 2. Problemas encontrados

_preencher_ (ou "nenhum"). Para cada problema, o item, o arquivo e a linha, e o commit que corrigiu.

## 3. Conclusão

_preencher_: o TA-12 é aprovado se os 6 itens forem conferidos. Depois, atualizar a linha do TA-12 na seção 5 do [plano de testes](../../plano-testes.md).
