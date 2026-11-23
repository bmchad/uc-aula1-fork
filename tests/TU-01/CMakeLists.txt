cmake_minimum_required(VERSION 3.20.0)

# TU-01: teste unitário do semáforo de pedestres (ver README.md desta pasta).
#
# Compila só o módulo sob teste (lib/semaforo_pedestres) e um programa de
# teste que faz o papel do semáforo de veículos. Os pinos são os mesmos do
# firmware principal: o overlay vem da raiz do projeto.
#   west build -b frdm_kl25z -d build/TU-01 tests/TU-01

get_filename_component(RAIZ ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)
set(DTC_OVERLAY_FILE ${RAIZ}/frdm_kl25z.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tu01-semaforo-pedestres)

target_sources(app PRIVATE
  src/main.c
  ${RAIZ}/lib/semaforo_pedestres/semaforo_pedestres.c
)
target_include_directories(app PRIVATE
  ${RAIZ}/lib/semaforo_pedestres
  ${RAIZ}/lib/modo_noturno
)
