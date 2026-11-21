## Notas

- Troquei os fios de sinalização por UART (TX da mestra vai no RX da escrava). Isso é melhor pois:
    - exige menos fios
    - 
- O que acontece se uma pessoa ficar clicando o botão de travessia infinitamente? Por causa do debounce, não rodará nunca a requisição de travessia, aí o ciclo só funcionará no modo normal mesmo. Essa é a minha hipótese