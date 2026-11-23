### 1. TA-01: modo normal, noturno e botão de travessia

- **Descrição:** valida o correto funcionamento do modo normal, do modo noturno e do botão de travessia
- **Requisitos:** RF-01, RF-02, RF-03.
- **Pré-condição:** placas compiladas com o firmware principal (../src), resetadas e com a serial ligada
- **Etapas:**
  1. Rodar os códigos e resetar as placas
  2. Esperar dois ciclos completos
  3. Apertar o botão do modo noturno, esperar aproximadamente 3 segundos
  4. Apertar novamente o botão do modo noturno
  5. Apertar o botão de travessia 
- **Resultado esperado:** o sistema deverá agir de acordo com os critérios estabelecidos em [RF-01](../docs/01-requisitos.md), [RF-02](../docs/01-requisitos.md) e [RF-03](../docs/01-requisitos.md)
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

Pelo vídeo, é possível ver que os requisitos RF-01, RF-02 e RF-03 são satisfeitos: nos dois primeiros ciclos, as duas vias continuaram sincronizadas, sem nenhuma colisão; após um clique do botão do modo NOTURNO, ambas as placas entram no modo NOTURNO e após um clique do botão de travessia, o semáforo de carros fecha e o de pedestres abre, com a outra placa sincronizada também.

### 5. Conclusão

O teste TA-01 foi aprovado.