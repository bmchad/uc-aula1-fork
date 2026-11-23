#!/usr/bin/env python3
"""Grava a serial de uma ou duas placas com o tempo do PC.

Uso:
    python3 captura_serial.py SAIDA.log mestre=/dev/ttyACM0 escrava=/dev/ttyACM1
    python3 captura_serial.py SAIDA.log unica=COM5

Cada placa é dada como NOME=PORTA. O arquivo de saída tem uma linha por linha
recebida:

    tempo do PC (s) <TAB> nome da placa <TAB> linha da placa

O tempo do PC conta desde o início da captura e é o mesmo relógio para as duas
placas: é ele que decide a ordem dos eventos entre as placas (RNF-04).

Enquanto a captura roda, cada linha digitada no terminal vira uma nota no log,
com o nome "nota" (por exemplo: "tirei o fio VEZ"). Ctrl+C encerra.

Se uma porta cair (USB desligado, placa sem energia), a captura registra isso
e tenta reabrir a porta a cada 0,5 s, então dá para desligar e religar uma
placa no meio do teste.
"""
import argparse
import datetime
import sys
import threading
import time

try:
    import serial
except ImportError:
    sys.exit("Falta o pyserial: pip install pyserial")


class Saida:
    """Arquivo de log compartilhado pelas threads, com o relógio da captura."""

    def __init__(self, caminho):
        self.arquivo = open(caminho, "w", encoding="utf-8")
        self.trava = threading.Lock()
        self.t0 = time.monotonic()
        inicio = datetime.datetime.now().isoformat(timespec="seconds")
        self.arquivo.write(f"# captura iniciada em {inicio}; o tempo do PC conta a partir daqui\n")
        self.arquivo.flush()

    def agora(self):
        return time.monotonic() - self.t0

    def escreve(self, t, nome, texto):
        linha = f"{t:.3f}\t{nome}\t{texto}"
        with self.trava:
            self.arquivo.write(linha + "\n")
            self.arquivo.flush()
            print(linha, flush=True)

    def fecha(self):
        with self.trava:
            self.arquivo.close()


def le_porta(nome, porta, baud, saida, parar):
    aberta_antes = False
    while not parar.is_set():
        try:
            conexao = serial.Serial(porta, baud, timeout=0.2)
        except (serial.SerialException, OSError):
            time.sleep(0.5)
            continue

        saida.escreve(saida.agora(), "nota",
                      f"{nome}: porta {porta} {'reaberta' if aberta_antes else 'aberta'}")
        aberta_antes = True
        resto = b""
        try:
            with conexao:
                while not parar.is_set():
                    dados = conexao.read(1)   # espera o primeiro byte (até 0,2 s)
                    if not dados:
                        continue
                    dados += conexao.read(conexao.in_waiting)
                    t = saida.agora()          # todas as linhas do pacote chegaram agora
                    resto += dados
                    *linhas, resto = resto.split(b"\n")
                    for bruta in linhas:
                        texto = bruta.decode("utf-8", errors="replace").rstrip("\r")
                        if texto:
                            saida.escreve(t, nome, texto)
        except (serial.SerialException, OSError):
            saida.escreve(saida.agora(), "nota", f"{nome}: porta {porta} caiu")
            time.sleep(0.5)


def main():
    parser = argparse.ArgumentParser(description="Grava a serial de uma ou duas placas com o tempo do PC.")
    parser.add_argument("saida", help="arquivo .log de saída (é sobrescrito)")
    parser.add_argument("placas", nargs="+", metavar="NOME=PORTA",
                        help="por exemplo mestre=/dev/ttyACM0 ou unica=COM5")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    placas = []
    for item in args.placas:
        nome, sep, porta = item.partition("=")
        if not sep or not nome or not porta or nome == "nota":
            parser.error(f"placa inválida: '{item}'. Use NOME=PORTA, com NOME diferente de 'nota'.")
        placas.append((nome, porta))

    saida = Saida(args.saida)
    parar = threading.Event()
    threads = [threading.Thread(target=le_porta, args=(nome, porta, args.baud, saida, parar), daemon=True)
               for nome, porta in placas]
    for th in threads:
        th.start()

    print("Capturando. Digite uma nota e Enter para registrá-la; Ctrl+C encerra.", file=sys.stderr)
    try:
        for linha in sys.stdin:
            linha = linha.strip()
            if linha:
                saida.escreve(saida.agora(), "nota", linha)
        # Ctrl+D (fim da entrada): continua capturando até Ctrl+C
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        parar.set()
        for th in threads:
            th.join(timeout=1)
        saida.fecha()
        print(f"Log salvo em {args.saida}", file=sys.stderr)


if __name__ == "__main__":
    main()