# Semáforos de pedestres e veículos

Um cruzamento de duas ruas controlado por duas placas FRDM-KL25Z. Cada placa cuida de uma rua inteira, com o semáforo dos carros e o dos pedestres. As duas placas conversam por UART para que **as duas ruas nunca abram ao mesmo tempo**.

## Demonstração

▶️ **[Assista ao vídeo da demonstração](tests/TA-01/captura.mp4)**

O vídeo mostra as duas placas funcionando juntas:

1. **Modo normal:** as ruas se revezam. Enquanto os carros de uma rua passam, os da outra esperam no vermelho, e os pedestres atravessam onde os carros estão parados.
2. **Modo noturno:** com um toque no botão, as duas placas passam a piscar: amarelo para os carros e vermelho para os pedestres. Outro toque volta ao modo normal.
3. **Botão de travessia:** o pedestre aperta o botão, o semáforo dos carros fecha, os pedestres atravessam, e a outra rua abre em seguida.

## Como funciona

Cada placa tem:
- **Semáforo dos carros:** o LED colorido da própria placa (verde, amarelo e vermelho).
- **Semáforo dos pedestres:** dois LEDs na protoboard, um verde e um vermelho.

Num ciclo normal, os carros ficam 3 s no verde, 1 s no amarelo e 4 s no vermelho. Os pedestres ficam no verde enquanto os carros estão no vermelho.

Uma placa é a **mestre**, que decide de quem é a vez e tem o botão do modo noturno. A outra é a **escrava**, que tem o botão de travessia.

O sistema também se protege contra falhas. Se uma placa for desligada ou um fio entre elas se soltar, as duas entram no **modo alerta** e piscam como no modo noturno. Quando tudo volta ao lugar, elas retomam o funcionamento normal sozinhas.

## Testes

Seguimos o modelo V: cada parte foi testada sozinha, depois juntas, e por fim o sistema completo. O plano está em [tests/plano-testes.md](tests/plano-testes.md), e cada teste tem uma pasta própria em [tests/](tests/), com o procedimento, os vídeos e os registros.

| Nível | O que foi testado |
|---|---|
| Unitário | Cada semáforo sozinho e a comunicação entre as placas |
| Integração | Pedestres com carros na mesma placa, e as duas ruas juntas |
| Sistema | Modo noturno e botão de travessia |
| Aceitação | O sistema completo, incluindo placa desligada, fio solto e placa reiniciada |

## Estrutura do repositório

```
.
├── README.md                 este arquivo
├── docs/                     documentação do projeto
│   ├── 01-requisitos.md      o que o sistema precisa fazer
│   ├── 02-arquitetura.md     como o sistema foi organizado
│   ├── 03-projeto-componentes.md   ligações das placas e detalhes de cada parte
│   └── enunciado.md          enunciado da atividade
├── src/                      programa principal de cada placa
│   ├── mestre.c              placa mestre
│   └── escrava.c             placa escrava
├── lib/                      partes usadas pelas duas placas
│   ├── semaforo_veiculo/     semáforo dos carros
│   ├── semaforo_pedestres/   semáforo dos pedestres
│   ├── modo_noturno/         modo noturno e modo alerta
│   └── enlace/               comunicação entre as placas
├── tests/                    testes
│   ├── plano-testes.md       plano com todos os testes
│   ├── TU-01 … TU-03/        testes unitários
│   ├── TI-01, TI-02/         testes de integração
│   ├── TS-01, TS-02/         testes de sistema
│   ├── TA-01 … TA-04/        testes de aceitação
│   └── ferramentas/          programa que grava o que as placas registram
├── CMakeLists.txt, prj.conf, frdm_kl25z.overlay   configuração da compilação e dos pinos
└── LICENSE                   licença
```

Cada pasta de teste tem um `README.md` com o procedimento e o resultado, e os vídeos e registros daquele teste. Os testes unitários, de integração e de sistema também têm um pequeno programa próprio, feito só para aquele teste.

## Onde encontrar mais

- [Requisitos](docs/01-requisitos.md): o que o sistema precisa fazer.
- [Arquitetura](docs/02-arquitetura.md) e [projeto dos componentes](docs/03-projeto-componentes.md): como foi construído e como montar.
- [Enunciado da atividade](docs/enunciado.md).

## Licença

[Apache 2.0](LICENSE).
