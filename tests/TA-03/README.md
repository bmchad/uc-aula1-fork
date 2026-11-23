### 1. TA-03: fios de comunicação soltos

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

No vídeo, é possível ver que, caso qualquer um dos fios (RX, TX ou ambos) é solto, as duas placas entram no modo alerta, assim como especifica o [RF-05](../../docs/01-requisitos.md).

### 5. Conclusão

O TA-03 foi aprovado.