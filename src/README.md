# Esboço: semáforos de pedestres e veículos (Aula 07)

## Objetivo

Esboçar a solução do sistema de semáforos da Aula 07 (enunciado em [../README.md](../README.md)) antes de implementá-la. O esboço mostra:

- como o sistema se divide entre as duas placas;
- quais threads existem em cada placa e o que cada uma faz;
- onde entram o mutex, os semáforos do Zephyr (`k_sem`) e os sinais entre as placas.

> Neste documento, "semáforo" sozinho é o semáforo de trânsito. A primitiva do Zephyr aparece sempre como `k_sem`.

## Visão geral

O enunciado põe cada semáforo em uma placa. `k_mutex` e `k_sem` só funcionam dentro de uma placa, então o sincronismo entre as duas depende de fios entre elas.

```
         Placa A (pedestres)                        Placa B (veículos)
  +------------------------------+          +------------------------------+
  | LED RGB: verde e vermelho    |          | LEDs externos: verde,        |
  | Botão de travessia (sw0)     |          | amarelo e vermelho           |
  |                              |          | Chave do modo noturno (sw1)  |
  |                       PEDIDO |--------->| PEDIDO                       |
  |               CARROS_PARADOS |<---------| CARROS_PARADOS               |
  |                      NOTURNO |<---------| NOTURNO                      |
  |                          GND |----------| GND                          |
  +------------------------------+          +------------------------------+
```

| Sinal | Direção | Significado |
|---|---|---|
| `CARROS_PARADOS` | B → A | Fica em 1 enquanto o vermelho dos veículos está aceso. Só ele libera o verde dos pedestres. |
| `NOTURNO` | B → A | Fica em 1 enquanto a placa B está no modo noturno. |
| `PEDIDO` | A → B | Pulso enviado quando o botão de travessia é apertado. |

A placa B comanda o ciclo e a placa A obedece. Quem decide é o lado que pode causar um atropelamento.

As entradas da placa A usam pull-down. Se um fio soltar, `CARROS_PARADOS` lê 0 e os pedestres ficam no vermelho, que é a falha segura.

## Ciclo normal (8 s)

| Tempo | Veículos | Pedestres | `CARROS_PARADOS` |
|---|---|---|---|
| 0 a 3 s | verde | vermelho | 0 |
| 3 a 4 s | amarelo | vermelho | 0 |
| 4 a 8 s | vermelho | verde | 1 |

Os dois ciclos têm 8 s. O verde mais o amarelo dos veículos (3 + 1 s) coincide com o vermelho dos pedestres (4 s), e o vermelho dos veículos (4 s) coincide com o verde dos pedestres (4 s). Por isso a placa A não conta o próprio ciclo, só segue `CARROS_PARADOS`.

## Threads

Cada LED tem uma thread, como o enunciado pede. A ordem entre elas é uma cadeia de `k_sem`: cada thread espera a sua vez e, ao terminar, passa a vez para a próxima. Os botões e a chave são tratados por interrupção (ISR). A ISR não pode usar mutex, então ela só chama `k_sem_give` e acorda uma thread.

### Placa B (veículos)

| Thread | O que faz | Espera | Ao terminar |
|---|---|---|---|
| `t_carro_verde` | Acende o verde por 3 s. Um pedido de travessia encerra o verde mais cedo, respeitando 1 s mínimo. | `vez_verde` | dá `vez_amarelo` |
| `t_carro_amarelo` | No modo normal, acende o amarelo por 1 s. No noturno, pisca 1 s aceso e 1 s apagado e liga a saída `NOTURNO`. | `vez_amarelo` | dá `vez_vermelho` |
| `t_carro_vermelho` | Acende o vermelho por 4 s e mantém `CARROS_PARADOS = 1` nesse período. | `vez_vermelho` | dá `vez_verde`, ou `vez_amarelo` se o modo for noturno |
| `t_modo` | Trata a chave sw1 com debounce e alterna a variável `modo`. | `chave_modo`, dado pela ISR da sw1 | (nada) |

ISR da entrada `PEDIDO`: chama `k_sem_give(&pedido)`. Quem consome é o verde, ou o amarelo no modo noturno.

### Placa A (pedestres)

| Thread | O que faz | Espera | Ao terminar |
|---|---|---|---|
| `t_ped_vermelho` | Acende o vermelho até `CARROS_PARADOS` subir. Com `NOTURNO = 1`, pisca 1 s aceso e 1 s apagado. | `vez_ped_vermelho` | dá `vez_ped_verde` |
| `t_ped_verde` | Acende o verde até `CARROS_PARADOS` descer, por no máximo 4 s. | `vez_ped_verde` | dá `vez_ped_vermelho` |
| `t_botao` | Trata o botão sw0 com debounce e manda um pulso em `PEDIDO`. | `botao`, dado pela ISR da sw0 | (nada) |

ISR da entrada `CARROS_PARADOS`, nas duas bordas: na subida dá `carros_pararam`, na descida dá `carros_andam`.

`t_modo` e `t_botao` têm prioridade maior que as threads dos LEDs (número menor no Zephyr), porque são curtas e precisam responder rápido.

## Onde entra cada primitiva

| Primitiva | Placa | Para quê |
|---|---|---|
| `mutex_carros` | B | Envolve acender e apagar qualquer LED dos veículos. |
| `mutex_modo` | B | Protege a variável `modo`, escrita por `t_modo` e lida pelo vermelho e pelo amarelo. |
| `mutex_ped` | A | Envolve acender e apagar qualquer LED dos pedestres. |
| `vez_*` (`k_sem`) | A e B | Ordem entre as threads dos LEDs. |
| `pedido`, `chave_modo`, `botao`, `carros_pararam`, `carros_andam` (`k_sem`) | A e B | Avisam uma thread de que uma ISR viu um evento. |

Com a cadeia de `k_sem`, só uma thread tem a vez por vez. No funcionamento normal, então, não é o mutex que evita dois LEDs acesos. O mutex é exigido pelo enunciado e funciona como garantia: se uma mudança quebrar a cadeia, por exemplo com dois `k_sem_give` seguidos, o mutex ainda impede o conflito. Um teste unitário pode provar isso forçando essa falha.

Nas threads da placa A, a thread segura o mutex enquanto espera um `k_sem`. Isso não trava o sistema porque quem dá esse `k_sem` é uma ISR, que nunca pede o mutex.

## Modos de operação

**Normal.** É o ciclo de 8 s da tabela acima.

**Travessia (botão sw0, na placa A).** O pedido chega à placa B. Se os carros estão no verde, o verde termina depois do mínimo de 1 s. Em seguida vêm amarelo (1 s) e vermelho (4 s), com o verde dos pedestres. O amarelo sempre passa, então nunca há salto de verde para vermelho. Um pedido feito durante o amarelo ou o vermelho é descartado, porque a travessia já está a caminho.

**Noturno (chave sw1, na placa B).**
- **Entrada:** ao terminar, o vermelho dos veículos passa a vez ao amarelo em vez do verde. O amarelo pisca e liga `NOTURNO`, e a placa A pisca o vermelho. A entrada só acontece nesse ponto seguro, então pode demorar até um ciclo (8 s).
- **Saída:** o amarelo fica 1 s aceso fixo, desliga `NOTURNO` e passa a vez ao vermelho.
- **Botão no noturno (proposta):** o amarelo fica 1 s fixo e passa a vez ao vermelho. A travessia acontece, e o ciclo volta a piscar.

## Esboço de código

Este código é um esboço e não foi compilado. Ele omite a configuração dos GPIOs, das interrupções e o `K_THREAD_DEFINE`.

### Placa B (veículos)

```c
K_MUTEX_DEFINE(mutex_carros);
K_SEM_DEFINE(vez_verde,    1, 1);   /* o ciclo começa no verde */
K_SEM_DEFINE(vez_amarelo,  0, 1);
K_SEM_DEFINE(vez_vermelho, 0, 1);
K_SEM_DEFINE(pedido,       0, 1);   /* dado pela ISR de PEDIDO */

#define VERDE_MS     3000
#define VERDE_MIN_MS 1000
#define VERMELHO_MS  4000

void t_carro_verde(void *a, void *b, void *c)
{
	while (1) {
		k_sem_take(&vez_verde, K_FOREVER);
		k_sem_reset(&pedido);                   /* descarta pedido antigo */
		k_mutex_lock(&mutex_carros, K_FOREVER);
		gpio_pin_set_dt(&led_verde, 1);
		k_msleep(VERDE_MIN_MS);
		/* volta antes do tempo se chegar um pedido de travessia */
		k_sem_take(&pedido, K_MSEC(VERDE_MS - VERDE_MIN_MS));
		gpio_pin_set_dt(&led_verde, 0);
		k_mutex_unlock(&mutex_carros);
		k_sem_give(&vez_amarelo);
	}
}

void t_carro_vermelho(void *a, void *b, void *c)
{
	while (1) {
		k_sem_take(&vez_vermelho, K_FOREVER);
		k_mutex_lock(&mutex_carros, K_FOREVER);
		gpio_pin_set_dt(&led_vermelho, 1);
		gpio_pin_set_dt(&saida_carros_parados, 1);  /* libera os pedestres */
		k_msleep(VERMELHO_MS);
		gpio_pin_set_dt(&saida_carros_parados, 0);  /* antes de apagar o vermelho */
		gpio_pin_set_dt(&led_vermelho, 0);
		k_mutex_unlock(&mutex_carros);
		k_sem_give(modo_noturno() ? &vez_amarelo : &vez_verde);
	}
}

void isr_pedido(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_sem_give(&pedido);   /* ISR: nada de mutex */
}
```

### Placa A (pedestres)

```c
K_MUTEX_DEFINE(mutex_ped);
K_SEM_DEFINE(vez_ped_vermelho, 1, 1);   /* começa no vermelho */
K_SEM_DEFINE(vez_ped_verde,    0, 1);
K_SEM_DEFINE(carros_pararam,   0, 1);
K_SEM_DEFINE(carros_andam,     0, 1);

void t_ped_vermelho(void *a, void *b, void *c)
{
	while (1) {
		k_sem_take(&vez_ped_vermelho, K_FOREVER);
		k_mutex_lock(&mutex_ped, K_FOREVER);
		gpio_pin_set_dt(&led_vermelho, 1);
		/* espera os carros pararem; no modo noturno, pisca enquanto espera */
		while (k_sem_take(&carros_pararam, K_SECONDS(1)) != 0) {
			if (gpio_pin_get_dt(&entrada_noturno)) {
				gpio_pin_toggle_dt(&led_vermelho);
			} else {
				gpio_pin_set_dt(&led_vermelho, 1);
			}
		}
		gpio_pin_set_dt(&led_vermelho, 0);
		k_mutex_unlock(&mutex_ped);
		k_sem_give(&vez_ped_verde);
	}
}

void t_ped_verde(void *a, void *b, void *c)
{
	while (1) {
		k_sem_take(&vez_ped_verde, K_FOREVER);
		k_sem_reset(&carros_andam);   /* descarta descida de um ciclo anterior */
		k_mutex_lock(&mutex_ped, K_FOREVER);
		gpio_pin_set_dt(&led_verde, 1);
		k_sem_take(&carros_andam, K_MSEC(4000));   /* apaga na descida ou em 4 s */
		gpio_pin_set_dt(&led_verde, 0);
		k_mutex_unlock(&mutex_ped);
		k_sem_give(&vez_ped_vermelho);
	}
}

void isr_carros_parados(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (gpio_pin_get_dt(&entrada_carros_parados)) {
		k_sem_give(&carros_pararam);
	} else {
		k_sem_give(&carros_andam);
	}
}
```

O `k_sem_reset(&carros_andam)` é necessário. Sem ele, quando o verde termina pelos 4 s antes da descida do sinal, essa descida fica guardada no `k_sem`. No ciclo seguinte, o verde apagaria assim que acendesse.

## Pontos de teste que o esboço cria

- **Unitário:**
  - Duração de cada LED.
  - Nunca dois LEDs acesos no mesmo semáforo, inclusive forçando a quebra da cadeia de `k_sem`.
  - Debounce dos botões.
- **Integração:**
  - O verde dos pedestres só acende com `CARROS_PARADOS = 1`.
  - Medir o atraso entre `CARROS_PARADOS` cair e o verde dos pedestres apagar.
  - Ligar as placas em ordens diferentes.
  - Soltar o fio de `CARROS_PARADOS` no meio do ciclo.
- **Sistema:**
  - Botão em cada fase (verde, amarelo, vermelho e noturno).
  - Entrada e saída do modo noturno em cada fase, com a latência máxima.
  - Botão apertado várias vezes seguidas.

## Pontos em aberto (confirmar com o professor)

1. **Comunicação entre as placas:** o enunciado não diz como as placas conversam. Este esboço usa 3 fios de GPIO e GND. A outra opção é UART, mas a UART0 já é o console.
2. **Comportamento do botão:** este esboço faz o botão encurtar o verde dos carros. A outra leitura é "carros sempre no verde até alguém apertar".
3. **Ativação do modo noturno:** o enunciado não diz como se entra no modo noturno. Este esboço usa a chave sw1 na placa B.
4. **LED amarelo:** a FRDM-KL25Z não tem LED amarelo, então a placa B precisa de 3 LEDs externos, declarados num overlay de devicetree. Misturar o vermelho e o verde do RGB dá amarelo, mas acende dois LEDs ao mesmo tempo, o que contradiz a exclusão mútua.
5. **Botões:** o devicetree declara `sw0` (PTA16) e `sw1` (PTA17) ativos em nível baixo. Conferir se a placa tem esses botões soldados. Se não tiver, é preciso ligar botões externos entre esses pinos e o GND.
6. **Pinos dos fios e dos LEDs externos:** ainda faltam escolher. Os pinos não podem conflitar com a I2C0 (acelerômetro), a UART0 (console) e os LEDs da placa.
7. **Intervalo todos-vermelhos:** o enunciado não pede um intervalo com todos os sinais no vermelho. Sem ele, a troca entre o verde dos pedestres e o verde dos carros depende só da latência entre as placas, de poucos milissegundos. Vale medir no teste de integração.
