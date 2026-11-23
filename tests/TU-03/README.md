### 1. TU-03: comunicação UART

- **Descrição**: testa a comunicação UART entre as placas
- **Pré-condição:** placas compiladas com o código contido no diretório do teste, resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
- **Resultado esperado:** as duas placas devem registrar na serial que o enlace está OK, e cada uma deve receber o sinal que a outra alterna a cada 1 s, sem nenhuma falha do enlace enquanto os fios estiverem ligados
- **Evidência:** 1 log

### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/TU-03-mestre tests/TU-03 -- -DPAPEL=mestre
west build -b frdm_kl25z -d build/TU-03-escrava tests/TU-03 -- -DPAPEL=escrava
```
Flashear:
```bash
west flash -d build/TU-03-mestre -r pyocd
west flash -d build/TU-03-escrava -r pyocd
```
Gravar os logs:
```bash
python3 tests/ferramentas/captura_serial.py tests/TU-03/captura.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
```

### 4. Resultado obtido

Pelo log, é possível ver que a comunicação UART está funcionando (sinal está sendo enviado e recebido).

### 5. Conclusão

O teste TU-03 foi aprovado.