## 1. Requisitos funcionais

O sistema controla **duas vias** que se cruzam. Cada placa controla uma via completa: o semáforo de veículos e o de pedestres dessa via. As duas vias trabalham em fases opostas: quando uma está aberta, a outra está fechada.

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RF-01 | O sistema deverá controlar duas vias que se cruzam, sem travar e sem nunca abrir os dois semáforos de carros simultaneamente | Enunciado | Em 2 ciclos, o sistema deverá funcionar normalmente |
| RF-02 | O sistema deverá possuir um botão de modo noturno que, caso apertado, deve fazer com que as duas placas entrem no modo NOTURNO (semáforos de carros piscando amarelo e os de pedestres piscando vermelho) | Enunciado | Após um clique no botão do modo noturno, ambas as placas devem entrar no modo NOTURNO |
| RF-03 | O sistema deverá possuir um botão de travessia que, caso apertado, deve fechar o semáforo de carros e abrir o de pedestres (sem que ocorra uma colisão entre os dois semáforos de carros, isto é, sem que os dois semáforos de carros fiquem abertos no mesmo instante) | Enunciado | Após um clique no botão de travessia, o semáforo de carros deve fechar e o de pedestres deve abrir, averiguando a sincronização com a outra placa também (como o semáforo de carros da via escrava fechará, então o da mestra deverá abrir) |
| RF-04 | Caso uma das duas placas seja desconectada, a outra deverá entrar no modo ALERTA (semáforo de carros piscando amarelo e o de pedestres piscando vermelho) | Dupla, para uma maior segurança do sistema | Após desconectar uma placa, a outra deverá entrar no modo ALERTA (repetir o procedimento com a outra placa) |
| RF-05 | Caso algum dos fios de comunicação (RX ou TX) entre as placas seja desconectado, as duas placas deverão entrar no modo ALERTA | Dupla, para uma maior segurança do sistema | Após desconectar algum dos fios entre as placas (RX, TX ou ambos), as duas placas deverão entrar no modo ALERTA. |
| RF-06 | Caso alguma das placas reinicie, o sistema deverá continuar funcionando normalmente | Dupla, para uma maior segurança do sistema | Garantir que, mesmo com o reinício de uma placa, os semáforos de carros continuam sincronizados (eles nunca devem ficar abertos no mesmo instante) e o sistema continua funcionando normalmente (não trava). Durante o reinício, a outra placa pode entrar no modo ALERTA (RF-04), mas as duas devem voltar ao modo normal depois dele |


## 2. Requisitos não funcionais

| ID | Requisito | Origem | Critério de aceitação |
|---|---|---|---|
| RNF-01 | Cada luz deve ser controlada por uma thread própria: três threads para os veículos e duas para os pedestres. | Enunciado | Revisão de código: uma thread por luz, e nenhuma thread acende a luz de outra |
| RNF-02 | A exclusão mútua entre as luzes de um mesmo semáforo deve usar mutex. | Enunciado | Revisão de código: um mutex por semáforo, travado por toda thread antes de acender sua luz |
