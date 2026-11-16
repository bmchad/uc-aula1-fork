/*
 * Acelerômetro MMA8451Q (FRDM-KL25Z) -> integração -> serial em CSV.
 *
 * A cada 50 ms (115200 bps) a placa envia uma linha com 9 campos:
 *   ax,ay,az,vx,vy,vz,px,py,pz
 *   ax..az em m/s² | vx..vz em m/s | px..pz em m
 *
 * Calibração: a placa precisa ficar parada e na mesma orientação durante
 * CALIBRACAO_MS. A média dessas leituras é subtraída de todas as leituras
 * seguintes. Assim ax..az são aceleração dinâmica (zero em repouso) e a
 * integração não acumula a gravidade (9,8 m/s²) como se fosse movimento.
 *
 * Limitação: integrar aceleração duas vezes faz qualquer erro de offset
 * crescer com o tempo. A posição vai derivar em poucos segundos.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#define PERIODO_MS        50
#define DT_S              (PERIODO_MS / 1000.0f)
#define CALIBRACAO_MS     2000
#define AMOSTRAS_CAL      (CALIBRACAO_MS / PERIODO_MS)

/* 1: remove a média do repouso (gravidade e offset). 0: envia valores crus */
#define REMOVER_GRAVIDADE 1

static const struct device *const acel = DEVICE_DT_GET(DT_NODELABEL(mma8451q));

static float offset[3];
static float vel[3];
static float pos[3];

static int ler_aceleracao(float a[3])
{
	struct sensor_value val[3];
	int ret;

	ret = sensor_sample_fetch(acel);
	if (ret < 0) {
		return ret;
	}

	ret = sensor_channel_get(acel, SENSOR_CHAN_ACCEL_XYZ, val);
	if (ret < 0) {
		return ret;
	}

	for (int i = 0; i < 3; i++) {
		a[i] = sensor_value_to_float(&val[i]);
	}

	return 0;
}

/* Imprime com 3 casas decimais sem usar %f (printk não tem ponto flutuante) */
static void imprime_valor(float v)
{
	int32_t mil = (int32_t)(v * 1000.0f + (v >= 0.0f ? 0.5f : -0.5f));
	uint32_t abs_mil = (mil < 0) ? (uint32_t)(-mil) : (uint32_t)mil;

	printk("%s%u.%03u", (mil < 0) ? "-" : "", abs_mil / 1000, abs_mil % 1000);
}

static void imprime_linha(const float v[9])
{
	for (int i = 0; i < 9; i++) {
		imprime_valor(v[i]);
		if (i < 8) {
			printk(",");
		} else {
			printk("\n");
		}
	}
}

static int calibra(void)
{
	float soma[3] = {0};
	float a[3];
	int64_t proximo = k_uptime_get();

	printk("Calibrando: mantenha a placa parada por %d ms...\n", CALIBRACAO_MS);

	for (int n = 0; n < AMOSTRAS_CAL; n++) {
		if (ler_aceleracao(a) < 0) {
			printk("Erro na calibracao\n");
			return -1;
		}
		for (int i = 0; i < 3; i++) {
			soma[i] += a[i];
		}
		proximo += PERIODO_MS;
		k_sleep(K_TIMEOUT_ABS_MS(proximo));
	}

	for (int i = 0; i < 3; i++) {
		offset[i] = REMOVER_GRAVIDADE ? soma[i] / AMOSTRAS_CAL : 0.0f;
	}

	printk("Calibracao concluida\n");
	return 0;
}

int main(void)
{
	if (!device_is_ready(acel)) {
		printk("Erro: acelerometro MMA8451Q nao esta pronto\n");
		return -1;
	}

	if (calibra() < 0) {
		return -1;
	}

	/* Cabeçalho: o leitor_serial.py grava a primeira linha de 9 campos */
	printk("ax,ay,az,vx,vy,vz,px,py,pz\n");

	int64_t proximo = k_uptime_get();

	while (1) {
		float a[3];
		float linha[9];

		if (ler_aceleracao(a) < 0) {
			/* Linha sem vírgulas: o leitor_serial.py descarta */
			printk("# erro de leitura\n");
		} else {
			for (int i = 0; i < 3; i++) {
				a[i] -= offset[i];
				vel[i] += a[i] * DT_S;
				pos[i] += vel[i] * DT_S;

				linha[i] = a[i];
				linha[i + 3] = vel[i];
				linha[i + 6] = pos[i];
			}
			imprime_linha(linha);
		}

		proximo += PERIODO_MS;
		k_sleep(K_TIMEOUT_ABS_MS(proximo));
	}

	return 0;
}
