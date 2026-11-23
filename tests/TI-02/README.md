### 1. TI-02: 2 vias - 2 semáforos de pedestres e 2 de carros

- **Descrição:** testa a sincronização entre os semáforos das duas vias
- **Pré-condição:** placas compiladas com o código contido no diretório do teste, resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
- **Resultado esperado:** as duas vias devem estar sincronizadas
- **Evidência:** 1 log e 1 vídeo

### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/TI-02-mestre tests/TI-02 -- -DPAPEL=mestre
west build -b frdm_kl25z -d build/TI-02-escrava tests/TI-02 -- -DPAPEL=escrava
```
Flashear:
```bash
west flash -d build/TI-02-mestre -r pyocd
west flash -d build/TI-02-escrava -r pyocd
```
Gravar os logs (alterar o N da placa):
```bash
python3 tests/ferramentas/captura_serial.py tests/TI-02/captura.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
```

### 4. Resultado obtido

Pelo vídeo, é possível ver que os semáforos estão funcionando corretamente (estão sincronizados).

### 5. Conclusão

O teste TI-02 foi aprovado.