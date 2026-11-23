### 1. TU-01: semáforo dos pedestres

- **Descrição**: testa o correto funcionamento do semáforo dos pedestres (4s vermelho, 4s verde)
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
- **Resultado esperado:** os LEDs verde e vermelho devem piscar em sincronia (4s vermelho, 4s verde)
- **Evidência:** 1 log e 1 vídeo por placa

### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/TU-01 tests/TU-01
```
Flashear:
```bash
west flash -d build/TU-01 -r pyocd
```
Gravar os logs (alterar o N da placa):
```bash
python3 tests/ferramentas/captura_serial.py tests/TU-01/placaN.log unica=/dev/ttyACM0
```

### 4. Resultado obtido

Para as duas placas, é possível ver nitidamente os LEDs piscando pelo vídeo. Já no log, a placa1 apresentou uma duração de 4000ms para o estado vermelho e 4000ms para o estado verde e a placa2 também.

### 5. Conclusão

O teste TU-01 foi aprovado.