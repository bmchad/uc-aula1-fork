cmake_minimum_required(VERSION 3.20.0)

# TI-01: teste de integração entre o semáforo de pedestres e o de carros, numa
# placa só.
#
# Compila os dois módulos integrados (lib/semaforo_veiculo e
# lib/semaforo_pedestres), o programa de teste e os dublês dos módulos que
# ficam fora deste teste (src/dubles.c). Os pinos são os mesmos do firmware
# principal: o overlay vem da raiz do projeto.
#   west build -b frdm_kl25z -d build/TI-01 tests/TI-01

get_filename_component(RAIZ ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)
set(DTC_OVERLAY_FILE ${RAIZ}/frdm_kl25z.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ti01-pedestres-carros)

target_sources(app PRIVATE
  src/main.c
  src/dubles.c
  ${RAIZ}/lib/semaforo_veiculo/semaforo_veiculo.c
  ${RAIZ}/lib/semaforo_pedestres/semaforo_pedestres.c
)
# Do enlace e do modo noturno, só os cabeçalhos: o código deles é trocado pelos dublês
target_include_directories(app PRIVATE
  ${RAIZ}/lib/semaforo_veiculo
  ${RAIZ}/lib/semaforo_pedestres
  ${RAIZ}/lib/enlace
  ${RAIZ}/lib/modo_noturno
)
