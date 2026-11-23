### 1. TA-02: uma placa desconectada

- **Requisitos:** RF-04.
- **Pré-condição:** placas compiladas com o firmware principal (../src), resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
  2. Desconectar uma das placas
  3. Reconectar a placa. 
  4. Desconectar a outra placa
- **Resultado esperado:** o sistema deverá agir de acordo com o critério estabelecido em [RF-04](../docs/01-requisitos.md)
- **Evidência:** 1 vídeo

### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/mestre -- -DPAPEL=mestre
west build -b frdm_kl25z -d build/escrava -- -DPAPEL=escrava
```
Flashear:
```bash
west flash -d build/mestre -r pyocd
west flash -d build/escrava -r pyocd
```

### 4. Resultado obtido

Após desconectar uma placa, a outra entrou no modo ALERTA, assim como foi especificado no [RF-04](../../docs/01-requisitos.md).

### 5. Conclusão

O TA-02 foi aprovado.