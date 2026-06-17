/* The Latent Darkroom — gallery + developing studio controller. */
(() => {
  const $ = s => document.querySelector(s);
  const el = (t, c, h) => { const e = document.createElement(t); if (c) e.className = c; if (h != null) e.innerHTML = h; return e; };

  const state = { manifest: null, exhibit: null, src: null, op: null, threshold: 110,
                  lastResult: null, busy: false, pending: false, debounce: 0 };

  // ---------------------------------------------------------------- bootstrap
  async function init() {
    const manifest = await fetch('gallery.json').then(r => r.json());
    state.manifest = manifest;
    const mu = manifest.museum;
    $('#brandName').textContent = mu.museumName;
    $('#heroTitle').textContent = mu.museumName;
    document.title = mu.museumName + ' — an image-processing museum';
    $('#heroTagline').textContent = mu.tagline;
    $('#heroIntro').textContent = mu.intro;
    $('#engineText').textContent = mu.pipelineExplainer;
    $('#creditsText').textContent = mu.creditsNote;
    $('#footCredits').textContent = mu.creditsNote;
    buildWall();
    wireNav();
  }

  // ------------------------------------------------------------------- wall
  function buildWall() {
    const wall = $('#wall');
    const io = new IntersectionObserver(es => es.forEach(e => { if (e.isIntersecting) { e.target.classList.add('in'); io.unobserve(e.target); } }), { threshold: .15 });
    state.manifest.exhibits.forEach((ex, i) => {
      const card = el('article', 'card');
      card.style.transitionDelay = (i % 4) * 60 + 'ms';
      card.innerHTML = `
        <div class="matframe"><div class="mat"><img loading="lazy" src="${ex.thumb}" alt="${escapeAttr(ex.altText)}"></div></div>
        <div class="placard-mini"><div class="t">${escapeHTML(ex.title)}</div><div class="b">${escapeHTML(ex.photographer)} · ${escapeHTML(ex.year)}</div></div>`;
      card.addEventListener('click', () => openExhibit(ex));
      wall.appendChild(card);
      io.observe(card);
    });
  }

  function wireNav() {
    $('#backBtn').addEventListener('click', () => showView('gallery'));
    document.querySelectorAll('[data-nav]').forEach(a => a.addEventListener('click', e => {
      if (a.dataset.nav === 'gallery') { e.preventDefault(); showView('gallery'); location.hash = '#gallery'; }
    }));
  }

  function showView(name) {
    $('#gallery').classList.toggle('is-active', name === 'gallery');
    $('#darkroom').classList.toggle('is-active', name === 'darkroom');
    window.scrollTo({ top: 0, behavior: 'instant' in window ? 'instant' : 'auto' });
  }

  // -------------------------------------------------------------- open exhibit
  async function openExhibit(ex) {
    state.exhibit = ex; state.op = null; state.lastResult = null; state.threshold = 110;
    $('#dTitle').textContent = ex.title;
    $('#dPhotographer').textContent = ex.photographer;
    $('#dYear').textContent = ex.year;
    $('#dCollection').textContent = ex.collection;
    $('#dPlacard').textContent = ex.placard;
    $('#dProv').textContent = ex.collection;
    $('#dCommons').href = ex.commonsPageUrl;
    buildRail();
    showView('darkroom');
    state.src = await loadImage(ex.img);
    // default instrument: an instant, obvious one
    const neg = findOp('negative');
    selectOp(neg);
  }

  function loadImage(url) {
    return new Promise((resolve, reject) => {
      const img = new Image();
      img.onload = () => {
        const c = el('canvas'); c.width = img.naturalWidth; c.height = img.naturalHeight;
        const x = c.getContext('2d'); x.drawImage(img, 0, 0);
        resolve(x.getImageData(0, 0, c.width, c.height));
      };
      img.onerror = reject; img.src = url;
    });
  }

  // ------------------------------------------------------------------- rail
  function buildRail() {
    const rail = $('#rail'); rail.innerHTML = '';
    OPS.CATEGORIES.forEach(cat => {
      const g = el('div', 'opgroup');
      g.appendChild(el('h3', null, cat.name));
      const ops = el('div', 'ops');
      cat.ops.forEach(op => {
        const b = el('button', 'op', op.label); b.dataset.op = op.id;
        b.addEventListener('click', () => selectOp(op));
        ops.appendChild(b);
      });
      g.appendChild(ops);
      rail.appendChild(g);
    });
    // controls block
    const ctl = el('div', 'opgroup');
    ctl.appendChild(el('div', 'opdesc', '<span id="opdesc">Choose an instrument above to develop this print.</span>'));
    ctl.appendChild(el('div', 'params')).id = 'params';
    ctl.innerHTML += `<div class="actions">
        <button class="btn" id="statsBtn">Statistics</button>
        <button class="btn" id="histBtn">Histogram</button>
        <button class="btn btn--primary" id="dlBtn">Download</button>
      </div>
      <div class="panel" id="panel" hidden></div>`;
    rail.appendChild(ctl);
    $('#statsBtn').addEventListener('click', showStats);
    $('#histBtn').addEventListener('click', showHistogram);
    $('#dlBtn').addEventListener('click', download);
    wireCompare();
  }

  function findOp(id) { for (const c of OPS.CATEGORIES) for (const o of c.ops) if (o.id === id) return o; }

  function selectOp(op) {
    state.op = op;
    document.querySelectorAll('.op').forEach(b => b.classList.toggle('active', b.dataset.op === op.id));
    $('#opdesc').textContent = op.blurb || '';
    renderParams(op);
    apply();
  }

  function renderParams(op) {
    const wrap = $('#params'); wrap.innerHTML = '';
    if (op.work && op.work.binary) addSlider(wrap, { id: '__thr', label: 'binarise', min: 40, max: 210, step: 5, def: state.threshold }, true);
    (op.params || []).forEach(p => addSlider(wrap, p, false));
  }

  function addSlider(wrap, p, isThreshold) {
    const row = el('div', 'param');
    const val = isThreshold ? state.threshold : p.def;
    row.innerHTML = `<label>${p.label}<span class="val">${fmt(val)}</span></label>
      <input type="range" min="${p.min}" max="${p.max}" step="${p.step}" value="${val}" data-pid="${p.id}">`;
    const input = row.querySelector('input'), out = row.querySelector('.val');
    input.addEventListener('input', () => {
      out.textContent = fmt(parseFloat(input.value));
      if (isThreshold) state.threshold = parseFloat(input.value);
      scheduleApply();
    });
    wrap.appendChild(row);
  }

  function collectParams() {
    const p = {};
    document.querySelectorAll('#params input[data-pid]').forEach(i => {
      if (i.dataset.pid === '__thr') return;
      p[i.dataset.pid] = parseFloat(i.value);
    });
    return p;
  }

  function fmt(v) { return Number.isInteger(v) ? v : (Math.round(v * 100) / 100); }

  // ------------------------------------------------------------------- apply
  function scheduleApply() { clearTimeout(state.debounce); state.debounce = setTimeout(apply, 130); }

  async function apply() {
    if (!state.op || !state.src) return;
    if (state.busy) { state.pending = true; return; }
    state.busy = true;
    const op = state.op;
    $('#spinner').hidden = false;
    // left plate: exactly what the C++ receives
    try {
      const input = Engine.preprocess(state.src, op.work, state.threshold);
      drawCanvas($('#srcCanvas'), input);
      $('#srcMeta').textContent = input.width + '×' + input.height + (op.work && op.work.binary ? ' · binary' : op.work && op.work.gray ? ' · grey' : '');
      const r = await Engine.runOp(op, state.src, collectParams(), state.threshold);
      state.lastResult = r.result;
      drawCanvas($('#outCanvas'), r.result, true);
      $('#outLabel').textContent = op.kind === 'spectrum' ? 'FFT spectrum' : op.kind === 'plot' ? 'Plot' : 'Result';
      $('#outMeta').textContent = r.result.width + '×' + r.result.height;
      const cmd = r.command.replace(/\/(input|i)\.bmp/g, '$1.bmp');
      $('#readout').classList.remove('err');
      $('#cmdText').innerHTML = `${escapeHTML(cmd)} <span class="dim"># ${r.workW}×${r.workH} · ${r.ms.toFixed(0)} ms</span>`;
    } catch (e) {
      $('#readout').classList.add('err');
      $('#cmdText').textContent = 'error: ' + (e && e.message ? e.message : e);
      console.error(e);
    }
    $('#spinner').hidden = true;
    state.busy = false;
    if (state.pending) { state.pending = false; apply(); }
  }

  // BMP.decode returns a plain {width,height,data}; putImageData needs a real ImageData.
  function asImageData(o) {
    if (typeof ImageData !== 'undefined' && o instanceof ImageData) return o;
    return new ImageData(o.data instanceof Uint8ClampedArray ? o.data : new Uint8ClampedArray(o.data), o.width, o.height);
  }
  function drawCanvas(canvas, o, animate) {
    const imageData = asImageData(o);
    canvas.width = imageData.width; canvas.height = imageData.height;
    canvas.getContext('2d').putImageData(imageData, 0, 0);
    if (animate) { canvas.style.animation = 'none'; void canvas.offsetWidth; canvas.style.animation = ''; }
  }

  // ----------------------------------------------------------- hold to compare
  function wireCompare() {
    const btn = $('#compareBtn'), out = $('#outCanvas');
    const show = () => { if (state.op && state.src) { const input = Engine.preprocess(state.src, state.op.work, state.threshold); drawCanvas(out, input); } };
    const hide = () => { if (state.lastResult) drawCanvas(out, state.lastResult); };
    ['mousedown', 'touchstart'].forEach(ev => btn.addEventListener(ev, e => { e.preventDefault(); show(); }));
    ['mouseup', 'mouseleave', 'touchend'].forEach(ev => btn.addEventListener(ev, hide));
  }

  // --------------------------------------------------------------- statistics
  async function showStats() {
    const panel = $('#panel'); panel.hidden = false;
    panel.innerHTML = '<h3>Image statistics</h3><div class="stats">measuring…</div>';
    try {
      const rows = await Engine.runStats(state.src);
      const html = rows.map(r => `<b>${escapeHTML(r.label)}</b>\n${escapeHTML(r.text)}`).join('\n\n');
      panel.querySelector('.stats').textContent = '';
      panel.querySelector('.stats').innerHTML = html;
    } catch (e) { panel.querySelector('.stats').textContent = 'failed: ' + e; }
  }

  // ---------------------------------------------------------------- histogram
  async function showHistogram() {
    const panel = $('#panel'); panel.hidden = false;
    panel.innerHTML = '<h3>Channel histograms</h3><div class="histos"></div>';
    const host = panel.querySelector('.histos');
    for (const ch of OPS.HISTOGRAM.channels) {
      const fig = el('figure'); const cv = el('canvas'); fig.appendChild(cv);
      fig.appendChild(el('figcaption', null, ch.label)); host.appendChild(fig);
      try { drawCanvas(cv, await Engine.runHistogram(state.src, ch.v)); } catch (e) { fig.appendChild(el('figcaption', null, 'err')); }
    }
  }

  // ----------------------------------------------------------------- download
  function download() {
    if (!state.lastResult) return;
    const r = asImageData(state.lastResult);
    const c = el('canvas'); c.width = r.width; c.height = r.height;
    c.getContext('2d').putImageData(r, 0, 0);
    c.toBlob(b => {
      const a = el('a'); a.href = URL.createObjectURL(b);
      a.download = `${state.exhibit.id}--${state.op.id}.png`;
      a.click(); setTimeout(() => URL.revokeObjectURL(a.href), 4000);
    }, 'image/png');
  }

  // ------------------------------------------------------------------ helpers
  function escapeHTML(s) { return String(s).replace(/[&<>]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;' }[c])); }
  function escapeAttr(s) { return String(s).replace(/"/g, '&quot;'); }

  init();
})();
