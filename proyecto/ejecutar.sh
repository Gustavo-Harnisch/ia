#!/usr/bin/env bash
set -euo pipefail

# Compilar fuera de GVFS/SFTP: ese montaje no admite archivos objeto.
proyecto_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
compilacion_dir="$(mktemp -d /tmp/inf423-ejecucion.XXXXXX)"
trap 'rm -rf -- "$compilacion_dir"' EXIT

dot_bin="$(command -v dot || true)"
if [[ -z "$dot_bin" ]]; then
    echo "Error: instala Graphviz (comando dot)." >&2
    exit 1
fi

printf 'Compilando proyecto…\n'
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -pthread \
    "-DPROYECTO_DIR=\"$proyecto_dir\"" \
    "-DGRAPHVIZ_DOT=\"$dot_bin\"" \
    "$proyecto_dir"/src/*.cpp -o "$compilacion_dir/proyecto"
"$compilacion_dir/proyecto" "$@"
