### 1. TI-01: semáforo de pedestres e carros

- **Descrição:** testa a sincronização entre os semáforos de pedestres e carros
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
- **Resultado esperado:** o semáforo de pedestres e o de carros devem estar sincronizados
- **Evidência:** 1 log e 1 vídeo por placa

### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/TI-01 tests/TI-01
```
Flashear:
```bash
west flash -d build/TI-01 -r pyocd
```
Gravar os logs (alterar o N da placa):
```bash
python3 tests/ferramentas/captura_serial.py tests/TI-01/placaN.log unica=/dev/ttyACM0
```

### 4. Resultado obtido

Os vídeos e os logs demonstram que os semáforos das duas placas estão funcionando. 

### 5. Conclusão

O teste TI-01 foi aprovado.