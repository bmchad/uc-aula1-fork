## Prompt para IA: demonstração de Race Condition

Copie o bloco abaixo e cole em uma IA para gerar o código de `src/main.c` desta branch (`race-condition`).

```text
Atue como engenheiro de sistemas embarcados. Escreva um programa em C para a placa
FRDM-KL25Z (Cortex-M0+, PlatformIO, framework Zephyr 4.2) que demonstre uma
race condition causada por acesso não sincronizado a um recurso compartilhado.

Cenário real: contador de vagas de um estacionamento. Duas portarias (entrada e
saída) são duas threads que compartilham uma variável `vagas_ocupadas`. Cada carro
que entra faz +1 e cada carro que sai faz -1. Cada portaria processa 1000 carros.
Ao final, o valor esperado é 0.

Requisitos:
1. O código deve compilar e rodar na placa sem alterar prj.conf nem platformio.ini.
2. Use K_THREAD_DEFINE para as duas threads, com a mesma prioridade.
3. A atualização do contador deve ser uma sequência ler -> modificar -> escrever.
   Entre a leitura e a escrita, chame k_yield() para que a outra thread possa
   rodar e a atualização se intercale. Não use atômicos nem desabilite interrupções
   na versão sem proteção, porque isso esconderia o problema.
4. Use uma macro USAR_MUTEX. Com 0, o programa mostra a race condition. Com 1, a
   mesma seção crítica é protegida por k_mutex e o resultado deve ser 0.
5. A main deve esperar as duas threads terminarem (por k_sem), imprimir o valor
   final com printk e indicar OK ou ERRO com o número de atualizações perdidas.
6. Comente o código em português, explicando a seção crítica e por que o erro
   acontece.
7. Ao final, explique em português: (a) em qual linha a intercalação ocorre,
   (b) por que o resultado muda de execução para execução ou de modo para modo,
   (c) por que volatile sozinho não resolve, e (d) como o k_mutex resolve.
```

Resultado esperado: sem mutex, o contador final é diferente de 0 (com valor variável). Com mutex, o contador final é 0.
