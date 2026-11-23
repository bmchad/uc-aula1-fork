cmake_minimum_required(VERSION 3.20.0)

# TU-03: teste unitário da comunicação UART entre as placas.
#
# Compila só o módulo sob teste (lib/enlace) e o programa de teste. Uma placa
# é gravada como mestre e a outra como escrava: o enlace descarta os quadros
# que vêm de uma placa com o mesmo papel. Uma pasta de build por papel:
#   west build -b frdm_kl25z -d build/TU-03-mestre  tests/TU-03 -- -DPAPEL=mestre
#   west build -b frdm_kl25z -d build/TU-03-escrava tests/TU-03 -- -DPAPEL=escrava

get_filename_component(RAIZ ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)
set(DTC_OVERLAY_FILE ${RAIZ}/frdm_kl25z.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tu03-enlace-uart)

set(PAPEL mestre CACHE STRING "Papel da placa no teste: mestre ou escrava")
set_property(CACHE PAPEL PROPERTY STRINGS mestre escrava)

if(PAPEL STREQUAL "mestre")
  target_compile_definitions(app PRIVATE TU03_MESTRE=1)
elseif(PAPEL STREQUAL "escrava")
  target_compile_definitions(app PRIVATE TU03_MESTRE=0)
else()
  message(FATAL_ERROR "PAPEL invalido: '${PAPEL}'. Use mestre ou escrava.")
endif()
message(STATUS "TU-03: papel da placa: ${PAPEL}")

target_sources(app PRIVATE
  src/main.c
  ${RAIZ}/lib/enlace/enlace.c
)
target_include_directories(app PRIVATE
  ${RAIZ}/lib/enlace
)
