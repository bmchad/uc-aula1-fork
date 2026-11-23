# Plano de testes

Este plano detalha os testes (unitários, de integração, de sistema e de aceitação) do nosso projeto dos semáforos.

OBS.: para obter os logs, rodar: 

```bash
python3 tests/ferramentas/captura_serial.py tests/TU-01/placa1.log unica=/dev/ttyACM0
``` 

para uma placa. Ou, para duas placas:

```bash
python3 tests/ferramentas/captura_serial.py tests/TU-03/captura.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
``` 

## 1. Testes unitários

### TU-01: semáforo dos pedestres

- **Descrição**: testa o correto funcionamento do semáforo dos pedestres (4s vermelho, 4s verde)
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
- **Resultado esperado:** os LEDs verde e vermelho devem piscar em sincronia (4s vermelho, 4s verde)
- **Evidência:** 1 log e 1 vídeo por placa

### TU-02: semáforo dos carros

- **Descrição**: testa o correto funcionamento do semáforo dos carros (4s vermelho, 3s verde, 1s amarelo)
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
- **Resultado esperado:** os LEDs verde, vermelho e amarelo devem piscar em sincronia (4s vermelho, 3s verde, 1s amarelo)
- **Evidência:** 1 log e 1 vídeo por placa

### TU-03: comunicação UART

- **Descrição**: testa a comunicação UART entre as placas
- **Pré-condição:** placas compiladas com o código contido no diretório do teste, resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
- **Resultado esperado:** as duas placas devem registrar na serial que o enlace está OK, e cada uma deve receber o sinal que a outra alterna a cada 1 s, sem nenhuma falha do enlace enquanto os fios estiverem ligados
- **Evidência:** 1 log

## 2. Testes de integração

### TI-01: semáforo de pedestres e carros

- **Descrição:** testa a sincronização entre os semáforos de pedestres e carros
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
- **Resultado esperado:** o semáforo de pedestres e o de carros devem estar sincronizados
- **Evidência:** 1 log e 1 vídeo por placa


### TI-02: 2 vias - 2 semáforos de pedestres e 2 de carros

- **Descrição:** testa a sincronização entre os semáforos das duas vias
- **Pré-condição:** placas compiladas com o código contido no diretório do teste, resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
- **Resultado esperado:** as duas vias devem estar sincronizadas
- **Evidência:** 1 log e 1 vídeo

## 3. Testes de sistema

### TS-01: modo noturno

- **Descrição:** testa o funcionamento do botão do modo noturno
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
  2. Apertar o botão do modo noturno
- **Resultado esperado:** ao apertar o botão do modo noturno, a placa deve entrar no modo NOTURNO (semáforo de carros piscando amarelo e o de pedestres piscando vermelho)
- **Evidência:** 1 log e 1 vídeo

### TS-02: botão de travessia

- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
  2. Apertar o botão de travessia quando o semáforo de carros estiver vermelho
  3. Apertar o botão de travessia quando o semáforo de carros estiver verde
  4. Apertar o botão de travessia quando o semáforo de carros estiver amarelo
- **Resultado esperado:** ao apertar o botão de travessia quando o semáforo dos carros estiver vermelho ou amarelo, nada deveria acontecer (e o tempo dos semáforos não deveria ser alterado). Caso o semáforo dos carros esteja verde, ele deve fechar e o semáforo de pedestres deve abrir
- **Evidência:** 1 log e 1 vídeo

## 4. Testes de aceitação

### TA-01: modo normal, noturno e botão de travessia

- **Descrição:** valida o correto funcionamento do modo normal, do modo noturno e do botão de travessia
- **Requisitos:** RF-01, RF-02, RF-03.
- **Pré-condição:** placas compiladas com o firmware principal (../src), resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
  2. Esperar dois ciclos completos
  3. Apertar o botão do modo noturno, esperar aproximadamente 3 segundos
  4. Apertar novamente o botão do modo noturno
  5. Apertar o botão de travessia em cada estado do semáforo de carros da placa mestra
- **Resultado esperado:** o sistema deverá agir de acordo com os critérios estabelecidos em [RF-01](../docs/01-requisitos.md), [RF-02](../docs/01-requisitos.md) e [RF-03](../docs/01-requisitos.md)
- **Evidência:** 1 log e 1 vídeo

### TA-02: uma placa desconectada

- **Requisitos:** RF-04.
- **Pré-condição:** placas compiladas com o firmware principal (../src), resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
  2. Desconectar uma das placas
  3. Reconectar a placa. Resetá-las
  4. Desconectar a outra placa
- **Resultado esperado:** o sistema deverá agir de acordo com o critério estabelecido em [RF-04](../docs/01-requisitos.md)
- **Evidência:** 1 log e 1 vídeo

### TA-03: fios de comunicação soltos

- **Requisitos:** RF-05.
- **Pré-condição:** placas compiladas com o firmware principal (../src), resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
  2. Desconectar o TX da mestra. Esperar ~4 segundos.
  3. Reconectar o TX da mestra. Esperar ~4 segundos.
  4. Desconectar o RX da mestra. Esperar ~4 segundos.
  5. Reconectar o RX da mestra. Esperar ~4 segundos.
  6. Desconectar o RX e o TX da mestra. Esperar ~4 segundos.
  7. Reconectar o RX e o TX da mestra. Esperar ~4 segundos.
- **Resultado esperado:** o sistema deverá agir de acordo com o critério estabelecido em [RF-05](../docs/01-requisitos.md)
- **Evidência:** 1 log e 1 vídeo

### TA-04: placa resetada

- **Requisitos:** RF-06.
- **Pré-condição:** placas compiladas com o firmware principal (../src), resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
  2. Resetar a placa mestra. Esperar ~4 segundos.
  3. Resetar a placa escrava. Esperar ~4 segundos.
- **Resultado esperado:** o sistema deverá agir de acordo com o critério estabelecido em [RF-06](../docs/01-requisitos.md)
- **Evidência:** 1 log e 1 vídeo

## 5. Situação atual

| Teste | Situação | Evidência existente |
|---|---|---|
| TU-01 | Pendente | - |
| TU-02 | Pendente | - |
| TU-03 | Pendente | - |
| TI-01 | Pendente | - |
| TI-02 | Pendente | - |
| TS-01 | Pendente | - |
| TS-02 | Pendente | - |
| TA-01 | Pendente | - |
| TA-02 | Pendente | - |
| TA-03 | Pendente | - |
| TA-04 | Pendente | - |
