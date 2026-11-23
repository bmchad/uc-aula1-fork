cmake_minimum_required(VERSION 3.20.0)

# TU-02: teste unitário do semáforo dos carros (ver README.md desta pasta).
#
# Compila só o módulo sob teste (lib/semaforo_veiculo), o programa de teste e
# os dublês dos módulos de que ele depende (src/dubles.c). Os pinos são os
# mesmos do firmware principal: o overlay vem da raiz do projeto.
#   west build -b frdm_kl25z -d build/TU-02 tests/TU-02

get_filename_component(RAIZ ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)
set(DTC_OVERLAY_FILE ${RAIZ}/frdm_kl25z.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tu02-semaforo-carros)

target_sources(app PRIVATE
  src/main.c
  src/dubles.c
  ${RAIZ}/lib/semaforo_veiculo/semaforo_veiculo.c
)
# Só os cabeçalhos dos outros módulos: o código deles é trocado pelos dublês
target_include_directories(app PRIVATE
  ${RAIZ}/lib/semaforo_veiculo
  ${RAIZ}/lib/enlace
  ${RAIZ}/lib/modo_noturno
  ${RAIZ}/lib/semaforo_pedestres
)
