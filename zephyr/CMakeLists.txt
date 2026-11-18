# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(blinky)

# O PlatformIO ignora a biblioteca "app" do CMake e compila o src/ pelo
# build_src_filter de cada ambiente (platformio.ini): mestre.c ou escrava.c.
# Esta linha só precisa citar um fonte que exista.
target_sources(app PRIVATE ../src/mestre.c)
