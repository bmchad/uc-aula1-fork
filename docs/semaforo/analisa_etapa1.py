"""Analisa o log da etapa 1 do semáforo e gera a linha do tempo.

Uso: python analisa_etapa1.py LOG FIGURA_PNG
O log tem uma linha por evento: "tempo_pc<TAB>uptime ms | evento".
"""
import re
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from matplotlib.lines import Line2D

log, figura = sys.argv[1], sys.argv[2]

ev = []
for linha in open(log, encoding="utf-8"):
    m = re.search(r"\t(\d+) ms \| (.+)$", linha)
    if m:
        ev.append((int(m.group(1)) / 1000.0, m.group(2)))

t_fim = ev[-1][0]

# Estados dos dois semáforos a partir dos eventos
carros, pedestres = [], []   # (inicio, fim, estado)
estado_c = estado_p = None
ini_c = ini_p = None
for t, e in ev:
    if e.startswith("carros ") and not e.startswith("carros travessia"):
        novo = e[len("carros "):]
        if estado_c is not None:
            carros.append((ini_c, t, estado_c))
        estado_c, ini_c = novo, t
    if e.startswith("pedestres "):
        novo = e[len("pedestres "):]
        if estado_p is not None:
            pedestres.append((ini_p, t, estado_p))
        estado_p, ini_p = novo, t
carros.append((ini_c, t_fim, estado_c))
pedestres.append((ini_p, t_fim, estado_p))

# O vermelho dos pedestres pisca enquanto o modo noturno está ativo
t_not_ini = next(t for t, e in ev if e == "modo NOTURNO")
t_not_fim = next(t for t, e in ev if e == "modo NORMAL")

# ---------- Estatísticas ----------
def duracoes(lista, estado):
    # Só fases completas (nem a primeira nem a última, cortadas pela captura)
    return [round((f - i) * 1000) for i, f, s in lista[1:-1] if s == estado]

print("carros VERDE   ms:", duracoes(carros, "VERDE"))
print("carros AMARELO ms:", duracoes(carros, "AMARELO"))
print("carros VERMELHO ms:", duracoes(carros, "VERMELHO"))
print("carros AMARELO PISCANDO ms:", duracoes(carros, "AMARELO PISCANDO"))
print("pedestres VERDE ms:", duracoes(pedestres, "VERDE"))
print("pedestres VERMELHO ms:", duracoes(pedestres, "VERMELHO"))
verdes = [i for i, f, s in carros if s == "VERDE"]
print("periodo VERDE->VERDE ms:", [round((b - a) * 1000) for a, b in zip(verdes, verdes[1:])])

# Segurança: pedestres no verde só com carros no vermelho
violacoes = 0
for pi, pf, ps in pedestres:
    if ps != "VERDE":
        continue
    for ci, cf, cs in carros:
        if cs != "VERMELHO" and ci < pf and cf > pi:
            violacoes += 1
print("violacoes de seguranca:", violacoes)

folgas = []
for i, (t, e) in enumerate(ev[1:], 1):
    if e == "pedestres VERDE" and ev[i - 1][1] == "carros VERMELHO":
        folgas.append(round((t - ev[i - 1][0]) * 1000))
    if e == "carros VERDE" and ev[i - 1][1] == "pedestres VERMELHO":
        folgas.append(round((t - ev[i - 1][0]) * 1000))
print("folgas entre trocas ms: min", min(folgas), "max", max(folgas), "n", len(folgas))

botao = [(t, e.split()[-1]) for t, e in ev if e.startswith("botao noturno")]
print("toques do botao:", len(botao))

# ---------- Figura ----------
VERDE, AMARELO, VERMELHO = "#008300", "#eda100", "#e34948"
SUPERFICIE, TINTA, TINTA2, GRADE = "#fcfcfb", "#0b0b0b", "#52514e", "#e4e3df"
cor = {"VERDE": VERDE, "AMARELO": AMARELO, "VERMELHO": VERMELHO,
       "AMARELO PISCANDO": AMARELO}
nome = {"VERDE": "verde", "AMARELO": "amarelo", "VERMELHO": "vermelho",
        "AMARELO PISCANDO": "pisca"}
tinta_rotulo = {VERDE: "white", VERMELHO: "white", AMARELO: TINTA}

plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 10})
fig, ax = plt.subplots(figsize=(13, 3.9), dpi=150)
fig.patch.set_facecolor(SUPERFICIE)
ax.set_facecolor(SUPERFICIE)

ALTURA, GAP = 0.62, 0.035   # gap = espaço de superfície entre trechos
Y_CARROS, Y_PED, Y_BOTAO = 2.0, 1.0, 0.25


def desenha(y, segmentos, pisca_ini=None, pisca_fim=None):
    for i, f, s in segmentos:
        c = cor[s]
        piscando = s == "AMARELO PISCANDO" or (
            pisca_ini is not None and s == "VERMELHO"
            and abs(i - pisca_ini) < 0.05 and abs(f - pisca_fim) < 0.05)
        ax.barh(y, (f - i) - 2 * GAP, left=i + GAP, height=ALTURA, color=c,
                hatch="////" if piscando else None, edgecolor=SUPERFICIE,
                linewidth=0)
        largura = f - i
        if largura >= 0.8:
            rotulo = "pisca" if piscando else nome[s]
            if largura < 1.5 and not piscando:
                rotulo = {"amarelo": "AM", "vermelho": "VM", "verde": "VD"}[rotulo]
            ax.text(i + largura / 2, y, rotulo, ha="center", va="center",
                    fontsize=8.5, color=tinta_rotulo[c],
                    bbox=dict(boxstyle="round,pad=0.15", fc=c, ec="none") if piscando else None)


desenha(Y_CARROS, carros)
desenha(Y_PED, pedestres, t_not_ini, next(t for t, e in ev
                                          if e == "pedestres VERDE" and t > t_not_ini))

# Toques do botão noturno
for t, pedido in botao:
    ax.plot([t], [Y_BOTAO], marker="v", markersize=8, linestyle="none",
            color=TINTA if pedido == "NOTURNO" else SUPERFICIE,
            markeredgecolor=TINTA, markeredgewidth=1.2)

# Modo noturno efetivo
ax.axvspan(t_not_ini, t_not_fim, color=TINTA2, alpha=0.08, linewidth=0)
ax.annotate("modo noturno: entra no fim\ndo vermelho, sai 1 s depois",
            xy=(t_not_ini, 2.36), xytext=(t_not_ini - 5.6, 2.72),
            fontsize=8.5, color=TINTA2,
            arrowprops=dict(arrowstyle="-", color=TINTA2, lw=0.8))
ax.annotate("6 toques terminando em NORMAL:\nignorados (o pedido é lido no fim do vermelho)",
            xy=(49.0, Y_BOTAO), xytext=(50.0, Y_BOTAO), va="center", fontsize=8.5, color=TINTA2,
            arrowprops=dict(arrowstyle="-", color=TINTA2, lw=0.8))

ax.set_yticks([Y_CARROS, Y_PED, Y_BOTAO])
ax.set_yticklabels(["Carros\n(LED RGB)", "Pedestres\n(PTC8 / PTC9)", "Botão\nnoturno (D9)"],
                   color=TINTA)
ax.set_ylim(-0.05, 3.15)
ax.set_xlim(ev[0][0] - 0.2, t_fim + 0.2)
ax.set_xlabel("tempo desde o reset da placa (s)", color=TINTA2)
ax.tick_params(axis="both", colors=TINTA2, length=0)
ax.xaxis.grid(True, color=GRADE, linewidth=0.8)
ax.set_axisbelow(True)
for lado in ("top", "right", "left"):
    ax.spines[lado].set_visible(False)
ax.spines["bottom"].set_color(GRADE)

ax.set_title("Etapa 1 numa placa: estados dos semáforos registrados pela serial "
             f"({violacoes} instantes com pedestres e carros abertos juntos)",
             loc="left", fontsize=11, color=TINTA, pad=10)

legenda = [Patch(facecolor=VERDE, label="verde"),
           Patch(facecolor=AMARELO, label="amarelo"),
           Patch(facecolor=VERMELHO, label="vermelho"),
           Patch(facecolor="#d9d8d3", hatch="////", edgecolor=SUPERFICIE,
                 label="piscando (noturno)"),
           Line2D([], [], marker="v", linestyle="none", color=TINTA, markersize=7,
                  label="toque: pedido NOTURNO"),
           Line2D([], [], marker="v", linestyle="none", color=SUPERFICIE,
                  markeredgecolor=TINTA, markersize=7, label="toque: pedido NORMAL")]
ax.legend(handles=legenda, loc="upper left", bbox_to_anchor=(0.0, -0.2), ncol=6,
          frameon=False, fontsize=8.5, labelcolor=TINTA2, handlelength=1.4)

fig.tight_layout()
fig.savefig(figura, facecolor=SUPERFICIE, bbox_inches="tight")
print("figura:", figura)
