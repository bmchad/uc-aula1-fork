# Faz o build perceber mudanças no zephyr/app.overlay.
#
# O builder Zephyr do PlatformIO só refaz a configuração do CMake, que é quem
# gera o devicetree, quando o zephyr/CMakeLists.txt ou o zephyr/prj.conf ficam
# mais novos que o CMakeCache.txt (is_cmake_reconfigure_required, em
# framework-zephyr/scripts/platformio/platformio-build.py). Uma mudança só no
# app.overlay passava despercebida, e o firmware saía com os pinos antigos.
#
# Este script roda antes do builder (pre:) e apaga o CMakeCache.txt do
# ambiente quando o overlay é mais novo que ele. O builder então refaz a
# configuração. O cache é regenerado sozinho.
import os

Import("env")

cache = os.path.join(env.subst("$BUILD_DIR"), "CMakeCache.txt")
overlay = os.path.join(env.subst("$PROJECT_DIR"), "zephyr", "app.overlay")

if (os.path.isfile(cache) and os.path.isfile(overlay)
        and os.path.getmtime(overlay) > os.path.getmtime(cache)):
    print("zephyr_overlay_fix: app.overlay mudou, refazendo a configuracao do CMake")
    os.remove(cache)
