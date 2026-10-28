#!/bin/sh
# Pruebas del compilador.
#   sh tests/run_tests.sh ./bin/compilador.exe          (UCRT64)
#   sh tests/run_tests.sh ./bin/compilador.exe --host   (CLANG64: ademas ejecuta cada .ll)
#
# - examples/*.olc deben compilar; con --host su salida debe ser igual a .esperado
# - tests/errores/*.olc deben FALLAR y NO deben dejar un .ll generado

COMPILER=${1:-./bin/compilador.exe}
HOST=$2
OUT=build/tests
mkdir -p "$OUT"
pass=0
fail=0

for src in examples/*.olc; do
    name=$(basename "$src" .olc)
    if ! "$COMPILER" "$src" -o "$OUT/$name.ll" >/dev/null; then
        echo "FALLA  $src no compilo"; fail=$((fail + 1)); continue
    fi
    if [ -n "$HOST" ]; then
        if ! llvm-as "$OUT/$name.ll" -o "$OUT/$name.bc"; then
            echo "FALLA  $name.ll no es LLVM IR valido"; fail=$((fail + 1)); continue
        fi
        clang -Wno-override-module "$OUT/$name.ll" runtime/host/runtime_host.c \
            runtime/common/formato.c -o "$OUT/$name.exe" || { fail=$((fail + 1)); continue; }
        "$OUT/$name.exe" | tr -d '\r' > "$OUT/$name.salida"
        if ! diff -u "examples/$name.esperado" "$OUT/$name.salida"; then
            echo "FALLA  $src salida distinta"; fail=$((fail + 1)); continue
        fi
    fi
    echo "OK     $src"; pass=$((pass + 1))
done

for src in tests/errores/*.olc; do
    name=$(basename "$src" .olc)
    rm -f "$OUT/$name.ll"
    if "$COMPILER" "$src" -o "$OUT/$name.ll" >/dev/null 2>"$OUT/$name.err"; then
        echo "FALLA  $src deberia dar error"; fail=$((fail + 1))
    elif [ -f "$OUT/$name.ll" ]; then
        echo "FALLA  $src genero LLVM IR a pesar del error"; fail=$((fail + 1))
    else
        echo "OK     $src -> $(head -n 1 "$OUT/$name.err")"; pass=$((pass + 1))
    fi
done

echo "Resultado: $pass correctas, $fail fallidas"
[ "$fail" -eq 0 ]
