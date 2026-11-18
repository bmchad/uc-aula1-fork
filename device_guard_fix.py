# Contorna o bloqueio do arm-none-eabi-g++.exe pelo Device Guard (Smart App
# Control) do Windows. O build falhava com "foi bloqueado pela politica do
# Device Guard" (Error 4551).
#
# O builder Zephyr do PlatformIO sempre compila um __dummy.cpp vazio, para o
# CMake configurar todas as linguagens. Como o projeto nao tem codigo C++, o
# g++ so e usado para esse arquivo e, pelo SmartLink do SCons, como driver do
# link. O arm-none-eabi-gcc compila .cpp do mesmo jeito (escolhe a linguagem
# pela extensao e chama o cc1plus) e nao esta bloqueado.
#
# Se um dia o projeto tiver codigo C++ que precise da libstdc++, revisar.
import os

Import("env")

# Ambiente principal: vale para o link.
env.Replace(CXX="arm-none-eabi-gcc")

# O builder do Zephyr clona um ambiente por grupo de compilacao antes de os
# scripts post: rodarem, e o __dummy.cpp e compilado num desses clones, que
# ainda aponta para o g++. O SCons so monta o comando na hora de executar,
# entao trocar o CXX no ambiente desse objeto ainda vale.
dummy_cpp = env.File(os.path.join(
    "$BUILD_DIR", "_pio___bare_module", "modules", "_bare_module",
    "__dummy.cpp.o"))
if dummy_cpp.has_builder():
    dummy_cpp.get_executor().get_build_env()["CXX"] = "arm-none-eabi-gcc"
