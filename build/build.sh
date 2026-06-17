#!/usr/bin/env bash
# Build the original CImg CLI programs (UNCHANGED) to WebAssembly.
# Each Task keeps its own main(); we drive it from JS via Module.callMain([...]).
#   -Dcimg_display=0  -> CImgDisplay becomes a no-op (constructor sets _is_closed=true),
#                        so the desktop GUI wait-loop exits instantly. Code runs untouched.
#   MEMFS + FORCE_FILESYSTEM -> CImg's fopen/save("newOne.bmp") hit an in-memory FS we
#                        seed from canvas pixels and read back.
set -euo pipefail
cd "$(dirname "$0")/.."          # repo root
source ~/emsdk/emsdk_env.sh >/dev/null 2>&1
OUT=docs/wasm
mkdir -p "$OUT"

COMMON="-Dcimg_display=0 -O2 -std=c++14 -fexceptions \
  -sMODULARIZE=1 -sINVOKE_RUN=0 -sEXIT_RUNTIME=0 \
  -sFORCE_FILESYSTEM=1 -sALLOW_MEMORY_GROWTH=1 \
  -sEXPORTED_RUNTIME_METHODS=callMain,FS \
  -sEXPORTED_FUNCTIONS=_main \
  -Wno-deprecated-declarations -Wno-unused-command-line-argument"

echo ">> Task1 (point ops, filters, geometry)"
emcc Task1/source_files/main.cpp $COMMON \
  -sEXPORT_NAME=createTask1 -o "$OUT/Task1.js"

echo ">> Task2 (histogram, Rayleigh, edges, stats)"
emcc Task2/source_files/main.cpp Task2/source_files/function.cpp $COMMON \
  -sEXPORT_NAME=createTask2 -o "$OUT/Task2.js"

echo ">> Task3 (morphology, region growing)"   # functions.cpp only; RegionGrowing.cpp duplicates symbols
emcc Task3/source_files/main.cpp Task3/source_files/functions.cpp -ITask3 $COMMON \
  -sSTACK_SIZE=83886080 \
  -sEXPORT_NAME=createTask3 -o "$OUT/Task3.js"

echo ">> Task4 (Fourier domain)"
emcc Task4/source_files/main.cpp Task4/source_files/functions.cpp Task4/source_files/Fourier.cpp -ITask4 $COMMON \
  -sEXPORT_NAME=createTask4 -o "$OUT/Task4.js"

echo "DONE -> $OUT"
ls -la "$OUT"
