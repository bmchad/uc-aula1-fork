# TA-01: inspeção das luzes

Teste descrito no [plano de testes](../../plano-testes.md#ta-01-inspeção-das-luzes).

| | |
|---|---|
| **Requisitos** | RF-01, RF-02 |
| **Nível** | Unitário. |
| **Data** | 21/11/2026 |
| **Executado por** | Felipe |
| **Commit testado** | 380c4b3 (`git rev-parse --short HEAD`). |
| **Firmware** | `-DPAPEL=placa_unica`, Zephyr v4.5.0-660-ge08106651fc7 |
| **Situação** | Aprovado |

## 1. Montagem e pré-condição

- Cada placa sozinha, montada como na seção 1.5 do [03-projeto-componentes.md](../../../docs/03-projeto-componentes.md).
- Veículos: LED RGB da placa (vermelho no PTB18, verde no PTB19, amarelo com os dois acesos).
- Pedestres: LED vermelho no D6 e LED verde no D7, cada um com resistor de 220 Ω até o GND.
- Nada ligado no D11 nem no D12, e os botões não são usados.
- Uma captura por placa, começada antes do RESET, com pelo menos 2 ciclos filmados.

## 2. Capturas

| Arquivo | O que é | 
|---|---|
| [placa1.log](placa1.log) | Serial da placa 1, do boot até cerca de 20 s | 
| [placa1.mp4](placa1.mp4) | Vídeo da placa 1 |
| [placa2.log](placa2.log) | Serial da placa 2, do boot até cerca de 20 s | 
| [placa2.mp4](placa2.mp4) | Vídeo da placa 2 |

**Como achar no vídeo o instante de cada linha do log:** escolha uma troca fácil de ver, como o primeiro verde dos veículos. Anote o tempo dela no vídeo e o tempo da placa no log. A diferença entre os dois vale para as outras linhas.

## 3. Critérios

### Placa 1

**Critérios de aprovação:**

| Requisito | Critério | Resultado |
|---|---|---|
| RF-01 | As três luzes dos veículos são distinguíveis a olho nu | Aprovado |
| RF-01 | O amarelo (vermelho e verde do RGB juntos) não se confunde com o vermelho nem com o verde | Aprovado |
| RF-02 | As duas luzes dos pedestres são distinguíveis a olho nu | Aprovado |
| RF-01, RF-02 | Em cada troca, o vídeo mostra a luz que o log registra | Aprovado |

### Placa 2

**Critérios de aprovação:**

| Requisito | Critério | Resultado |
|---|---|---|
| RF-01 | As três luzes dos veículos são distinguíveis a olho nu | Aprovado |
| RF-01 | O amarelo (vermelho e verde do RGB juntos) não se confunde com o vermelho nem com o verde | Aprovado |
| RF-02 | As duas luzes dos pedestres são distinguíveis a olho nu | Aprovado |
| RF-01, RF-02 | Em cada troca, o vídeo mostra a luz que o log registra | Aprovado |

## 4. Observações

- O LED verde está meio fraco, mas dá para ver

## 5. Conclusão

O teste TA-01 foi aprovado em todos os critérios.
