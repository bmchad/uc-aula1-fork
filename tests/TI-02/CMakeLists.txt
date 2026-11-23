cmake_minimum_required(VERSION 3.20.0)

# TI-02: teste de integração das duas vias (2 semáforos de pedestres e 2 de
# carros, em duas placas).
#
# Compila os módulos integrados (lib/enlace, lib/semaforo_veiculo e
# lib/semaforo_pedestres), o programa de teste e o dublê do modo noturno
# (src/dubles.c). Uma placa é gravada como mestre e a outra como escrava, com
# uma pasta de build por papel:
#   west build -b frdm_kl25z -d build/TI-02-mestre  tests/TI-02 -- -DPAPEL=mestre
#   west build -b frdm_kl25z -d build/TI-02-escrava tests/TI-02 -- -DPAPEL=escrava

get_filename_component(RAIZ ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)
set(DTC_OVERLAY_FILE ${RAIZ}/frdm_kl25z.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ti02-duas-vias)

set(PAPEL mestre CACHE STRING "Papel da placa no teste: mestre ou escrava")
set_property(CACHE PAPEL PROPERTY STRINGS mestre escrava)

if(PAPEL STREQUAL "mestre")
  target_compile_definitions(app PRIVATE TI02_MESTRE=1)
elseif(PAPEL STREQUAL "escrava")
  target_compile_definitions(app PRIVATE TI02_MESTRE=0)
else()
  message(FATAL_ERROR "PAPEL invalido: '${PAPEL}'. Use mestre ou escrava.")
endif()
message(STATUS "TI-02: papel da placa: ${PAPEL}")

target_sources(app PRIVATE
  src/main.c
  src/dubles.c
  ${RAIZ}/lib/enlace/enlace.c
  ${RAIZ}/lib/semaforo_veiculo/semaforo_veiculo.c
  ${RAIZ}/lib/semaforo_pedestres/semaforo_pedestres.c
)
# Do modo noturno, só o cabeçalho: o código dele é trocado pelo dublê
target_include_directories(app PRIVATE
  ${RAIZ}/lib/enlace
  ${RAIZ}/lib/semaforo_veiculo
  ${RAIZ}/lib/semaforo_pedestres
  ${RAIZ}/lib/modo_noturno
)
