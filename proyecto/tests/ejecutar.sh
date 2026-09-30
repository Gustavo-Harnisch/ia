#!/usr/bin/env bash
set -euo pipefail
proyecto_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
pruebas_dir="$(mktemp -d /tmp/inf423-pruebas.XXXXXX)"
trap 'rm -rf -- "$pruebas_dir"' EXIT
g++ -std=c++17 -O1 -g -Wall -Wextra -Wpedantic -pthread \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$proyecto_dir/src" "$proyecto_dir/tests/evolutivo_test.cpp" \
    "$proyecto_dir/src/"{Grafo,MST,Steiner,Heuristica,Evolutivo}.cpp \
    -o "$pruebas_dir/evolutivo_test"
"$pruebas_dir/evolutivo_test"
