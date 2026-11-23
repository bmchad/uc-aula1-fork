### 1. TU-02: semáforo dos carros

- **Descrição**: testa o correto funcionamento do semáforo dos carros (4s vermelho, 3s verde, 1s amarelo)
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
- **Resultado esperado:** os LEDs verde, vermelho e amarelo devem piscar em sincronia (4s vermelho, 3s verde, 1s amarelo)
- **Evidência:** 1 log e 1 vídeo por placa


### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/TU-02 tests/TU-02
```
Flashear:
```bash
west flash -d build/TU-02 -r pyocd
```
Gravar os logs (alterar o N da placa):
```bash
python3 tests/ferramentas/captura_serial.py tests/TU-02/placaN.log unica=/dev/ttyACM0
```

### 4. Resultado obtido

Para as duas placas, é possível ver nitidamente os LEDs piscando. Já no lo, a placa1 apresentou uma duração de 4009ms para o estado vermelho, 3002ms para o estado verde e 1003ms para o estado amarelo. O mesmo ocorreu para a placa2.

### 5. Conclusão

O teste TU-02 foi aprovado.