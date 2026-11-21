#!/usr/bin/env python3
"""Analisa o log do semáforo, de uma ou duas placas, e confere os critérios.

Uso:
    python3 analisa_log.py LOG [--relatorio RELATORIO.md] [--figura FIGURA.png]

O LOG pode ser:
  - o da captura_serial.py: "tempo do PC <TAB> placa <TAB> linha da placa";
  - o da etapa 1: "tempo do PC <TAB> linha da placa" (a placa vira "unica").

O que é medido, e com qual relógio:
  - durações e atrasos dentro de uma placa: tempo da placa (ms);
  - ordem e sobreposição entre placas: tempo do PC (RNF-04).

Com uma placa, valem as faixas da parte A do plano de testes; com duas, as da
parte B. Fases afetadas por travessia, modo noturno, partida ou falha não
entram na conferência dos tempos do ciclo normal, mas aparecem no relatório.

O relatório sai em Markdown. O código de saída é 0 se nenhuma conferência
falhou, 1 se alguma falhou e 2 se o log não pôde ser lido.

A figura precisa do matplotlib (pip install matplotlib); sem ele, só o
relatório é gerado.
"""
import argparse
import re
import sys
from collections import defaultdict
from dataclasses import dataclass

# Critérios do 01-requisitos.md, em segundos -----------------------------------
FAIXAS_UMA_PLACA = {
    ("carros", "VERDE"): (2.97, 3.03),        # RF-03
    ("carros", "AMARELO"): (0.99, 1.01),      # RF-03
    ("carros", "VERMELHO"): (3.96, 4.04),     # RF-03
    ("pedestres", "VERDE"): (3.96, 4.04),     # RF-04
    ("pedestres", "VERMELHO"): (3.96, 4.04),  # RF-04
}
FAIXAS_DUAS_PLACAS = dict(FAIXAS_UMA_PLACA)
FAIXAS_DUAS_PLACAS[("carros", "VERMELHO")] = (4.00, 4.20)    # RF-03, com o handshake
FAIXAS_DUAS_PLACAS[("pedestres", "VERDE")] = (4.00, 4.20)    # RF-04, acompanha o vermelho

MIN_MEDIDAS = 5                  # RF-03, RF-04: pelo menos 5 ciclos
SINCRONISMO_MAX = 0.050          # RF-05: troca do outro semáforo da mesma via
VERDE_ENCURTADO = (1.00, 1.05)   # RF-08: toque no primeiro segundo do verde
TOQUE_ATE_TRAVESSIA_MAX = 2.05   # RF-09
PISCA_MEIO_PERIODO = (0.99, 1.01)  # RF-13, RF-14
PISCA_FASE_MAX = 0.050           # RF-15
ENTRADA_NOTURNO_MAX = 8.4        # RF-16
SAIDA_NOTURNO_MAX_BOTAO = 1.05   # RF-17, na placa do botão
SAIDA_NOTURNO_MAX_OUTRA = 2.1    # RF-17, na outra placa
AVISO_INTERVALO_MAX = 10.1       # RS-05: aviso a cada 10 s (mais 1%)

RE_EVENTO = re.compile(r"^(\d+) ms \| (.+)$")
SESSOES_COM_BOOT = set()   # (placa, sessao) que começam com a linha de boot no log


@dataclass
class Evento:
    n: int        # ordem no arquivo
    tpc: float    # tempo do PC, s
    placa: str
    sessao: int   # aumenta a cada reinício da placa
    t: float      # tempo da placa, s
    tipo: str
    valor: str
    texto: str


@dataclass
class Fase:
    placa: str
    semaforo: str
    estado: str
    ini: Evento
    fim: Evento
    indice: int
    anterior: str

    @property
    def dur(self):
        return self.fim.t - self.ini.t


def classifica(texto):
    """Devolve (tipo, valor) de uma linha de evento da placa."""
    if texto in ("carros VERDE", "carros AMARELO", "carros VERMELHO"):
        return "carros", texto.split()[1]
    if texto == "carros AMARELO PISCANDO":
        return "carros", "PISCA"
    if texto in ("carros AMARELO aceso", "carros AMARELO apagado"):
        return "pisca_carros", texto.split()[2]
    if texto.startswith("carros travessia pedida"):
        return "travessia", ""
    if texto.startswith("carros aguardando") or texto.startswith("carros escrava nao pegou"):
        return "aviso", texto[len("carros "):]
    if texto in ("pedestres VERDE", "pedestres VERMELHO"):
        return "pedestres", texto.split()[1]
    if texto in ("pedestres VERMELHO aceso", "pedestres VERMELHO apagado"):
        return "pisca_pedestres", texto.split()[2]
    if texto.startswith("botao noturno: pedido "):
        return "pedido", texto.split()[-1]
    if texto == "botao travessia: toque":
        return "toque", ""
    if texto in ("modo NOTURNO", "modo NORMAL"):
        return "modo", texto.split()[1]
    if texto.startswith("enlace"):
        return "enlace", texto[len("enlace"):].lstrip(" :")
    return "outro", texto


def le_log(caminho):
    eventos, notas = [], []
    sessao = defaultdict(int)
    ultimo_ms = {}
    with open(caminho, encoding="utf-8", errors="replace") as arquivo:
        for n, linha in enumerate(arquivo):
            linha = linha.rstrip("\n")
            if not linha.strip() or linha.startswith("#"):
                continue
            partes = linha.split("\t")
            try:
                tpc = float(partes[0])
            except ValueError:
                continue
            if len(partes) == 2:
                placa, resto = "unica", partes[1]
            elif len(partes) >= 3:
                placa, resto = partes[1], "\t".join(partes[2:])
            else:
                continue
            if placa == "nota":
                notas.append((tpc, resto))
                continue
            if "Booting Zephyr" in resto:
                sessao[placa] += 1
                ultimo_ms.pop(placa, None)
                SESSOES_COM_BOOT.add((placa, sessao[placa]))
                continue
            m = RE_EVENTO.match(resto)
            if not m:
                continue
            ms = int(m.group(1))
            if placa in ultimo_ms and ms < ultimo_ms[placa]:
                sessao[placa] += 1          # reiniciou sem a linha de boot
                SESSOES_COM_BOOT.add((placa, sessao[placa]))
            ultimo_ms[placa] = ms
            tipo, valor = classifica(m.group(2))
            eventos.append(Evento(n, tpc, placa, sessao[placa], ms / 1000, tipo, valor, m.group(2)))
    return eventos, notas


class Relatorio:
    def __init__(self):
        self.linhas = []
        self.conferencias = []

    def escreve(self, texto=""):
        self.linhas.append(texto)

    def secao(self, titulo):
        self.escreve()
        self.escreve(f"## {titulo}")
        self.escreve()

    def confere(self, requisito, descricao, ok, detalhe=""):
        self.conferencias.append((requisito, descricao, ok, detalhe))
        marca = "OK" if ok else "FALHA"
        self.escreve(f"- **[{marca}] {requisito}:** {descricao}" + (f" ({detalhe})" if detalhe else ""))

    def texto(self, titulo):
        falhas = [c for c in self.conferencias if not c[2]]
        topo = [f"# {titulo}", ""]
        if not self.conferencias:
            topo.append("Nenhuma conferência pôde ser feita com este log.")
        elif falhas:
            topo.append(f"**{len(falhas)} de {len(self.conferencias)} conferências falharam:**")
            topo.append("")
            topo += [f"- {req}: {desc}" + (f" ({det})" if det else "") for req, desc, _, det in falhas]
        else:
            topo.append(f"**Todas as {len(self.conferencias)} conferências passaram.**")
        return "\n".join(topo + self.linhas) + "\n"


def s(x):
    return f"{x:.3f}".replace(".", ",")


def eventos_entre(eventos_pc, t0, t1, tipos, placa=None):
    """Eventos de certos tipos com tempo do PC em [t0, t1], de uma placa ou de todas."""
    return [e for e in eventos_pc if t0 <= e.tpc <= t1 and e.tipo in tipos
            and (placa is None or e.placa == placa)]


def monta_fases(sessoes):
    fases = []
    for (placa, _), evs in sessoes.items():
        for semaforo in ("carros", "pedestres"):
            estados = [e for e in evs if e.tipo == semaforo]
            for i in range(len(estados) - 1):
                fases.append(Fase(placa, semaforo, estados[i].valor, estados[i], estados[i + 1], i,
                                  estados[i - 1].valor if i > 0 else ""))
    return fases


def motivo_exclusao(fase, eventos_pc, carros_por_sessao, sessoes, duas):
    """Por que a fase não entra na conferência do ciclo normal ("" se entra)."""
    carros = carros_por_sessao[(fase.placa, fase.ini.sessao)]
    if (fase.placa, fase.ini.sessao) in SESSOES_COM_BOOT:
        # Desde o boot, as fases até o primeiro verde dos veículos são da partida
        primeiro_verde = next((c for c in carros if c.valor == "VERDE"), None)
        if fase.indice == 0 or primeiro_verde is None or fase.ini.n < primeiro_verde.n:
            return "partida"
    # Sem o boot no log, a primeira fase começa num evento e está completa
    if fase.estado == "PISCA":
        return "pisca"
    t0, t1 = fase.ini.tpc, fase.fim.tpc
    if eventos_entre(eventos_pc, t0, t1, ("travessia",)):
        return "travessia"
    if eventos_entre(eventos_pc, t0, t1, ("aviso",)):
        return "espera com aviso"
    if eventos_entre(eventos_pc, t0, t1, ("enlace",)):
        return "mudança no enlace"
    # Pisca ou mudança de modo no meio da fase. A entrada no modo noturno logo
    # no fim de um vermelho não conta: esse vermelho teve a duração normal.
    for e in sessoes[(fase.placa, fase.ini.sessao)]:
        if fase.ini.n < e.n < fase.fim.n and e.tipo in ("modo", "pisca_carros", "pisca_pedestres"):
            if not (e.tipo == "modo" and e.valor == "NOTURNO" and fase.fim.t - e.t <= 0.1):
                return "modo noturno"
    if not duas:
        return ""   # com uma placa, os vermelhos em volta do pisca têm a duração normal
    # Com duas placas, o vermelho em volta do pisca depende da outra placa: a
    # escrava entra no pisca logo depois de fechar, e a saída espera a outra
    # placa sair dele. O verde dos pedestres acompanha esse vermelho.
    if fase.semaforo == "carros":
        vermelho = fase.ini if fase.estado == "VERMELHO" else None
    else:
        antes = [c for c in carros if c.n <= fase.ini.n]
        vermelho = antes[-1] if antes and antes[-1].valor == "VERMELHO" and fase.estado == "VERDE" else None
    if vermelho is not None:
        idx = carros.index(vermelho)
        if idx > 0 and carros[idx - 1].valor != "AMARELO":
            return "saída do modo noturno"
        if idx + 1 < len(carros) and carros[idx + 1].valor == "PISCA":
            return "entrada no modo noturno"
    return ""


def analisa_tempos(rel, fases, eventos_pc, carros_por_sessao, sessoes, faixas, placas):
    rel.secao("Tempos do ciclo normal (RF-03, RF-04)")
    rel.escreve("Medidos pelo tempo da placa. Fases excluídas não entram na conferência.")
    rel.escreve()
    rel.escreve("| Placa | Semáforo | Fase | Medidas | Mínimo (s) | Máximo (s) | Faixa (s) | Excluídas |")
    rel.escreve("|---|---|---|---|---|---|---|---|")
    resultado = []
    for placa in placas:
        for (semaforo, estado), (lo, hi) in faixas.items():
            todas = [f for f in fases if f.placa == placa and f.semaforo == semaforo and f.estado == estado]
            validas, excluidas = [], defaultdict(int)
            for f in todas:
                motivo = motivo_exclusao(f, eventos_pc, carros_por_sessao, sessoes, len(placas) >= 2)
                if motivo:
                    excluidas[motivo] += 1
                else:
                    validas.append(f)
            durs = [f.dur for f in validas]
            exc = ", ".join(f"{k}: {v}" for k, v in excluidas.items()) or "—"
            if durs:
                rel.escreve(f"| {placa} | {semaforo} | {estado} | {len(durs)} | {s(min(durs))} | "
                            f"{s(max(durs))} | {s(lo)} a {s(hi)} | {exc} |")
            else:
                rel.escreve(f"| {placa} | {semaforo} | {estado} | 0 | — | — | {s(lo)} a {s(hi)} | {exc} |")
            resultado.append((placa, semaforo, estado, validas, lo, hi))
    rel.escreve()
    for placa, semaforo, estado, validas, lo, hi in resultado:
        if not validas:
            continue
        fora = [f for f in validas if not lo <= f.dur <= hi]
        req = "RF-03" if semaforo == "carros" else "RF-04"
        det = "; ".join(f"{s(f.dur)} s em {s(f.ini.t)} s" for f in fora[:5])
        rel.confere(req, f"{placa}: {semaforo} {estado} dentro da faixa", not fora, det)
    poucas = sorted({p for p, _, _, v, _, _ in resultado if len(v) < MIN_MEDIDAS})
    if poucas:
        rel.escreve()
        rel.escreve(f"Atenção: menos de {MIN_MEDIDAS} fases medidas em pelo menos um estado ({', '.join(poucas)}). "
                    f"O critério pede pelo menos {MIN_MEDIDAS} ciclos seguidos.")


def analisa_via(rel, sessoes):
    rel.secao("Sincronismo e segurança dentro de cada via (RF-05, RS-01, RS-03)")
    atrasos, fora_ordem, sobreposicoes = defaultdict(list), defaultdict(list), defaultdict(list)
    for (placa, _), evs in sessoes.items():
        ultimo = {"carros": None, "pedestres": None}
        for e in evs:
            if e.tipo == "carros" and e.valor == "VERDE" and ultimo["pedestres"] is not None:
                p = ultimo["pedestres"]
                atrasos[placa].append((e.t - p.t, e))
                if p.valor != "VERMELHO":
                    fora_ordem[placa].append(e)
            if e.tipo == "pedestres" and e.valor == "VERDE" and ultimo["carros"] is not None:
                c = ultimo["carros"]
                atrasos[placa].append((e.t - c.t, e))
                if c.valor != "VERMELHO":
                    fora_ordem[placa].append(e)
            if e.tipo in ultimo:
                ultimo[e.tipo] = e
                c, p = ultimo["carros"], ultimo["pedestres"]
                if c and p and p.valor == "VERDE" and c.valor in ("VERDE", "AMARELO"):
                    sobreposicoes[placa].append(e)
    for placa in sorted({p for p, _ in sessoes}):
        lista = atrasos[placa]
        if lista:
            pior, ev = max(lista, key=lambda x: x[0])
            rel.confere("RF-05", f"{placa}: troca do outro semáforo da via em até {s(SINCRONISMO_MAX)} s",
                        pior <= SINCRONISMO_MAX, f"{len(lista)} trocas, pior atraso {s(pior)} s em {s(ev.t)} s")
        rel.confere("RS-03", f"{placa}: dentro da via, o semáforo que fecha fecha antes de o outro abrir",
                    not fora_ordem[placa],
                    "; ".join(f"{e.texto} em {s(e.t)} s" for e in fora_ordem[placa][:5]))
        rel.confere("RS-01", f"{placa}: verde dos pedestres nunca junto com os veículos abertos",
                    not sobreposicoes[placa],
                    "; ".join(f"{e.texto} em {s(e.t)} s" for e in sobreposicoes[placa][:5]))


def analisa_entre_vias(rel, eventos_pc, placas):
    rel.secao("Segurança entre as vias (RS-03, RS-04)")
    rel.escreve("Comparado pelo tempo do PC. O pisca do modo noturno não conta como via aberta.")
    rel.escreve()
    aberta = {p: False for p in placas}
    ultimo_fechamento = {}
    sessao = {}
    violacoes, folgas, abriu_junto = [], [], []
    inicio_violacao = None
    for e in eventos_pc:
        if sessao.get(e.placa) != e.sessao:       # placa reiniciou: volta fechada
            sessao[e.placa] = e.sessao
            aberta[e.placa] = False
        if e.tipo != "carros":
            continue
        if e.valor == "VERDE":
            outras = [p for p in placas if p != e.placa]
            if all(not aberta[p] for p in outras):
                fech = [ultimo_fechamento[p] for p in outras if p in ultimo_fechamento]
                if fech:
                    folgas.append((e.tpc - max(fech), e))
            else:
                abriu_junto.append(e)
        aberta[e.placa] = e.valor in ("VERDE", "AMARELO")
        if e.valor not in ("VERDE", "AMARELO"):
            ultimo_fechamento[e.placa] = e.tpc
        juntas = sum(aberta.values()) >= 2
        if juntas and inicio_violacao is None:
            inicio_violacao = e
        elif not juntas and inicio_violacao is not None:
            violacoes.append((inicio_violacao, e))
            inicio_violacao = None
    if inicio_violacao is not None:
        violacoes.append((inicio_violacao, None))
    det = "; ".join(f"de {s(a.tpc)} s a {s(b.tpc) + ' s' if b else 'fim do log'}" for a, b in violacoes[:5])
    rel.confere("RS-04", "as duas vias nunca abertas ao mesmo tempo", not violacoes, det)
    if folgas or abriu_junto:
        det = []
        if abriu_junto:
            det.append("abriu com a outra aberta: "
                       + "; ".join(f"{e.placa} em {s(e.tpc)} s" for e in abriu_junto[:5]))
        if folgas:
            menor, ev = min(folgas, key=lambda x: x[0])
            det.append(f"{len(folgas)} aberturas normais, menor folga {s(menor)} s "
                       f"({ev.placa} em {s(ev.tpc)} s do PC)")
        rel.confere("RS-03", "entre as vias, uma só abre depois de a outra fechar",
                    not abriu_junto and all(f >= 0 for f, _ in folgas), "; ".join(det))


def analisa_travessia(rel, sessoes):
    rel.secao("Botão de travessia (RF-07 a RF-11)")
    houve = False
    for (placa, _), evs in sessoes.items():
        carros = [e for e in evs if e.tipo == "carros"]
        peds = [e for e in evs if e.tipo == "pedestres"]
        toques = [e for e in evs if e.tipo == "toque"]
        for tr in (e for e in evs if e.tipo == "travessia"):
            houve = True
            verde = [c for c in carros if c.n < tr.n and c.valor == "VERDE"][-1]
            depois = [c for c in carros if c.n > tr.n]
            if len(depois) < 2 or depois[0].valor != "AMARELO" or depois[1].valor != "VERMELHO":
                rel.confere("RF-07", f"{placa}: travessia em {s(tr.t)} s passa pelo amarelo e fecha", False,
                            "sequência incompleta no log")
                continue
            amarelo, vermelho = depois[0], depois[1]
            dur_verde = amarelo.t - verde.t
            dur_amarelo = vermelho.t - amarelo.t
            ped_verde = next((p for p in peds if p.n > vermelho.n and p.valor == "VERDE"), None)
            no_verde = [t for t in toques if verde.t <= t.t <= tr.t]
            toque = no_verde[0] if no_verde else None
            ref = toque.t if toque else verde.t     # sem o toque no log, o início do verde é o pior caso
            rel.escreve(f"- {placa}, verde de {s(verde.t)} s: {len(no_verde)} toque(s) registrado(s), "
                        f"verde de {s(dur_verde)} s, amarelo de {s(dur_amarelo)} s")
            ok7 = 0.99 <= dur_amarelo <= 1.01 and ped_verde is not None
            rel.confere("RF-07", f"{placa}: depois do toque, amarelo de 0,990 a 1,010 s e verde dos pedestres",
                        ok7, f"amarelo {s(dur_amarelo)} s")
            if toque is None or toque.t - verde.t < 1.0:
                lo, hi = VERDE_ENCURTADO
                rel.confere("RF-08", f"{placa}: toque no primeiro segundo, verde de {s(lo)} a {s(hi)} s",
                            lo <= dur_verde <= hi, f"verde {s(dur_verde)} s")
            if ped_verde is not None:
                atraso = ped_verde.t - ref
                rel.confere("RF-09", f"{placa}: do toque ao verde dos pedestres em até {s(TOQUE_ATE_TRAVESSIA_MAX)} s",
                            atraso <= TOQUE_ATE_TRAVESSIA_MAX,
                            f"{s(atraso)} s" + ("" if toque else ", medido do início do verde"))
        # Toques que não deveriam ter efeito: fora do verde
        sem_efeito = defaultdict(int)
        for t in toques:
            antes = [c for c in carros if c.n < t.n]
            estado = antes[-1].valor if antes else "partida"
            if estado != "VERDE":
                sem_efeito[estado] += 1
        if sem_efeito:
            houve = True
            rel.escreve(f"- {placa}: toques fora do verde (RF-10, RF-18; não devem encurtar nada): "
                        + ", ".join(f"{k}: {v}" for k, v in sem_efeito.items()))
    if not houve:
        rel.escreve("Nenhum toque de travessia no log.")


def analisa_noturno(rel, sessoes, eventos_pc):
    rel.secao("Modo noturno (RF-13 a RF-17)")
    houve = False
    # Pisca: meio período e fase entre veículos e pedestres da mesma via
    for (placa, _), evs in sessoes.items():
        pc = [e for e in evs if e.tipo == "pisca_carros"]
        pp = [e for e in evs if e.tipo == "pisca_pedestres"]
        if not pc and not pp:
            continue
        houve = True
        for nome, lista, req in (("amarelo dos veículos", pc, "RF-13"), ("vermelho dos pedestres", pp, "RF-14")):
            meios = []
            for a, b in zip(lista, lista[1:]):
                entre = [e for e in evs if a.n < e.n < b.n and e.tipo in ("carros", "modo")]
                if not entre:                      # mesma sequência de pisca
                    meios.append(b.t - a.t)
            if meios:
                lo, hi = PISCA_MEIO_PERIODO
                fora = [m for m in meios if not lo <= m <= hi]
                rel.confere(req, f"{placa}: meio período do pisca do {nome} de {s(lo)} a {s(hi)} s", not fora,
                            f"{len(meios)} meios períodos, de {s(min(meios))} a {s(max(meios))} s")
        # Durante o pisca, nada de verde nem de vermelho dos veículos, nem verde dos pedestres
        proibidos = []
        modo = "NORMAL"
        for e in evs:
            if e.tipo == "modo":
                modo = e.valor
            elif modo == "NOTURNO" and ((e.tipo == "carros" and e.valor in ("VERDE", "VERMELHO", "AMARELO"))
                                        or (e.tipo == "pedestres" and e.valor == "VERDE")):
                proibidos.append(e)
        rel.confere("RF-13", f"{placa}: durante o pisca, nenhuma outra luz acende", not proibidos,
                    "; ".join(f"{e.texto} em {s(e.t)} s" for e in proibidos[:5]))
        # Fase: cada troca do amarelo com a troca igual do vermelho dos pedestres mais próxima
        refs = {"aceso": [e for e in evs if (e.tipo == "pisca_pedestres" and e.valor == "aceso")
                          or (e.tipo == "pedestres" and e.valor == "VERMELHO")],
                "apagado": [e for e in pp if e.valor == "apagado"]}
        difs, sem_par = [], 0
        for e in pc:
            perto = [abs(e.t - r.t) for r in refs[e.valor] if abs(e.t - r.t) < 0.5]
            if perto:
                difs.append(min(perto))
            else:
                sem_par += 1
        if difs:
            rel.confere("RF-15", f"{placa}: veículos e pedestres piscam em fase (até {s(PISCA_FASE_MAX)} s)",
                        max(difs) <= PISCA_FASE_MAX,
                        f"{len(difs)} trocas, maior diferença {s(max(difs))} s"
                        + (f", {sem_par} sem par (fim do pisca)" if sem_par else ""))
    # Entrada e saída, a partir do pedido na placa do botão
    pedidos = [e for e in eventos_pc if e.tipo == "pedido"]
    if pedidos:
        houve = True
    botoes = sorted({e.placa for e in pedidos})
    for botao in botoes:
        modos = [e for e in eventos_pc if e.tipo == "modo" and e.placa == botao]
        for m in modos:
            antes = [p for p in pedidos if p.placa == botao and p.n < m.n and p.sessao == m.sessao]
            if not antes or antes[-1].valor != m.valor:
                continue
            pedido = antes[-1]
            if m.valor == "NOTURNO":
                atraso = m.t - pedido.t
                rel.confere("RF-16", f"{botao}: entra no pisca em até {s(ENTRADA_NOTURNO_MAX)} s do pedido",
                            atraso <= ENTRADA_NOTURNO_MAX, f"{s(atraso)} s, pedido em {s(pedido.t)} s")
                for outra in sorted({e.placa for e in eventos_pc} - {botao}):
                    prox = next((e for e in eventos_pc if e.placa == outra and e.tpc >= pedido.tpc
                                 and e.tipo == "modo" and e.valor == "NOTURNO"), None)
                    if prox:
                        atraso = prox.tpc - pedido.tpc
                        rel.confere("RF-16", f"{outra}: entra no pisca em até {s(ENTRADA_NOTURNO_MAX)} s do pedido",
                                    atraso <= ENTRADA_NOTURNO_MAX, f"{s(atraso)} s pelo tempo do PC")
            else:
                for placa in sorted({e.placa for e in eventos_pc}):
                    prox = next((e for e in eventos_pc if e.placa == placa and e.tpc >= pedido.tpc
                                 and e.tipo == "carros"), None)
                    if prox is None:
                        continue
                    if placa == botao:
                        atraso, limite = prox.t - pedido.t, SAIDA_NOTURNO_MAX_BOTAO
                    else:
                        atraso, limite = prox.tpc - pedido.tpc, SAIDA_NOTURNO_MAX_OUTRA
                    rel.confere("RF-17", f"{placa}: sai do pisca para o vermelho em até {s(limite)} s do pedido",
                                prox.valor == "VERMELHO" and atraso <= limite,
                                f"{prox.texto} depois de {s(atraso)} s")
    if not houve:
        rel.escreve("Nenhum modo noturno no log.")


def analisa_avisos(rel, sessoes):
    rel.secao("Enlace e avisos de espera (RS-05, RS-06)")
    houve = False
    for (placa, _), evs in sessoes.items():
        for e in (e for e in evs if e.tipo == "enlace"):
            houve = True
            rel.escreve(f"- {placa}: enlace {e.valor} em {s(e.t)} s (tempo da placa), {s(e.tpc)} s (PC)")
        avisos = [e for e in evs if e.tipo == "aviso"]
        if not avisos:
            continue
        houve = True
        por_texto = defaultdict(list)
        for a in avisos:
            por_texto[a.valor].append(a)
        for texto, lista in por_texto.items():
            gaps = [b.t - a.t for a, b in zip(lista, lista[1:])]
            rel.escreve(f"- {placa}: \"{texto}\" {len(lista)} vez(es), de {s(lista[0].t)} a {s(lista[-1].t)} s")
            seguidos = [g for g in gaps if g < 3 * AVISO_INTERVALO_MAX]   # mesma espera
            if seguidos and texto != "escrava nao pegou a vez: esperando o ciclo dela":
                rel.confere("RS-05", f"{placa}: aviso \"{texto}\" pelo menos a cada {s(AVISO_INTERVALO_MAX)} s",
                            max(seguidos) <= AVISO_INTERVALO_MAX, f"maior intervalo {s(max(seguidos))} s")
    if not houve:
        rel.escreve("Nenhum evento do enlace nem aviso de espera no log.")


def desenha(caminho, eventos_pc, placas):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("Sem o matplotlib: a figura não foi gerada (pip install matplotlib).", file=sys.stderr)
        return
    cores = {"VERDE": "#2e7d32", "AMARELO": "#f9a825", "VERMELHO": "#c62828", "PISCA": "#ffe082"}
    linhas = [(p, sem) for p in placas for sem in ("carros", "pedestres")]
    fig, ax = plt.subplots(figsize=(14, 1 + 0.6 * len(linhas)))
    t_fim = eventos_pc[-1].tpc
    for y, (placa, sem) in enumerate(reversed(linhas)):
        estados = [e for e in eventos_pc if e.placa == placa and e.tipo == sem]
        for a, b in zip(estados, estados[1:] + [None]):
            fim = b.tpc if b else t_fim
            ax.broken_barh([(a.tpc, fim - a.tpc)], (y - 0.35, 0.7), facecolors=cores.get(a.valor, "#9e9e9e"))
    for e in eventos_pc:
        if e.tipo == "toque":
            y = len(linhas) - 1 - linhas.index((e.placa, "carros"))
            ax.plot(e.tpc, y + 0.45, "kv", markersize=5)
    ax.set_yticks(range(len(linhas)))
    ax.set_yticklabels([f"{p}: {sem}" for p, sem in reversed(linhas)])
    ax.set_xlabel("tempo do PC (s)")
    ax.set_title("Linha do tempo dos semáforos (triângulos: toques de travessia)")
    fig.tight_layout()
    fig.savefig(caminho, dpi=120)
    print(f"Figura salva em {caminho}", file=sys.stderr)


def main():
    parser = argparse.ArgumentParser(description="Analisa o log do semáforo e confere os critérios.")
    parser.add_argument("log")
    parser.add_argument("--relatorio", help="salva o relatório Markdown neste arquivo")
    parser.add_argument("--figura", help="salva a linha do tempo neste PNG (precisa do matplotlib)")
    args = parser.parse_args()

    try:
        eventos, notas = le_log(args.log)
    except OSError as erro:
        print(f"Não consegui ler o log: {erro}", file=sys.stderr)
        return 2
    if not eventos:
        print("O log não tem nenhuma linha de evento no formato 'N ms | evento'.", file=sys.stderr)
        return 2

    eventos_pc = sorted(eventos, key=lambda e: (e.tpc, e.n))
    placas = sorted({e.placa for e in eventos})
    sessoes = defaultdict(list)
    for e in eventos:
        sessoes[(e.placa, e.sessao)].append(e)
    carros_por_sessao = {k: [e for e in v if e.tipo == "carros"] for k, v in sessoes.items()}
    duas = len(placas) >= 2
    faixas = FAIXAS_DUAS_PLACAS if duas else FAIXAS_UMA_PLACA

    rel = Relatorio()
    rel.secao("Resumo")
    rel.escreve(f"- Log: `{args.log}`")
    rel.escreve(f"- Placas: {', '.join(placas)} ({'parte B, duas placas' if duas else 'parte A, uma placa'})")
    for placa in placas:
        n_sessoes = len({e.sessao for e in eventos if e.placa == placa})
        rel.escreve(f"- {placa}: {sum(e.placa == placa for e in eventos)} eventos, {n_sessoes} sessão(ões) "
                    "(cada reinício abre uma sessão)")
    rel.escreve(f"- Duração pelo tempo do PC: {s(eventos_pc[-1].tpc - eventos_pc[0].tpc)} s")

    analisa_tempos(rel, monta_fases(sessoes), eventos_pc, carros_por_sessao, sessoes, faixas, placas)
    analisa_via(rel, sessoes)
    if duas:
        analisa_entre_vias(rel, eventos_pc, placas)
    analisa_travessia(rel, sessoes)
    analisa_noturno(rel, sessoes, eventos_pc)
    analisa_avisos(rel, sessoes)
    if notas:
        rel.secao("Notas do operador")
        for tpc, texto in notas:
            rel.escreve(f"- {s(tpc)} s: {texto}")

    saida = rel.texto(f"Análise de {args.log}")
    print(saida)
    if args.relatorio:
        with open(args.relatorio, "w", encoding="utf-8") as arquivo:
            arquivo.write(saida)
    if args.figura:
        desenha(args.figura, eventos_pc, placas)
    return 0 if all(ok for _, _, ok, _ in rel.conferencias) else 1


if __name__ == "__main__":
    sys.exit(main())
