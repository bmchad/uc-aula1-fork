# Análise de tests/evidencias/TA-02/parteA.log

**Todas as 8 conferências passaram.**

## Resumo

- Log: `tests/evidencias/TA-02/parteA.log`
- Placas: unica (parte A, uma placa)
- unica: 35 eventos, 1 sessão(ões) (cada reinício abre uma sessão)
- Duração pelo tempo do PC: 50,161 s

## Tempos do ciclo normal (RF-03, RF-04)

Medidos pelo tempo da placa. Fases excluídas não entram na conferência.

| Placa | Semáforo | Fase | Medidas | Mínimo (s) | Máximo (s) | Faixa (s) | Excluídas |
|---|---|---|---|---|---|---|---|
| unica | carros | VERDE | 6 | 3,003 | 3,003 | 2,970 a 3,030 | — |
| unica | carros | AMARELO | 6 | 1,002 | 1,002 | 0,990 a 1,010 | — |
| unica | carros | VERMELHO | 6 | 4,005 | 4,005 | 3,960 a 4,040 | partida: 1 |
| unica | pedestres | VERDE | 6 | 4,000 | 4,000 | 3,960 a 4,040 | partida: 1 |
| unica | pedestres | VERMELHO | 5 | 4,010 | 4,010 | 3,960 a 4,040 | partida: 2 |

- **[OK] RF-03:** unica: carros VERDE dentro da faixa
- **[OK] RF-03:** unica: carros AMARELO dentro da faixa
- **[OK] RF-03:** unica: carros VERMELHO dentro da faixa
- **[OK] RF-04:** unica: pedestres VERDE dentro da faixa
- **[OK] RF-04:** unica: pedestres VERMELHO dentro da faixa

## Sincronismo e segurança dentro de cada via (RF-05, RS-01, RS-03)

- **[OK] RF-05:** unica: troca do outro semáforo da via em até 0,050 s (14 trocas, pior atraso 0,004 s em 0,013 s)
- **[OK] RS-03:** unica: dentro da via, o semáforo que fecha fecha antes de o outro abrir
- **[OK] RS-01:** unica: verde dos pedestres nunca junto com os veículos abertos

## Botão de travessia (RF-07 a RF-11)

Nenhum toque de travessia no log.

## Modo noturno (RF-13 a RF-17)

Nenhum modo noturno no log.

## Enlace e avisos de espera (RS-05, RS-06)

Nenhum evento do enlace nem aviso de espera no log.
