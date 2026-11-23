

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



### 5. Conclusão
