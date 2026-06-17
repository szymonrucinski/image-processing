# The Latent Darkroom, browser port of this image-processing coursework

This branch wraps the original C++/CImg programs in `Task1…Task4` in a small
web "museum": you apply the algorithms to public-domain photographs live in the
browser. The C++ is compiled **untouched** to WebAssembly with Emscripten and
driven exactly as it is from a terminal, write `input.bmp` into an in-memory
filesystem, run the program's real `main()`, read back the `.bmp` it saves.

```
canvas pixels ──encode 24-bit BMP──▶ Emscripten MEMFS  /input.bmp
              Module.callMain(['--roberts','/input.bmp'])   ← your main(), unchanged
              MEMFS /newOne.bmp ──decode BMP──▶ canvas
```

## What changed, precisely

Your **algorithm source is byte-for-byte unchanged**, every file under
`Task*/source_files/*.cpp` is exactly as it was. Only these changed:

1. **`Task*/header_files/CImg.h` (vendored third-party library, 4 copies)**, one
   function. Under `-Dcimg_display=0` (no window system, which is the case in a
   browser) CImg's `CImgDisplay::_no_display_exception()` used to `throw`. It now
   returns silently, so the original `CImgDisplay disp(image); while(!disp.is_closed()) disp.wait();`
   blocks become harmless no-ops and execution continues to `image.save(...)`.
   This is library behaviour for a headless target, not your code.

2. **`Task3/header_files/functions.h` and `RegionGrowing.h`**, these still
   contained unresolved git merge-conflict markers (`<<<<<<< HEAD … >>>>>>> dev`)
   and never compiled. Resolved to the `dev` side, which is the only one
   consistent with `Task3/source_files/main.cpp` (it calls the `CImg`-returning
   `applySegmentation` and the 2-argument `HMT`).

## What is exhibited vs. omitted

Exhibited: negative, brightness, contrast, gamma, median, harmonic-mean, three
flips, Roberts edges, line detector, Rayleigh equalisation, the FFT spectrum and
low/high/band-pass frequency filters, erosion/dilation/opening/closing/hit-or-miss,
the histogram plot, and the statistical measures (mean, variance, std-dev,
variation coefficient, skewness, kurtosis).

Omitted from the UI (the code still compiles):
- **enlarge / shrink**, the originals build the output with width/height swapped
  (`CImg img(height, width, …)`), so non-square images come out transposed.
- **region growing (`--reggrow`)**, a recursive flood-fill; on any sizeable
  region it exceeds the browser's WASM call-stack depth (V8 caps recursion
  regardless of linear-memory stack size).
- **`--M2`**, reads coordinates from `stdin`; no terminal in a browser.
- **DFT / IDFT**, O(N⁴); the FFT path is exhibited instead.
- **mse / snr / psnr / md**, compare against a second image.

## Building the WASM

Needs the Emscripten SDK on `PATH` (`source ~/emsdk/emsdk_env.sh`). Then:

```bash
bash build/build.sh      # emits docs/wasm/Task{1..4}.{js,wasm}
```

## The site (`docs/`)

Static, no build step, deployable to GitHub Pages from the `docs/` folder:
`index.html`, `styles.css`, `js/{bmp,operations,engine,app}.js`,
`wasm/Task*.{js,wasm}`, `assets/gallery/*` and `gallery.json`. All eight
photographs are public-domain works (NASA, the U.S. National Archives, the Farm
Security Administration); see each exhibit's placard for provenance.
