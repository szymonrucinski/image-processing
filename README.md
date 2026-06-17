# The Image Processing

A small in-browser museum of image processing. Pick a photograph and run classic algorithms on it —
brightness and contrast, median and harmonic filters, edge and line detection, histogram equalisation,
morphology, and the Fourier domain — live, in your browser.

**Live:** https://szymonrucinski.github.io/Image-Processing/

The algorithms are the original C++ (CImg) I wrote in 2019 for the Computer Vision course of my
Information Technology bachelor's degree. They are compiled to WebAssembly with Emscripten and run
**unchanged**: canvas pixels → a BMP in Emscripten's in-memory filesystem → the program's real `main()`
→ a BMP back → the canvas. No image is uploaded; everything runs on your machine.

## Layout

- `Task1`–`Task4/` — the original C++/CImg programs (the algorithm source is unchanged).
- `docs/` — the website (HTML/CSS/JS and the compiled `wasm/`), served by GitHub Pages.
- `build/build.sh` — compiles the four programs to WebAssembly.
- `WEB-MUSEUM.md` — what changed for the browser port, and which operations are exhibited vs. omitted.

## Build the WebAssembly

```bash
source ~/emsdk/emsdk_env.sh      # Emscripten SDK
bash build/build.sh              # emits docs/wasm/Task{1..4}.{js,wasm}
```

## Credits

Algorithms by Szymon Ruciński (2019). The curated gallery is public-domain photography — NASA, the
Farm Security Administration / Office of War Information, the U.S. National Archives (Ansel Adams's
National Park Service series), and the 1873 Wheeler Survey — via the Library of Congress, NARA and
Wikimedia Commons.
