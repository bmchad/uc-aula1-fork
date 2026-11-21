cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(semaforo-pedestres-veiculo)

# Papel da placa, escolhido na compilação (padrão: mestre). Uma pasta de build
# por papel evita misturar as configurações:
#   west build -b frdm_kl25z -d build/mestre      -- -DPAPEL=mestre
#   west build -b frdm_kl25z -d build/escrava     -- -DPAPEL=escrava
#   west build -b frdm_kl25z -d build/placa_unica -- -DPAPEL=placa_unica
set(PAPEL mestre CACHE STRING "Papel da placa: mestre, escrava ou placa_unica")
set_property(CACHE PAPEL PROPERTY STRINGS mestre escrava placa_unica)

if(PAPEL STREQUAL "mestre")
  target_sources(app PRIVATE src/mestre.c)
elseif(PAPEL STREQUAL "escrava")
  target_sources(app PRIVATE src/escrava.c)
elseif(PAPEL STREQUAL "placa_unica")
  # Etapa 1: o mestre roda sozinho, sem os fios para a outra placa
  target_sources(app PRIVATE src/mestre.c)
  target_compile_definitions(app PRIVATE UMA_PLACA=1)
else()
  message(FATAL_ERROR "PAPEL invalido: '${PAPEL}'. Use mestre, escrava ou placa_unica.")
endif()
message(STATUS "Papel da placa: ${PAPEL}")

# Bibliotecas comuns às duas placas
target_sources(app PRIVATE
  lib/enlace/enlace.c
  lib/modo_noturno/modo_noturno.c
  lib/semaforo_pedestres/semaforo_pedestres.c
  lib/semaforo_veiculo/semaforo_veiculo.c
)
target_include_directories(app PRIVATE
  lib/enlace
  lib/modo_noturno
  lib/semaforo_pedestres
  lib/semaforo_veiculo
)
