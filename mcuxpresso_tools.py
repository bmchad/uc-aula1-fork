# Usa as ferramentas instaladas pelo MCUXpresso Installer no build/upload:
#
# - arm-none-eabi-size: o Smart App Control do Windows bloqueia o do
#   toolchain-gccarmnoneeabi@1.80201 do PlatformIO, e o "checkprogsize" roda
#   antes de todo upload. Usa o do Arm GNU Toolchain em ~/.mcuxpressotools.
# - LinkServer: o Upload grava o ELF via SWD pelo LinkServer mais novo em
#   C:/NXP, entao atualizar o LinkServer pelo installer nao quebra o upload.
import glob
import os
import re

Import("env")

LINKSERVER_DEVICE = "MKL25Z4:FRDM-KL25Z"


def mais_novo(padrao, regex_versao):
    def versao(caminho):
        m = re.search(regex_versao, caminho.replace("\\", "/"))
        return tuple(int(x) for x in m.group(1).split(".")) if m else ()
    caminhos = glob.glob(padrao)
    return max(caminhos, key=versao) if caminhos else None


size = mais_novo(
    os.path.join(os.path.expanduser("~"), ".mcuxpressotools",
                 "arm-gnu-toolchain-*", "bin", "arm-none-eabi-size.exe"),
    r"arm-gnu-toolchain-(\d+(?:\.\d+)*)")
if size:
    env.Replace(SIZETOOL=size)
else:
    print("AVISO: arm-none-eabi-size do MCUXpresso nao encontrado em "
          "~/.mcuxpressotools; usando o do PlatformIO (pode ser bloqueado).")

linkserver = mais_novo("C:/NXP/LinkServer_*/LinkServer.exe",
                       r"LinkServer_(\d+(?:\.\d+)*)/")
if linkserver:
    env.Replace(UPLOADCMD='"%s" flash %s load "$PROGPATH"'
                % (linkserver, LINKSERVER_DEVICE))
else:
    print("AVISO: LinkServer nao encontrado em C:/NXP; instale-o pelo "
          "MCUXpresso Installer para o Upload funcionar.")
