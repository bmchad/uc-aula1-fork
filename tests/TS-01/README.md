### 1. TS-01: modo noturno

- **Descrição:** testa o funcionamento do botão do modo noturno
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
  2. Apertar o botão do modo noturno. Esperar.
  3. Apertar o botão do modo noturno novamente.
- **Resultado esperado:** ao apertar o botão do modo noturno, a placa deve entrar no modo NOTURNO (semáforo de carros piscando amarelo e o de pedestres piscando vermelho). Depois de apertá-lo novamente, o sistema deverá voltar ao modo normal
- **Evidência:** 1 vídeo

### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/TS-01 tests/TS-01
```
Flashear:
```bash
west flash -d build/TS-01 -r pyocd
```

### 4. Resultado obtido

Pelo vídeo, é possível ver que o botão do modo NOTURNO funciona corretamente.

### 5. Conclusão

O teste TS-01 foi aprovado.