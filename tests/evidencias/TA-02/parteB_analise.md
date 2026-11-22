# Análise de tests/evidencias/TA-02/parteB.log

**Todas as 18 conferências passaram.**

## Resumo

- Log: `tests/evidencias/TA-02/parteB.log`
- Placas: escrava, mestre (parte B, duas placas)
- escrava: 38 eventos, 1 sessão(ões) (cada reinício abre uma sessão)
- mestre: 44 eventos, 1 sessão(ões) (cada reinício abre uma sessão)
- Duração pelo tempo do PC: 59,049 s

## Tempos do ciclo normal (RF-03, RF-04)

Medidos pelo tempo da placa. Fases excluídas não entram na conferência.

| Placa | Semáforo | Fase | Medidas | Mínimo (s) | Máximo (s) | Faixa (s) | Excluídas |
|---|---|---|---|---|---|---|---|
| escrava | carros | VERDE | 7 | 3,002 | 3,003 | 2,970 a 3,030 | — |
| escrava | carros | AMARELO | 7 | 1,002 | 1,003 | 0,990 a 1,010 | — |
| escrava | carros | VERMELHO | 7 | 4,117 | 4,118 | 4,000 a 4,200 | — |
| escrava | pedestres | VERDE | 7 | 4,111 | 4,112 | 4,000 a 4,200 | — |
| escrava | pedestres | VERMELHO | 7 | 4,010 | 4,011 | 3,960 a 4,040 | — |
| mestre | carros | VERDE | 8 | 3,002 | 3,003 | 2,970 a 3,030 | — |
| mestre | carros | AMARELO | 8 | 1,002 | 1,003 | 0,990 a 1,010 | — |
| mestre | carros | VERMELHO | 7 | 4,117 | 4,117 | 4,000 a 4,200 | partida: 1 |
| mestre | pedestres | VERDE | 7 | 4,111 | 4,112 | 4,000 a 4,200 | partida: 1 |
| mestre | pedestres | VERMELHO | 7 | 4,010 | 4,011 | 3,960 a 4,040 | partida: 2 |

- **[OK] RF-03:** escrava: carros VERDE dentro da faixa
- **[OK] RF-03:** escrava: carros AMARELO dentro da faixa
- **[OK] RF-03:** escrava: carros VERMELHO dentro da faixa
- **[OK] RF-04:** escrava: pedestres VERDE dentro da faixa
- **[OK] RF-04:** escrava: pedestres VERMELHO dentro da faixa
- **[OK] RF-03:** mestre: carros VERDE dentro da faixa
- **[OK] RF-03:** mestre: carros AMARELO dentro da faixa
- **[OK] RF-03:** mestre: carros VERMELHO dentro da faixa
- **[OK] RF-04:** mestre: pedestres VERDE dentro da faixa
- **[OK] RF-04:** mestre: pedestres VERMELHO dentro da faixa

## Sincronismo e segurança dentro de cada via (RF-05, RS-01, RS-03)

- **[OK] RF-05:** escrava: troca do outro semáforo da via em até 0,050 s (15 trocas, pior atraso 0,003 s em 8,143 s)
- **[OK] RS-03:** escrava: dentro da via, o semáforo que fecha fecha antes de o outro abrir
- **[OK] RS-01:** escrava: verde dos pedestres nunca junto com os veículos abertos
- **[OK] RF-05:** mestre: troca do outro semáforo da via em até 0,050 s (17 trocas, pior atraso 0,005 s em 0,012 s)
- **[OK] RS-03:** mestre: dentro da via, o semáforo que fecha fecha antes de o outro abrir
- **[OK] RS-01:** mestre: verde dos pedestres nunca junto com os veículos abertos

## Segurança entre as vias (RS-03, RS-04)

Comparado pelo tempo do PC. O pisca do modo noturno não conta como via aberta.

- **[OK] RS-04:** as duas vias nunca abertas ao mesmo tempo
- **[OK] RS-03:** entre as vias, uma só abre depois de a outra fechar (15 aberturas normais, menor folga 0,052 s (escrava em 16,465 s do PC))

## Botão de travessia (RF-07 a RF-11)

Nenhum toque de travessia no log.

## Modo noturno (RF-13 a RF-17)

Nenhum modo noturno no log.

## Enlace e avisos de espera (RS-05, RS-06)

- mestre: enlace conectado em 0,050 s (tempo da placa), 4,659 s (PC)
- escrava: enlace conectado em 0,025 s (tempo da placa), 4,622 s (PC)
