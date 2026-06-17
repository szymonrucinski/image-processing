/* Minimal BMP codec — the bridge between the browser's canvas (RGBA, top-down)
 * and what the original CImg C++ reads/writes: 24-bit uncompressed BGR, rows
 * stored bottom-up, each row padded to a 4-byte boundary.
 *
 * We always ENCODE 24-bit RGB (greyscale sources get R=G=B) so the C++ always
 * sees spectrum==3. We DECODE 24/32/8-bit because CImg::save_bmp writes 24-bit
 * (greyscale results come back as R=G=B), but we stay liberal in what we accept.
 */
(function (root) {
  function rd32(b, o) { return b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24); }
  function rd16(b, o) { return b[o] | (b[o + 1] << 8); }

  // ImageData-like {width,height,data:Uint8ClampedArray RGBA} -> Uint8Array BMP
  function encode(img) {
    const w = img.width, h = img.height, px = img.data;
    const rowRaw = w * 3;
    const pad = (4 - (rowRaw % 4)) % 4;
    const rowSize = rowRaw + pad;
    const dataSize = rowSize * h;
    const fileSize = 54 + dataSize;
    const out = new Uint8Array(fileSize);
    // BITMAPFILEHEADER
    out[0] = 0x42; out[1] = 0x4d;                 // 'BM'
    wr32(out, 2, fileSize);
    wr32(out, 10, 54);                            // pixel data offset
    // BITMAPINFOHEADER
    wr32(out, 14, 40);
    wr32(out, 18, w);
    wr32(out, 22, h);                             // positive => bottom-up
    out[26] = 1;                                  // planes
    out[28] = 24;                                 // bpp
    wr32(out, 34, dataSize);
    wr32(out, 38, 2835); wr32(out, 42, 2835);     // 72 DPI
    let o = 54;
    for (let y = h - 1; y >= 0; y--) {            // bottom-up
      let s = y * w * 4;
      for (let x = 0; x < w; x++) {
        out[o++] = px[s + 2];                     // B
        out[o++] = px[s + 1];                     // G
        out[o++] = px[s];                         // R
        s += 4;
      }
      for (let p = 0; p < pad; p++) out[o++] = 0;
    }
    return out;
  }

  function wr32(b, o, v) { b[o] = v & 255; b[o + 1] = (v >> 8) & 255; b[o + 2] = (v >> 16) & 255; b[o + 3] = (v >> 24) & 255; }

  // Uint8Array BMP -> {width,height,data:Uint8ClampedArray RGBA}
  function decode(buf) {
    const b = buf instanceof Uint8Array ? buf : new Uint8Array(buf);
    if (b[0] !== 0x42 || b[1] !== 0x4d) throw new Error('not a BMP');
    const dataOffset = rd32(b, 10);
    const headerSize = rd32(b, 14);
    let w = rd32(b, 18);
    let hRaw = rd32(b, 22) | 0;                    // signed
    const topDown = hRaw < 0;
    const h = Math.abs(hRaw);
    const bpp = rd16(b, 28);
    const out = new Uint8ClampedArray(w * h * 4);

    // palette (for 8-bit)
    let palOff = 14 + headerSize;
    const readRow = (bpp === 24 || bpp === 32)
      ? function (o, dst) {
          const step = bpp === 32 ? 4 : 3;
          for (let x = 0; x < w; x++) {
            dst[x * 4] = b[o + 2]; dst[x * 4 + 1] = b[o + 1]; dst[x * 4 + 2] = b[o]; dst[x * 4 + 3] = 255;
            o += step;
          }
        }
      : function (o, dst) {                        // 8-bit palette
          for (let x = 0; x < w; x++) {
            const idx = b[o + x]; const p = palOff + idx * 4;
            dst[x * 4] = b[p + 2]; dst[x * 4 + 1] = b[p + 1]; dst[x * 4 + 2] = b[p]; dst[x * 4 + 3] = 255;
          }
        };
    const rowRaw = bpp === 8 ? w : w * (bpp / 8);
    const rowSize = rowRaw + ((4 - (rowRaw % 4)) % 4);
    const tmp = new Uint8ClampedArray(w * 4);
    for (let y = 0; y < h; y++) {
      const srcY = topDown ? y : (h - 1 - y);
      readRow(dataOffset + srcY * rowSize, tmp);
      out.set(tmp, y * w * 4);
    }
    return { width: w, height: h, data: out };
  }

  const api = { encode, decode };
  if (typeof module !== 'undefined' && module.exports) module.exports = api;
  else root.BMP = api;
})(typeof globalThis !== 'undefined' ? globalThis : this);
