cmake_minimum_required(VERSION 3.20.0)

# TS-01: teste de sistema do modo noturno, numa placa só.
#
# Compila os módulos do sistema de uma via (lib/modo_noturno, com o botão no
# D9, lib/semaforo_veiculo e lib/semaforo_pedestres), o programa de teste e o
# dublê do enlace (src/dubles.c). Os pinos são os mesmos do firmware
# principal: o overlay vem da raiz do projeto.
#   west build -b frdm_kl25z -d build/TS-01 tests/TS-01

get_filename_component(RAIZ ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)
set(DTC_OVERLAY_FILE ${RAIZ}/frdm_kl25z.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ts01-modo-noturno)

target_sources(app PRIVATE
  src/main.c
  src/dubles.c
  ${RAIZ}/lib/modo_noturno/modo_noturno.c
  ${RAIZ}/lib/semaforo_veiculo/semaforo_veiculo.c
  ${RAIZ}/lib/semaforo_pedestres/semaforo_pedestres.c
)
# Do enlace, só o cabeçalho: o código dele é trocado pelo dublê
target_include_directories(app PRIVATE
  ${RAIZ}/lib/modo_noturno
  ${RAIZ}/lib/semaforo_veiculo
  ${RAIZ}/lib/semaforo_pedestres
  ${RAIZ}/lib/enlace
)
