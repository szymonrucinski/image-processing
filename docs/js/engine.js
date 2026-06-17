/* The engine room.
 *
 * Loads the four WASM modules (each is an original CImg CLI program, compiled
 * untouched) and drives them exactly as you would from a terminal:
 *   write input.bmp into the module's in-memory filesystem,
 *   callMain(argv) — runs the program's real main(),
 *   read the .bmp it saved back out.
 * A fresh module instance is created per run so global state (Task4's Fourier
 * buffers) never leaks between operations.
 */
const Engine = (() => {
  const loaded = {};
  function loadFactory(task) {
    if (loaded[task]) return loaded[task];
    loaded[task] = new Promise((resolve, reject) => {
      const name = 'create' + task;
      if (window[name]) return resolve(window[name]);
      const s = document.createElement('script');
      s.src = 'wasm/' + task + '.js';
      s.onload = () => resolve(window[name]);
      s.onerror = () => reject(new Error('failed to load ' + task));
      document.head.appendChild(s);
    });
    return loaded[task];
  }

  // ---- working-image preprocessing (canvas) -------------------------------
  const scratch = document.createElement('canvas');
  const sctx = scratch.getContext('2d', { willReadFrequently: true });
  const srcCanvas = document.createElement('canvas');
  const srcCtx = srcCanvas.getContext('2d', { willReadFrequently: true });

  function toCanvas(imgData) {
    srcCanvas.width = imgData.width; srcCanvas.height = imgData.height;
    srcCtx.putImageData(imgData, 0, 0);
    return srcCanvas;
  }

  function preprocess(src, work, threshold) {
    work = work || {};
    const sw = src.width, sh = src.height;
    let tw, th, sx = 0, sy = 0, ssw = sw, ssh = sh;
    if (work.square) {
      tw = th = work.square;
      const side = Math.min(sw, sh);          // center-crop to square (cover)
      sx = ((sw - side) / 2) | 0; sy = ((sh - side) / 2) | 0; ssw = ssh = side;
    } else {
      const maxEdge = Math.min(work.maxEdge || 700, Math.max(sw, sh));
      const scale = maxEdge / Math.max(sw, sh);
      tw = Math.max(1, Math.round(sw * scale));
      th = Math.max(1, Math.round(sh * scale));
    }
    scratch.width = tw; scratch.height = th;
    sctx.imageSmoothingQuality = 'high';
    sctx.clearRect(0, 0, tw, th);
    sctx.drawImage(toCanvas(src), sx, sy, ssw, ssh, 0, 0, tw, th);
    const out = sctx.getImageData(0, 0, tw, th);
    const d = out.data;
    if (work.gray || work.binary) {
      const thr = threshold == null ? 110 : threshold;
      for (let i = 0; i < d.length; i += 4) {
        let v = (0.299 * d[i] + 0.587 * d[i + 1] + 0.114 * d[i + 2]) | 0;
        if (work.binary) v = v > thr ? 255 : 0;
        d[i] = d[i + 1] = d[i + 2] = v;
      }
    }
    return out;
  }

  // ---- run one operation ---------------------------------------------------
  async function runOp(op, src, params, threshold) {
    const factory = await loadFactory(op.task);
    const work = preprocess(src, op.work, threshold);
    const inPath = op.inPath || OPS.IN;
    const outPath = op.out.startsWith('/') ? op.out : '/' + op.out;
    const argv = op.args(params || {});
    let stdout = '';
    const M = await factory({ print: s => { stdout += s + '\n'; }, printErr: () => {}, locateFile: f => 'wasm/' + f });
    M.FS.writeFile(inPath, BMP.encode(work));
    const t0 = performance.now();
    M.callMain(argv.slice());           // slice: callMain prepends argv[0] in place
    const ms = performance.now() - t0;
    const result = BMP.decode(M.FS.readFile(outPath));
    return { result, ms, argv, stdout, workW: work.width, workH: work.height, command: op.task + ' ' + argv.join(' ') };
  }

  // ---- statistics readout (stdout-only commands) ---------------------------
  async function runStats(src) {
    const factory = await loadFactory(OPS.STATS.task);
    const work = preprocess(src, { maxEdge: 700 });
    const bmp = BMP.encode(work);
    const out = [];
    for (const it of OPS.STATS.items) {
      let stdout = '';
      const M = await factory({ print: s => { stdout += s + '\n'; }, printErr: () => {}, locateFile: f => 'wasm/' + f });
      M.FS.writeFile(OPS.IN, bmp);
      M.callMain([it.cmd, OPS.IN]);
      out.push({ label: it.label, text: stdout.trim() });
    }
    return out;
  }

  async function runHistogram(src, ch) {
    const factory = await loadFactory(OPS.HISTOGRAM.task);
    const work = preprocess(src, { maxEdge: 700 });
    const M = await factory({ print: () => {}, printErr: () => {}, locateFile: f => 'wasm/' + f });
    M.FS.writeFile(OPS.IN, BMP.encode(work));
    M.callMain(OPS.HISTOGRAM.args(ch));
    return BMP.decode(M.FS.readFile('/' + OPS.HISTOGRAM.out));
  }

  return { runOp, runStats, runHistogram, preprocess };
})();
