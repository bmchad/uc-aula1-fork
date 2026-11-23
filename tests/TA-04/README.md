### 1. TA-04: placa resetada

- **Requisitos:** RF-06.
- **Pré-condição:** placas compiladas com o firmware principal (../src), resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
  2. Resetar a placa mestra. Esperar ~4 segundos.
  3. Resetar a placa escrava. Esperar ~4 segundos.
- **Resultado esperado:** o sistema deverá agir de acordo com o critério estabelecido em [RF-06](../../docs/01-requisitos.md)
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

Mesmo resetando as placas, os semáforos de carros não ficaram abertos simultaneamente e o sistema não travou, assim como especifica o [RF-06](../../docs/01-requisitos.md)

### 5. Conclusão

O TA-04 foi aprovado.