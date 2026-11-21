cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(semaforo-pedestres-veiculo)

# Para compilar para a placa escrava, comentar a linha da mestre e descomentar a linha da escrava
target_sources(app PRIVATE src/mestre.c)
# target_sources(app PRIVATE src/escrava.c)