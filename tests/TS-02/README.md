### 1. TS-02: botão de travessia

- **Descrição:** testa o funcionamento do botão de travessia
- **Pré-condição:** placa compilada com o código contido no diretório do teste, resetada e com a serial ligada
- **Etapas:**
  1. Rodar o código e resetar a placa
  2. Apertar o botão de travessia quando o semáforo de carros estiver vermelho
  3. Apertar o botão de travessia quando o semáforo de carros estiver verde
  4. Apertar o botão de travessia quando o semáforo de carros estiver amarelo
- **Resultado esperado:** ao apertar o botão de travessia quando o semáforo dos carros estiver vermelho ou amarelo, nada deveria acontecer (e o tempo dos semáforos não deveria ser alterado). Caso o semáforo dos carros esteja verde, ele deve fechar e o semáforo de pedestres deve abrir
- **Evidência:** 1 vídeo

### 2. Montagem

Seguir a montagem descrita na [seção 1 de 03-projeto-componentes.md (Hardware de cada placa)](../../docs/03-projeto-componentes.md#1-hardware-de-cada-placa)

### 3. Como rodar

Buildar:
```bash
west build -b frdm_kl25z -d build/TS-02 tests/TS-02
```
Flashear:
```bash
west flash -d build/TS-02 -r pyocd
```

### 4. Resultado obtido

Pelo vídeo, é possível ver que o botão de travessia funciona corretamente: quando ele é apertado no estado VERMELHO ou AMARELO do semáforo de carros, nada ocorre. Quando ele é apertado no estado VERDE, o semáforo de carros fecha e o de pedestres abre.

### 5. Conclusão

O teste TS-02 foi aprovado.