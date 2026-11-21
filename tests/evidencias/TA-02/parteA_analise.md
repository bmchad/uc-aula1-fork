# Análise de tests/evidencias/TA-02/parteA.log

**Todas as 8 conferências passaram.**

## Resumo

- Log: `tests/evidencias/TA-02/parteA.log`
- Placas: placa1 (parte A, uma placa)
- placa1: 40 eventos, 1 sessão(ões) (cada reinício abre uma sessão)
- Duração pelo tempo do PC: 58,415 s

## Tempos do ciclo normal (RF-03, RF-04)

Medidos pelo tempo da placa. Fases excluídas não entram na conferência.

| Placa | Semáforo | Fase | Medidas | Mínimo (s) | Máximo (s) | Faixa (s) | Excluídas |
|---|---|---|---|---|---|---|---|
| placa1 | carros | VERDE | 7 | 3,003 | 3,003 | 2,970 a 3,030 | — |
| placa1 | carros | AMARELO | 7 | 1,002 | 1,002 | 0,990 a 1,010 | — |
| placa1 | carros | VERMELHO | 7 | 4,005 | 4,005 | 3,960 a 4,040 | partida: 1 |
| placa1 | pedestres | VERDE | 7 | 4,000 | 4,000 | 3,960 a 4,040 | partida: 1 |
| placa1 | pedestres | VERMELHO | 6 | 4,010 | 4,010 | 3,960 a 4,040 | partida: 2 |

- **[OK] RF-03:** placa1: carros VERDE dentro da faixa
- **[OK] RF-03:** placa1: carros AMARELO dentro da faixa
- **[OK] RF-03:** placa1: carros VERMELHO dentro da faixa
- **[OK] RF-04:** placa1: pedestres VERDE dentro da faixa
- **[OK] RF-04:** placa1: pedestres VERMELHO dentro da faixa

## Sincronismo e segurança dentro de cada via (RF-05, RS-01, RS-03)

- **[OK] RF-05:** placa1: troca do outro semáforo da via em até 0,050 s (16 trocas, pior atraso 0,004 s em 0,013 s)
- **[OK] RS-03:** placa1: dentro da via, o semáforo que fecha fecha antes de o outro abrir
- **[OK] RS-01:** placa1: verde dos pedestres nunca junto com os veículos abertos

## Botão de travessia (RF-07 a RF-11)

Nenhum toque de travessia no log.

## Modo noturno (RF-13 a RF-17)

Nenhum modo noturno no log.

## Avisos de espera (RS-05)

Nenhum aviso de espera no log.
