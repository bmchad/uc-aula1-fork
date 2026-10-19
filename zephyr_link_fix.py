# Corrige o link do Zephyr 4.2 (framework-zephyr 3.40200) no Cortex-M0+.
#
# O builder Zephyr do PlatformIO monta o comando de link a partir do code
# model do CMake, que nao traz -mcpu/-mthumb. Sem eles o arm-none-eabi-gcc
# escolhe a libgcc padrao, compilada em modo ARM (32 bits). O Cortex-M0+ so
# executa Thumb e nao tem divisao em hardware, entao a primeira divisao do
# boot (__udivsi3, chamada por CLOCK_GetPll0Freq) cai em HardFault antes do
# main: o upload grava, mas a placa nao roda e nada sai na serial.
#
# Os dois links (firmware-pre0.elf e firmware.elf) leem LINKFLAGS so na hora
# de executar, entao acrescentar aqui vale para ambos.
Import("env")

cpu = env.BoardConfig().get("build.cpu")  # "cortex-m0plus" na frdm_kl25z
env.Append(LINKFLAGS=["-mcpu=%s" % cpu, "-mthumb"])
