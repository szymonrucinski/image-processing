/* The Image Processing, 3D edition.
 *
 * A walk-through museum built with Three.js. Photos come from the same
 * gallery.json as the 2D site, and opening one runs the same original C++
 * (via the 2D site's Engine / OPS / BMP globals and its wasm/ folder).
 *
 * Look: toon materials (3 hard light bands), inverted-hull outlines on frames,
 * and a post pass that inks depth/normal edges and hatches dark areas.
 * Photos and placards are drawn unlit and flagged (alpha 0) so the post pass
 * leaves their pixels alone.
 */
import * as THREE from 'three';

const $ = (s) => document.querySelector(s);
const el = (tag, cls, text) => { const e = document.createElement(tag); if (cls) e.className = cls; if (text != null) e.textContent = text; return e; };
const TOUCH = matchMedia('(pointer: coarse)').matches;

// ---------------------------------------------------------------- data + font
const manifest = await fetch('gallery.json').then((r) => r.json());
const EXHIBITS = manifest.exhibits;
await new Promise((r) => { const l = $('#fontcss'); if (l.sheet) r(); else { l.addEventListener('load', r, { once: true }); l.addEventListener('error', r, { once: true }); } });
await Promise.race([Promise.all([document.fonts.load('64px Bangers'), document.fonts.load('600 30px Inter')]), new Promise((r) => setTimeout(r, 3000))]);
const FONT = 'Bangers, Impact, sans-serif';

// ---------------------------------------------------------------- renderer
const canvas = $('#scene');
const renderer = new THREE.WebGLRenderer({ canvas, powerPreference: 'high-performance' });
renderer.shadowMap.enabled = !TOUCH;
renderer.shadowMap.type = THREE.PCFShadowMap;
renderer.shadowMap.autoUpdate = false; // the building never moves: render shadows once
const MAX_ANISO = renderer.capabilities.getMaxAnisotropy();

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x1a0f0b);
const camera = new THREE.PerspectiveCamera(70, 1, 0.1, 120);
camera.rotation.order = 'YXZ';
camera.layers.enable(1); // layer 1 = ink hulls, left out of the normal pass

// Cel shading: the toon material looks lighting up in this 3-texel ramp.
// NearestFilter means no blending between texels, so light falls into 3 hard bands.
const bands = new THREE.DataTexture(new Uint8Array([95, 180, 255]), 3, 1, THREE.RedFormat);
bands.minFilter = bands.magFilter = THREE.NearestFilter;
bands.needsUpdate = true;
const toon = (p) => new THREE.MeshToonMaterial({ gradientMap: bands, ...p });
const INK = 0x140c0a;

function canvasTex(w, h, draw) {
  const c = document.createElement('canvas');
  c.width = w; c.height = h;
  draw(c.getContext('2d'), w, h);
  const t = new THREE.CanvasTexture(c);
  t.colorSpace = THREE.SRGBColorSpace;
  t.anisotropy = MAX_ANISO;
  return t;
}

// Unlit picture material with a comic "ink blot" wipe from mapA to mapB.
// Alpha 0 marks these pixels as artwork for the post pass (no hatching, no colour boost).
const NOISE = /* glsl */ `
  float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
  float noise(vec2 p) {
    vec2 i = floor(p), f = fract(p); f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1, 0)), f.x), mix(hash(i + vec2(0, 1)), hash(i + vec2(1, 1)), f.x), f.y);
  }`;
function artMat(map) {
  return new THREE.ShaderMaterial({
    uniforms: { mapA: { value: map }, mapB: { value: map }, prog: { value: 0 } },
    vertexShader: 'varying vec2 vUv; void main() { vUv = uv; gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0); }',
    fragmentShader: /* glsl */ `
      uniform sampler2D mapA, mapB; uniform float prog; varying vec2 vUv;
      ${NOISE}
      void main() {
        vec3 a = texture2D(mapA, vUv).rgb, b = texture2D(mapB, vUv).rgb;
        float d = distance(vUv, vec2(0.5)) * 1.3 + noise(vUv * 6.0) * 0.45 + noise(vUv * 23.0) * 0.18;
        float front = prog * 1.75;
        float shown = step(d, front);
        float rim = (step(d, front + 0.07) - shown) * step(0.0001, prog) * step(prog, 0.9999);
        vec3 col = mix(mix(a, b, shown), vec3(0.006, 0.004, 0.003), rim);
        gl_FragColor = vec4(col, 0.0);
      }`,
  });
}

// ---------------------------------------------------------------- building
const HALF_W = 7, ROOM_D = 18, ROOMS = 3, H = 6, T = 0.4, DOOR = 4, DOOR_H = 4.2;
const LEN = ROOM_D * ROOMS;
const ROOM_COLORS = [0xb3262e, 0x0f7d8c, 0xd8901c];
const solids = [];   // meshes the crosshair ray can hit (for occlusion)
const boxes = [];    // XZ rectangles + height range the player collides with / stands on

function shadowed(o) { o.traverse((m) => { if (m.isMesh) m.castShadow = m.receiveShadow = true; }); return o; }

function box(cx, cy, cz, sx, sy, sz, mat, collide = true) {
  const m = shadowed(new THREE.Mesh(new THREE.BoxGeometry(sx, sy, sz), mat));
  m.position.set(cx, cy, cz);
  scene.add(m);
  if (collide) {
    boxes.push({ x0: cx - sx / 2, x1: cx + sx / 2, z0: cz - sz / 2, z1: cz + sz / 2, y0: cy - sy / 2, y1: cy + sy / 2 });
    solids.push(m);
  }
  return m;
}

// thick black inverted hull around a box-shaped object
const inkMat = new THREE.MeshBasicMaterial({ color: INK, side: THREE.BackSide });
function hullFor(mesh, pad, mat = inkMat) {
  const p = mesh.geometry.parameters;
  const h = new THREE.Mesh(new THREE.BoxGeometry(p.width + pad * 2, p.height + pad * 2, p.depth + pad * 2), mat);
  h.layers.set(1);
  mesh.add(h);
  return h;
}

function tileTexture() {
  const t = canvasTex(512, 512, (g, w) => {
    const s = w / 2;
    for (let i = 0; i < 4; i++) {
      const x = (i % 2) * s, y = (i >> 1) * s;
      g.fillStyle = (i === 0 || i === 3) ? '#efe2c2' : '#a8452a';
      g.fillRect(x, y, s, s);
      for (let k = 0; k < 220; k++) { // a little mottling so big floors aren't dead flat
        g.fillStyle = Math.random() < 0.5 ? 'rgba(0,0,0,0.05)' : 'rgba(255,255,255,0.06)';
        g.fillRect(x + Math.random() * s, y + Math.random() * s, 5, 5);
      }
    }
    g.strokeStyle = '#140c0a'; g.lineWidth = 10;
    for (let i = 0; i <= 2; i++) {
      g.beginPath(); g.moveTo(i * s, 0); g.lineTo(i * s, w); g.stroke();
      g.beginPath(); g.moveTo(0, i * s); g.lineTo(w, i * s); g.stroke();
    }
  });
  t.wrapS = t.wrapT = THREE.RepeatWrapping;
  return t;
}

function signTex(text, w, h, bg, fg, size) {
  return canvasTex(w, h, (g) => {
    g.fillStyle = bg; g.fillRect(0, 0, w, h);
    g.lineWidth = 14; g.strokeStyle = '#140c0a'; g.strokeRect(7, 7, w - 14, h - 14);
    g.textAlign = 'center'; g.textBaseline = 'middle';
    g.font = `${size}px ${FONT}`;
    g.lineWidth = size / 6; g.lineJoin = 'round'; g.strokeStyle = '#140c0a';
    const lines = text.split('\n'), lh = size * 1.02, y0 = h / 2 - ((lines.length - 1) * lh) / 2 + size * 0.06;
    lines.forEach((l, i) => { g.strokeText(l, w / 2, y0 + i * lh); });
    g.fillStyle = fg;
    lines.forEach((l, i) => { g.fillText(l, w / 2, y0 + i * lh); });
  });
}

function buildBuilding() {
  const trim = toon({ color: 0x3b2114 });
  const wains = toon({ color: 0xf2e3bf });

  // floor + ceiling
  const floorTex = tileTexture();
  floorTex.repeat.set(HALF_W, LEN / 2);
  const floor = new THREE.Mesh(new THREE.PlaneGeometry(HALF_W * 2, LEN), toon({ map: floorTex }));
  floor.rotation.x = -Math.PI / 2;
  floor.position.set(0, 0, -LEN / 2);
  floor.receiveShadow = true;
  scene.add(floor);
  const ceil = new THREE.Mesh(new THREE.PlaneGeometry(HALF_W * 2, LEN), toon({ color: 0x2a1a12 }));
  ceil.rotation.x = Math.PI / 2;
  ceil.position.set(0, H, -LEN / 2);
  scene.add(ceil);

  for (let r = 0; r < ROOMS; r++) {
    const z0 = -r * ROOM_D, zc = z0 - ROOM_D / 2;
    const wall = toon({ color: ROOM_COLORS[r] });
    // side walls, one box per room so each room gets its colour
    for (const s of [-1, 1]) {
      box(s * (HALF_W + T / 2), H / 2, zc, T, H, ROOM_D, wall);
      box(s * (HALF_W - 0.04), 0.6, zc, 0.08, 1.2, ROOM_D, wains, false);       // wainscot
      box(s * (HALF_W - 0.08), 1.22, zc, 0.16, 0.1, ROOM_D, trim, false);       // chair rail
      box(s * (HALF_W - 0.05), 0.09, zc, 0.12, 0.18, ROOM_D, trim, false);      // skirting
      box(s * (HALF_W - 0.1), H - 0.15, zc, 0.2, 0.3, ROOM_D, trim, false);     // cornice
    }
    // skylight framed by beams
    const sky = new THREE.Mesh(new THREE.PlaneGeometry(6, 11), new THREE.MeshBasicMaterial({ color: 0xfff3cf }));
    sky.rotation.x = Math.PI / 2;
    sky.position.set(0, H - 0.02, zc);
    scene.add(sky);
    for (const dz of [-5.5, 5.5]) box(0, H - 0.2, zc + dz, 6.4, 0.4, 0.4, trim, false);
    for (const dx of [-3, 0, 3]) box(dx, H - 0.2, zc, 0.4, 0.4, 11, trim, false);
    for (const dz of [-8, 8]) box(0, H - 0.25, zc + dz, HALF_W * 2, 0.5, 0.5, trim, false);

    // bench in the middle of the room
    const seat = box(0, 0.5, zc, 3.2, 0.14, 0.9, toon({ color: 0x6b3a1f }));
    hullFor(seat, 0.03);
    for (const dx of [-1.3, 1.3]) box(dx, 0.22, zc, 0.2, 0.44, 0.7, trim, false);

    // plants in the far corners
    for (const s of [-1, 1]) plant(s * (HALF_W - 0.8), z0 - ROOM_D + 0.9);

    // a light in each room so walls get toon bands, not flat fill
    const lamp = new THREE.PointLight(0xffe2b0, 40, 22, 1.6);
    lamp.position.set(0, H - 1.2, zc);
    scene.add(lamp);
  }

  // end walls
  box(0, H / 2, T / 2, HALF_W * 2 + T * 2, H, T, toon({ color: ROOM_COLORS[0] }));
  box(0, H / 2, -LEN - T / 2, HALF_W * 2 + T * 2, H, T, toon({ color: ROOM_COLORS[ROOMS - 1] }));
  for (const [z, s] of [[0, -1], [-LEN, 1]]) {
    box(0, 0.6, z + s * 0.04, HALF_W * 2, 1.2, 0.08, wains, false);
    box(0, 1.22, z + s * 0.08, HALF_W * 2, 0.1, 0.16, trim, false);
  }

  // partitions with a doorway, each face painted in its own room's colour
  const segW = HALF_W - DOOR / 2;
  for (let k = 1; k < ROOMS; k++) {
    const z = -k * ROOM_D;
    const south = toon({ color: ROOM_COLORS[k - 1] }), north = toon({ color: ROOM_COLORS[k] });
    const faces = [trim, trim, trim, trim, south, north];
    for (const s of [-1, 1]) box(s * (DOOR / 2 + segW / 2), H / 2, z, segW, H, T, faces);
    box(0, (DOOR_H + H) / 2, z, DOOR, H - DOOR_H, T, faces);
    // door frame posts + lintel trim
    for (const s of [-1, 1]) hullFor(box(s * (DOOR / 2 + 0.1), DOOR_H / 2, z, 0.2, DOOR_H, T + 0.16, trim, false), 0.025);
    hullFor(box(0, DOOR_H + 0.1, z, DOOR + 0.4, 0.2, T + 0.16, trim, false), 0.025);
    // room signs above the doorway, one on each side
    for (const [side, label] of [[1, `ROOM ${k + 1}`], [-1, `ROOM ${k}`]]) {
      const sign = new THREE.Mesh(new THREE.PlaneGeometry(2.6, 0.9), toon({ map: signTex(label, 520, 180, '#ffd23f', '#ff7b1c', 120) }));
      sign.position.set(0, DOOR_H + 0.85, z + side * (T / 2 + 0.01));
      if (side < 0) sign.rotation.y = Math.PI;
      scene.add(sign);
    }
  }

  // title banner hanging in room 1
  const banner = new THREE.Mesh(new THREE.BoxGeometry(8.4, 1.9, 0.08), [trim, trim, trim, trim,
    toon({ map: signTex('THE IMAGE PROCESSING\n2019 C++, RUNNING IN YOUR BROWSER', 1680, 380, '#fff4d6', '#e63946', 130) }), trim]);
  banner.position.set(0, 4.35, -ROOM_D + 4.5);
  hullFor(banner, 0.05);
  scene.add(shadowed(banner));
  for (const dx of [-3.6, 3.6]) box(dx, (H + 5.3) / 2, banner.position.z, 0.04, H - 5.3, 0.04, trim, false);

  // the way out: a door on the entrance wall that leads to the 2D site
  const door = box(0, 1.5, -0.08, 2.2, 3, 0.16, toon({ color: 0x6b3a1f }), false);
  hullFor(door, 0.04);
  door.userData.door = true;
  solids.push(door);
  const exit = new THREE.Mesh(new THREE.PlaneGeometry(2.4, 0.7), toon({ map: signTex('2D MUSEUM', 480, 140, '#13b6c9', '#fff4d6', 96) }));
  exit.position.set(0, 3.45, -0.03);
  exit.rotation.y = Math.PI; // the entrance wall faces -z, into the building
  scene.add(exit);
}

function plant(x, z) {
  const g = new THREE.Group();
  const pot = new THREE.Mesh(new THREE.CylinderGeometry(0.42, 0.3, 0.7, 10), toon({ color: 0xc8553d }));
  pot.position.y = 0.35;
  const leaves = new THREE.Mesh(new THREE.IcosahedronGeometry(0.75, 0), toon({ color: 0x3fa34d }));
  leaves.position.y = 1.35; leaves.scale.y = 1.3;
  const top = new THREE.Mesh(new THREE.IcosahedronGeometry(0.5, 0), toon({ color: 0x5cc25f }));
  top.position.y = 2.1;
  g.add(pot, leaves, top);
  g.position.set(x, 0, z);
  scene.add(shadowed(g));
  boxes.push({ x0: x - 0.5, x1: x + 0.5, z0: z - 0.5, z1: z + 0.5, y0: 0, y1: 2.6 });
}

// ---------------------------------------------------------------- exhibits
const loader = new THREE.TextureLoader();
const placeholder = canvasTex(64, 64, (g) => { g.fillStyle = '#3a2a22'; g.fillRect(0, 0, 64, 64); });
const glowTex = canvasTex(256, 256, (g) => {
  const r = g.createRadialGradient(128, 110, 10, 128, 128, 128);
  r.addColorStop(0, 'rgba(255,232,170,0.55)'); r.addColorStop(1, 'rgba(255,232,170,0)');
  g.fillStyle = r; g.fillRect(0, 0, 256, 256);
});
const photos = [];

function wrapLines(g, text, maxW) {
  const out = []; let line = '';
  for (const w of text.split(' ')) {
    const t = line ? line + ' ' + w : w;
    if (g.measureText(t).width > maxW && line) { out.push(line); line = w; } else line = t;
  }
  if (line) out.push(line);
  return out;
}

function placardTex(ex) {
  return canvasTex(640, 400, (g, w, h) => {
    g.fillStyle = '#fff4d6'; g.fillRect(0, 0, w, h);
    g.lineWidth = 12; g.strokeStyle = '#140c0a'; g.strokeRect(6, 6, w - 12, h - 12);
    g.fillStyle = '#140c0a';
    g.font = `58px ${FONT}`;
    let y = 76;
    for (const l of wrapLines(g, ex.title.toUpperCase(), w - 70).slice(0, 3)) { g.fillText(l, 34, y); y += 58; }
    g.font = '600 30px Inter, sans-serif';
    g.fillText(`${ex.photographer}, ${ex.year}`, 34, y + 12);
    g.font = '26px Inter, sans-serif';
    const badge = ex.rights || 'Public domain';
    const bw = g.measureText(badge).width + 28;
    g.lineWidth = 4; g.strokeRect(34, h - 84, bw, 46);
    g.fillText(badge, 48, h - 52);
    g.fillStyle = '#b3262e'; g.font = `40px ${FONT}`; g.textAlign = 'right';
    g.fillText(TOUCH ? 'TAP TO OPEN' : 'E TO OPEN', w - 34, h - 48);
  });
}

function hangPhoto(ex, pos, normal) {
  const s = Math.min(3.0 / ex.w, 2.3 / ex.h);
  const w = ex.w * s, h = ex.h * s;
  const group = new THREE.Group();
  group.position.copy(pos);
  group.rotation.y = Math.atan2(normal.x, normal.z); // local +z faces into the room

  const frameMat = toon({ color: 0x2b1a10 });
  const frame = new THREE.Mesh(new THREE.BoxGeometry(w + 0.42, h + 0.42, 0.12), frameMat);
  frame.position.z = 0.06;
  const hullMat = inkMat.clone();
  const hull = hullFor(frame, 0.045, hullMat);
  const matBoard = new THREE.Mesh(new THREE.PlaneGeometry(w + 0.2, h + 0.2), toon({ color: 0xf6ecd4 }));
  matBoard.position.z = 0.127; // clear of the frame's front face (0.12) to avoid z-fighting
  const material = artMat(placeholder);
  const pic = new THREE.Mesh(new THREE.PlaneGeometry(w, h), material);
  pic.position.z = 0.135;

  // picture lamp above the frame, and the pool of light it throws
  const arm = new THREE.Mesh(new THREE.BoxGeometry(0.06, 0.06, 0.38), toon({ color: 0xc9a227 }));
  arm.position.set(0, h / 2 + 0.36, 0.2);
  const shade = new THREE.Mesh(new THREE.CylinderGeometry(0.11, 0.11, Math.min(w * 0.6, 1.4), 10), toon({ color: 0xc9a227 }));
  shade.rotation.z = Math.PI / 2;
  shade.position.set(0, h / 2 + 0.36, 0.4);
  hullFor(arm, 0.02);
  const glow = new THREE.Mesh(new THREE.PlaneGeometry(w + 2.2, h + 2.4),
    new THREE.MeshBasicMaterial({ map: glowTex, transparent: true, blending: THREE.AdditiveBlending, depthWrite: false }));
  glow.position.set(0, 0.25, 0.01);

  const plaque = new THREE.Mesh(new THREE.PlaneGeometry(0.72, 0.45), artMat(placardTex(ex)));
  plaque.position.set(w / 2 + 0.25 + 0.42, -h / 2 + 0.55, 0.03);

  group.add(frame, matBoard, pic, arm, shade, glow, plaque);
  shadowed(frame); shadowed(arm); shadowed(shade);
  scene.add(group);

  const p = { ex, group, frame, pic, material, hull, w, h, normal: normal.clone(), center: pos.clone(), thumb: null, loading: false };
  frame.userData.photo = pic.userData.photo = plaque.userData.photo = p;
  solids.push(frame, pic, plaque);
  photos.push(p);
}

function hangAll() {
  // per room: 3 photos on each side wall; whatever is left goes on the far end wall
  let i = 0;
  for (let r = 0; r < ROOMS && i < EXHIBITS.length; r++) {
    for (const side of [-1, 1]) {
      for (const dz of [4, 9, 14]) {
        if (i >= EXHIBITS.length) break;
        hangPhoto(EXHIBITS[i++], new THREE.Vector3(side * HALF_W, 2.45, -r * ROOM_D - dz), new THREE.Vector3(-side, 0, 0));
      }
    }
  }
  const rest = EXHIBITS.slice(i);
  rest.forEach((ex, k) => {
    const x = (k - (rest.length - 1) / 2) * 4.6;
    hangPhoto(ex, new THREE.Vector3(x, 2.45, -LEN), new THREE.Vector3(0, 0, 1));
  });
}

// thumbnails load only once you get near a photo
function lazyLoad() {
  for (const p of photos) {
    if (p.loading || camera.position.distanceTo(p.center) > 26) continue;
    p.loading = true;
    loader.load(p.ex.thumb, (t) => {
      t.colorSpace = THREE.SRGBColorSpace;
      t.anisotropy = MAX_ANISO;
      p.thumb = t;
      if (p.material.uniforms.mapA.value === placeholder) setPicture(p, t);
    });
  }
}

function setPicture(p, tex) {
  const u = p.material.uniforms;
  u.mapA.value = u.mapB.value = tex;
  u.prog.value = 0;
}

// comic ink-blot wipe from whatever is shown to `tex`
function wipeTo(p, tex, dur = 750) {
  const u = p.material.uniforms;
  if (u.prog.value > 0) u.mapA.value = u.mapB.value; // finish a wipe already running
  u.mapB.value = tex;
  return animate(dur, (k) => { u.prog.value = k; }).then(() => {
    if (u.mapB.value === tex) setPicture(p, tex);
  });
}

// ---------------------------------------------------------------- lights
function buildLights() {
  scene.add(new THREE.AmbientLight(0xffffff, 0.75));
  scene.add(new THREE.HemisphereLight(0xfff1d6, 0x5a2e1c, 0.6));
  const sun = new THREE.DirectionalLight(0xfff0d0, 2.2);
  sun.position.set(6, 18, -LEN / 2 + 7);
  sun.target.position.set(0, 0, -LEN / 2);
  scene.add(sun, sun.target);
  sun.castShadow = renderer.shadowMap.enabled;
  Object.assign(sun.shadow.camera, { left: -LEN / 2 - 4, right: LEN / 2 + 4, top: 12, bottom: -12, near: 1, far: 50 });
  sun.shadow.mapSize.set(2048, 2048);
  sun.shadow.bias = -0.0008;
}

// ---------------------------------------------------------------- post: ink + hatching
// Pass 1 renders colour+depth, pass 2 renders view-space normals. The final shader
// draws ink wherever depth or normals jump (silhouettes, creases) and hatches dark areas.
const SAMPLES = TOUCH ? 0 : 4;
const rtColor = new THREE.WebGLRenderTarget(1, 1, { samples: SAMPLES, depthTexture: new THREE.DepthTexture(1, 1) });
const rtNormal = new THREE.WebGLRenderTarget(1, 1, { samples: SAMPLES });
const normalMat = new THREE.MeshNormalMaterial();
const post = new THREE.ShaderMaterial({
  uniforms: {
    tColor: { value: rtColor.texture },
    tDepth: { value: rtColor.depthTexture },
    tNormal: { value: rtNormal.texture },
    texel: { value: new THREE.Vector2() },
    near: { value: camera.near },
    far: { value: camera.far },
    pr: { value: 1 },
  },
  vertexShader: 'varying vec2 vUv; void main() { vUv = uv; gl_Position = vec4(position.xy, 0.0, 1.0); }',
  fragmentShader: /* glsl */ `
    #include <packing>
    uniform sampler2D tColor, tDepth, tNormal;
    uniform vec2 texel;
    uniform float near, far, pr;
    varying vec2 vUv;
    float viewZ(vec2 uv) { return -perspectiveDepthToViewZ(texture2D(tDepth, uv).x, near, far); }
    vec3 nrm(vec2 uv) { return texture2D(tNormal, uv).xyz * 2.0 - 1.0; }
    float hatch(vec2 p, float dir) {
      float d = abs(fract((p.x + dir * p.y) / 6.0) - 0.5) * 6.0;
      return 1.0 - smoothstep(0.55, 1.3, d);
    }
    void main() {
      vec2 o = texel * 1.5 * pr;
      vec2 ox = vec2(o.x, 0.0), oy = vec2(0.0, o.y);
      float d = viewZ(vUv);
      // Laplacian of depth: zero on flat or tilted planes, spikes at silhouettes
      float lap = abs(viewZ(vUv - ox) + viewZ(vUv + ox) + viewZ(vUv - oy) + viewZ(vUv + oy) - 4.0 * d) / d;
      float depthEdge = smoothstep(0.02, 0.06, lap);
      vec3 n = nrm(vUv);
      float nd = distance(n, nrm(vUv - ox)) + distance(n, nrm(vUv + ox)) + distance(n, nrm(vUv - oy)) + distance(n, nrm(vUv + oy));
      float normalEdge = smoothstep(0.5, 0.9, nd);
      float edge = max(depthEdge, normalEdge) * smoothstep(60.0, 25.0, d); // far lines fade out

      vec4 c = texture2D(tColor, vUv);
      vec3 col = linearToOutputTexel(vec4(c.rgb, 1.0)).rgb;
      float art = 1.0 - smoothstep(0.3, 0.7, c.a); // photos and placards: leave their pixels alone
      float l = dot(col, vec3(0.299, 0.587, 0.114));
      vec3 punchy = clamp(mix(vec3(l), col, 1.3), 0.0, 1.0);

      vec2 p = gl_FragCoord.xy / pr;
      float h = smoothstep(0.4, 0.22, l) * hatch(p, 1.0);
      h = max(h, smoothstep(0.22, 0.08, l) * hatch(p, -1.0));
      punchy *= 1.0 - 0.5 * h;
      col = mix(punchy, col, art);

      float v = length((vUv - 0.5) * vec2(1.1, 1.0));
      col *= mix(1.0, 0.5, smoothstep(0.4, 0.9, v));
      col = mix(col, vec3(0.08, 0.05, 0.04), edge);
      gl_FragColor = vec4(col, 1.0);
    }`,
});
const postScene = new THREE.Scene();
postScene.add(new THREE.Mesh(new THREE.PlaneGeometry(2, 2), post));
const postCam = new THREE.OrthographicCamera();

function render() {
  renderer.setRenderTarget(rtColor);
  renderer.render(scene, camera);

  scene.overrideMaterial = normalMat;
  camera.layers.disable(1);
  renderer.setRenderTarget(rtNormal);
  renderer.render(scene, camera);
  camera.layers.enable(1);
  scene.overrideMaterial = null;

  renderer.setRenderTarget(null);
  renderer.render(postScene, postCam);
}

let viewW = 1, viewH = 1, maxPr = TOUCH ? 1 : 1.5;
function resize() {
  const w = innerWidth, h = innerHeight, pr = Math.min(devicePixelRatio, maxPr);
  viewW = w; viewH = h;
  renderer.setPixelRatio(pr);
  renderer.setSize(w, h, false);
  rtColor.setSize(w * pr, h * pr);
  rtNormal.setSize(w * pr, h * pr);
  post.uniforms.texel.value.set(1 / (w * pr), 1 / (h * pr));
  post.uniforms.pr.value = pr;
  camera.aspect = w / h;
  applyViewOffset(viewOffset.x, viewOffset.y);
}

// Shift the picture sideways/up so the exhibit sits in the space the panel leaves free.
const viewOffset = { x: 0, y: 0 };
function applyViewOffset(x, y) {
  viewOffset.x = x; viewOffset.y = y;
  if (x || y) camera.setViewOffset(viewW, viewH, x, y, viewW, viewH);
  else camera.clearViewOffset();
  camera.updateProjectionMatrix();
}

// ---------------------------------------------------------------- tweening
const tweens = [];
const animate = (dur, step) => new Promise((done) => tweens.push({ t0: performance.now(), dur, step, done }));
const easeInOut = (k) => (k < 0.5 ? 4 * k * k * k : 1 - (-2 * k + 2) ** 3 / 2);
function runTweens(now) {
  for (let i = tweens.length - 1; i >= 0; i--) {
    const t = tweens[i], k = Math.min(1, (now - t.t0) / t.dur);
    t.step(k);
    if (k >= 1) { tweens.splice(i, 1); t.done(); }
  }
}

// ---------------------------------------------------------------- player
const EYE = 1.65, BODY = 1.8, RADIUS = 0.35, REACH = 5.5;
const JUMP_V = 5.2, GRAVITY = 15; // ~0.9 m jump: clears the benches
// pos.y = feet height (0 on the floor, more on a bench or mid-jump)
const player = { pos: new THREE.Vector3(0, 0, -2.6), vel: new THREE.Vector3(), vy: 0, grounded: true, yaw: 0, pitch: -0.04, bob: 0 };
let mode = 'intro'; // intro | walk | tween | exhibit
const keys = new Set();
const stick = { id: null, x0: 0, y0: 0, x: 0, y: 0 };

function collide(p) {
  for (let it = 0; it < 2; it++) {
    for (const b of boxes) {
      // only things at body height block: not the lintel overhead, not a bench we're standing on
      if (p.y >= b.y1 - 0.01 || p.y + BODY <= b.y0) continue;
      const cx = Math.max(b.x0, Math.min(p.x, b.x1)), cz = Math.max(b.z0, Math.min(p.z, b.z1));
      const dx = p.x - cx, dz = p.z - cz, d2 = dx * dx + dz * dz;
      if (d2 >= RADIUS * RADIUS) continue;
      if (d2 > 1e-9) {
        const d = Math.sqrt(d2), k = (RADIUS - d) / d;
        p.x += dx * k; p.z += dz * k;
      } else { // centre inside the box: leave by the nearest side
        const outs = [[b.x0 - RADIUS - p.x, 0], [b.x1 + RADIUS - p.x, 0], [0, b.z0 - RADIUS - p.z], [0, b.z1 + RADIUS - p.z]];
        outs.sort((a, c) => Math.hypot(...a) - Math.hypot(...c));
        p.x += outs[0][0]; p.z += outs[0][1];
      }
    }
  }
}

function walk(dt) {
  const k = (c) => (keys.has(c) ? 1 : 0);
  let f = k('KeyW') + k('ArrowUp') - k('KeyS') - k('ArrowDown') - stick.y;
  let s = k('KeyD') + k('ArrowRight') - k('KeyA') - k('ArrowLeft') + stick.x;
  const len = Math.hypot(f, s);
  if (len > 1) { f /= len; s /= len; }
  const speed = keys.has('ShiftLeft') || keys.has('ShiftRight') ? 7 : 4;
  const sin = Math.sin(player.yaw), cos = Math.cos(player.yaw);
  const want = new THREE.Vector3((-sin * f + cos * s) * speed, 0, (-cos * f - sin * s) * speed);
  player.vel.lerp(want, 1 - Math.exp(-dt * 10));
  player.pos.addScaledVector(player.vel, dt);
  collide(player.pos);
  // gravity: fall to the highest surface under our feet (floor or a bench top)
  const ground = groundAt(player.pos); // measured before falling, so a fast drop can't skip a bench top
  player.vy -= GRAVITY * dt;
  player.pos.y += player.vy * dt;
  player.grounded = player.pos.y <= ground;
  if (player.grounded) { player.pos.y = ground; player.vy = 0; }
  const v = player.grounded ? player.vel.length() : 0; // no head bob mid-air
  player.bob += dt * v * 2.1;
  placeCamera(Math.sin(player.bob) * 0.035 * Math.min(1, v / 4));
}

function groundAt(p) {
  let g = 0;
  for (const b of boxes) {
    if (p.x >= b.x0 && p.x <= b.x1 && p.z >= b.z0 && p.z <= b.z1 && b.y1 <= p.y + 0.05) g = Math.max(g, b.y1);
  }
  return g;
}

function jump() {
  if (player.grounded) { player.vy = JUMP_V; player.grounded = false; }
}

function placeCamera(bob = 0) {
  camera.position.set(player.pos.x, player.pos.y + EYE + bob, player.pos.z);
  camera.rotation.set(player.pitch, player.yaw, 0);
}

function look(dx, dy) {
  player.yaw -= dx;
  player.pitch = Math.max(-1.2, Math.min(1.2, player.pitch - dy));
}

// ---------------------------------------------------------------- aiming
const ray = new THREE.Raycaster();
ray.far = REACH;
let target = null;

function pick(ndcX, ndcY) {
  ray.setFromCamera(new THREE.Vector2(ndcX, ndcY), camera);
  const hit = ray.intersectObjects(solids, false)[0];
  if (!hit) return null;
  return hit.object.userData.photo || (hit.object.userData.door ? 'door' : null);
}

function aim() {
  const t = mode === 'walk' && !TOUCH ? pick(0, 0) : null;
  if (t === target) return;
  if (target && target !== 'door') target.hull.material.color.setHex(INK);
  target = t;
  if (target && target !== 'door') target.hull.material.color.setHex(0xffd23f);
  $('#cross').classList.toggle('hot', !!target);
  const pr = $('#prompt');
  pr.hidden = !target;
  if (target === 'door') pr.textContent = 'E · Go to the 2D museum';
  else if (target) pr.textContent = `E · Open "${target.ex.title}"`;
}

function interact(t) {
  if (t === 'door') { if (document.pointerLockElement) document.exitPointerLock(); location.href = './'; }
  else if (t) openExhibit(t);
}

// ---------------------------------------------------------------- exhibit mode
const cur = { p: null, src: null, srcTex: null, resultTex: null, op: null, threshold: 110, busy: false, pending: false, debounce: 0, showingBefore: false, saved: null };
const lookAtHelper = new THREE.Camera(); // camera-style lookAt: -z faces the target

function loadImage(url, cap) {
  return new Promise((resolve, reject) => {
    const img = new Image();
    img.onload = () => {
      let w = img.naturalWidth, h = img.naturalHeight;
      if (Math.max(w, h) > cap) { const s = cap / Math.max(w, h); w = Math.round(w * s); h = Math.round(h * s); }
      const c = el('canvas'); c.width = w; c.height = h;
      const g = c.getContext('2d', { willReadFrequently: true });
      g.drawImage(img, 0, 0, w, h);
      resolve(g.getImageData(0, 0, w, h));
    };
    img.onerror = reject;
    img.src = url;
  });
}

// Engine results can be square crops or tiny spectra: letterbox them into the frame's shape.
function frameTex(imgData, p) {
  const long = 1024, ar = p.w / p.h;
  const cw = ar >= 1 ? long : Math.round(long * ar), ch = ar >= 1 ? Math.round(long / ar) : long;
  const src = el('canvas'); src.width = imgData.width; src.height = imgData.height;
  src.getContext('2d').putImageData(imgData instanceof ImageData ? imgData : new ImageData(new Uint8ClampedArray(imgData.data), imgData.width, imgData.height), 0, 0);
  return canvasTex(cw, ch, (g) => {
    g.fillStyle = '#0b0b0d'; g.fillRect(0, 0, cw, ch);
    const s = Math.min(cw / src.width, ch / src.height), w = src.width * s, h = src.height * s;
    g.imageSmoothingQuality = 'high';
    g.drawImage(src, (cw - w) / 2, (ch - h) / 2, w, h);
  });
}

function panelSize() {
  const pnl = $('#panel');
  const bottom = matchMedia('(max-width: 700px), (max-aspect-ratio: 1/1)').matches;
  return bottom ? { x: 0, y: pnl.offsetHeight } : { x: pnl.offsetWidth, y: 0 };
}

function exhibitPose(p) {
  const ps = panelSize();
  const tan = Math.tan(THREE.MathUtils.degToRad(camera.fov / 2));
  const freeW = viewW - ps.x, freeH = viewH - ps.y;
  const dH = (p.h * 0.8) / tan * (viewH / freeH);
  const dW = (p.w * 0.8) / (tan * camera.aspect) * (viewW / freeW);
  const pos = p.center.clone().addScaledVector(p.normal, Math.max(1.5, dH, dW) + 0.135);
  lookAtHelper.position.copy(pos);
  lookAtHelper.lookAt(p.center.clone().addScaledVector(p.normal, 0.135));
  return { pos, quat: lookAtHelper.quaternion.clone(), offset: { x: ps.x / 2, y: ps.y / 2 } };
}

async function flyTo(pos, quat, offset, dur = 900) {
  const p0 = camera.position.clone(), q0 = camera.quaternion.clone();
  const o0 = { ...viewOffset };
  await animate(dur, (k) => {
    const e = easeInOut(k);
    camera.position.lerpVectors(p0, pos, e);
    camera.quaternion.slerpQuaternions(q0, quat, e);
    applyViewOffset(o0.x + (offset.x - o0.x) * e, o0.y + (offset.y - o0.y) * e);
  });
}

async function openExhibit(p) {
  if (mode !== 'walk') return;
  mode = 'tween';
  aimOff();
  $('#jump').hidden = true;
  if (document.pointerLockElement) document.exitPointerLock();
  cur.p = p; cur.op = null; cur.src = null; cur.showingBefore = false;
  cur.saved = { yaw: player.yaw, pitch: player.pitch };
  fillPanel(p.ex);
  const pnl = $('#panel');
  pnl.hidden = false;
  pnl.scrollTop = 0;
  requestAnimationFrame(() => pnl.classList.remove('off'));
  const pose = exhibitPose(p);
  const loading = loadImage(p.ex.img, 1500);
  await flyTo(pose.pos, pose.quat, pose.offset);
  mode = 'exhibit';
  try {
    cur.src = await loading;
    if (cur.p !== p) return;
    cur.srcTex?.dispose();
    cur.srcTex = frameTex(cur.src, p);
    wipeTo(p, cur.srcTex, 500);
    document.querySelectorAll('#ops button').forEach((b) => { b.disabled = false; });
  } catch {
    $('#readout').textContent = 'could not load this photograph';
  }
}

async function closeExhibit() {
  if (mode !== 'exhibit') return;
  mode = 'tween';
  $('#panel').classList.add('off');
  $('#prompt').hidden = true;
  lookAtHelper.position.set(player.pos.x, player.pos.y + EYE, player.pos.z);
  lookAtHelper.rotation.set(cur.saved.pitch, cur.saved.yaw, 0, 'YXZ');
  await flyTo(new THREE.Vector3(player.pos.x, player.pos.y + EYE, player.pos.z), lookAtHelper.quaternion.clone(), { x: 0, y: 0 }, 750);
  $('#panel').hidden = true;
  player.yaw = cur.saved.yaw; player.pitch = cur.saved.pitch;
  placeCamera();
  cur.p = null;
  mode = 'walk';
  $('#cross').hidden = TOUCH;
  $('#jump').hidden = !TOUCH;
}

function aimOff() {
  if (target && target !== 'door') target.hull.material.color.setHex(INK);
  target = null;
  $('#cross').hidden = true;
  $('#prompt').hidden = true;
}

function fillPanel(ex) {
  $('#pTitle').textContent = ex.title;
  $('#pBy').textContent = `${ex.photographer}, ${ex.year}`;
  $('#pBadge').textContent = ex.rights || 'Public domain';
  $('#pPlacard').textContent = ex.placard;
  const prov = $('#pProv');
  prov.textContent = ex.collection + ' ';
  if (ex.commonsPageUrl) {
    const a = el('a', null, 'Wikimedia Commons ↗');
    a.href = ex.commonsPageUrl; a.target = '_blank'; a.rel = 'noopener';
    prov.appendChild(a);
  }
  $('#readout').textContent = '$ pick an operation';
  $('#extra').textContent = '';
  $('#lTitle').textContent = '';
  $('#lBody').textContent = 'Each button runs the original C++ program on this photograph and hangs the result in the frame.';
  $('#params').textContent = '';
  $('#ba').disabled = true;
  $('#ba').textContent = 'Show before';
  const ops = $('#ops');
  ops.textContent = '';
  for (const cat of OPS.CATEGORIES) {
    ops.appendChild(el('h3', null, cat.name));
    const row = el('div', 'ops');
    for (const op of cat.ops) {
      const b = el('button', null, op.label);
      b.dataset.op = op.id;
      b.disabled = true; // until the full-size photo has loaded
      b.addEventListener('click', () => selectOp(op));
      row.appendChild(b);
    }
    ops.appendChild(row);
  }
}

function selectOp(op) {
  cur.op = op;
  document.querySelectorAll('#ops button').forEach((b) => b.classList.toggle('on', b.dataset.op === op.id));
  $('#lTitle').textContent = op.label;
  $('#lBody').textContent = OPS.LESSONS[op.id] || op.blurb || '';
  const wrap = $('#params');
  wrap.textContent = '';
  const sliders = [];
  if (op.work && op.work.binary) sliders.push({ id: '__thr', label: 'binarise threshold', min: 40, max: 210, step: 5, def: cur.threshold });
  sliders.push(...(op.params || []));
  for (const p of sliders) {
    const row = el('div', 'param');
    const lab = el('label', null, p.label);
    const val = el('span', null, String(p.def));
    lab.appendChild(val);
    const input = el('input');
    Object.assign(input, { type: 'range', min: p.min, max: p.max, step: p.step, value: p.def });
    input.dataset.pid = p.id;
    input.addEventListener('input', () => {
      val.textContent = input.value;
      if (p.id === '__thr') cur.threshold = +input.value;
      clearTimeout(cur.debounce);
      cur.debounce = setTimeout(() => apply(false), 160);
    });
    row.append(lab, input);
    wrap.appendChild(row);
  }
  apply(true);
}

function params() {
  const out = {};
  document.querySelectorAll('#params input').forEach((i) => { if (i.dataset.pid !== '__thr') out[i.dataset.pid] = +i.value; });
  return out;
}

const nextFrame = () => new Promise((r) => requestAnimationFrame(() => requestAnimationFrame(r)));

async function apply(loud) {
  if (!cur.op || !cur.src) return;
  if (cur.busy) { cur.pending = true; return; }
  cur.busy = true;
  const p = cur.p, op = cur.op;
  if (loud) { popup('Processing…', 'wait'); await nextFrame(); }
  try {
    const r = await Engine.runOp(op, cur.src, params(), cur.threshold);
    if (cur.p !== p) return;
    const old = cur.resultTex;
    cur.resultTex = frameTex(r.result, p);
    cur.showingBefore = false;
    $('#ba').disabled = false;
    $('#ba').textContent = 'Show before';
    $('#readout').textContent = `$ ${r.command.replace(/\/(input|i)\.bmp/g, '$1.bmp')}\n# ${r.workW}×${r.workH} px · ${r.ms.toFixed(0)} ms`;
    if (loud) popup(op.label + '!', 'good'); else popup('', '');
    await wipeTo(p, cur.resultTex, loud ? 750 : 300);
    if (old && old !== p.material.uniforms.mapA.value) old.dispose();
  } catch (e) {
    $('#readout').textContent = 'error: ' + (e && e.message ? e.message : e);
    popup('Error', 'bad');
    console.error(e);
  } finally {
    cur.busy = false;
    if (cur.pending) { cur.pending = false; apply(false); }
  }
}

function toggleBefore() {
  if (!cur.resultTex || !cur.srcTex) return;
  cur.showingBefore = !cur.showingBefore;
  $('#ba').textContent = cur.showingBefore ? 'Show after' : 'Show before';
  wipeTo(cur.p, cur.showingBefore ? cur.srcTex : cur.resultTex, 450);
}

async function showStats() {
  if (!cur.src) return;
  const x = $('#extra');
  x.textContent = '';
  const pre = el('pre', null, 'measuring…');
  x.appendChild(pre);
  const rows = await Engine.runStats(cur.src);
  pre.textContent = rows.map((r) => `${r.label}\n${r.text}`).join('\n\n');
}

async function showHistogram() {
  if (!cur.src) return;
  const x = $('#extra');
  x.textContent = '';
  for (const ch of OPS.HISTOGRAM.channels) {
    const r = await Engine.runHistogram(cur.src, ch.v);
    const c = el('canvas');
    c.width = r.width; c.height = r.height;
    c.getContext('2d').putImageData(new ImageData(new Uint8ClampedArray(r.data), r.width, r.height), 0, 0);
    c.title = ch.label;
    x.appendChild(c);
  }
}

let popTimer = 0;
function popup(text, kind) {
  const p = $('#popup');
  clearTimeout(popTimer);
  p.className = '';
  void p.offsetWidth; // restart the CSS animation
  p.textContent = text;
  if (!kind) return;
  p.classList.add(kind);
  if (kind !== 'wait') popTimer = setTimeout(() => { p.className = ''; }, 1150);
}

// ---------------------------------------------------------------- input
let hadLock = false, lockedAt = 0;
function lockPointer() {
  if (TOUCH) return;
  try { canvas.requestPointerLock()?.catch?.(() => {}); } catch { /* no pointer lock: drag to look instead */ }
}

document.addEventListener('pointerlockchange', () => {
  const locked = document.pointerLockElement === canvas;
  if (locked) { hadLock = true; lockedAt = performance.now(); $('#paused').hidden = true; }
  else if (mode === 'walk' && hadLock) $('#paused').hidden = false;
});

addEventListener('keydown', (e) => {
  if (e.target.tagName === 'INPUT') return;
  keys.add(e.code);
  if (mode === 'walk' && e.code === 'KeyE') interact(target);
  if (mode === 'walk' && e.code === 'Space') { e.preventDefault(); if (!e.repeat) jump(); }
  if (mode === 'exhibit' && (e.code === 'Escape' || e.code === 'KeyQ')) closeExhibit();
  if (mode === 'exhibit' && e.code === 'KeyB') toggleBefore();
  if (e.code.startsWith('Arrow') && mode === 'walk') e.preventDefault();
});
addEventListener('keyup', (e) => keys.delete(e.code));
addEventListener('blur', () => keys.clear());

// One pointer handler for mouse and touch. Mouse: pointer lock, or drag to look if
// the lock is unavailable. Touch: left third is a joystick, the rest drags to look,
// and a quick tap opens whatever photo is under the finger.
const drags = new Map();
canvas.addEventListener('pointerdown', (e) => {
  if (mode !== 'walk') return;
  if (e.pointerType === 'touch' && e.clientX < viewW * 0.35 && stick.id === null) {
    Object.assign(stick, { id: e.pointerId, x0: e.clientX, y0: e.clientY, x: 0, y: 0 });
    return;
  }
  drags.set(e.pointerId, { x: e.clientX, y: e.clientY, sx: e.clientX, sy: e.clientY, t: performance.now() });
});
addEventListener('pointermove', (e) => {
  if (mode !== 'walk') return;
  if (document.pointerLockElement === canvas) {
    // browsers can report one bogus jump right after locking; skip it
    if (performance.now() - lockedAt > 150 && Math.abs(e.movementX) + Math.abs(e.movementY) < 400) look(e.movementX * 0.0022, e.movementY * 0.0022);
    return;
  }
  if (e.pointerId === stick.id) {
    const dx = e.clientX - stick.x0, dy = e.clientY - stick.y0, m = Math.max(1, Math.hypot(dx, dy) / 50);
    stick.x = dx / 50 / m; stick.y = dy / 50 / m;
    $('#knob').style.transform = `translate(${stick.x * 34}px, ${stick.y * 34}px)`;
    return;
  }
  const d = drags.get(e.pointerId);
  if (!d) return;
  const k = e.pointerType === 'touch' ? 0.006 : 0.004;
  look((e.clientX - d.x) * k, (e.clientY - d.y) * k);
  d.x = e.clientX; d.y = e.clientY;
});
const endPointer = (e) => {
  if (e.pointerId === stick.id) { stick.id = null; stick.x = stick.y = 0; $('#knob').style.transform = ''; return; }
  const d = drags.get(e.pointerId);
  drags.delete(e.pointerId);
  if (!d || mode !== 'walk' || e.type === 'pointercancel') return;
  const tap = Math.hypot(e.clientX - d.sx, e.clientY - d.sy) < 8 && performance.now() - d.t < 400;
  if (!tap) return;
  if (document.pointerLockElement === canvas) return interact(target);
  const hit = pick((e.clientX / viewW) * 2 - 1, -(e.clientY / viewH) * 2 + 1);
  if (hit) interact(hit);
  else lockPointer();
};
addEventListener('pointerup', endPointer);
addEventListener('pointercancel', endPointer);

$('#start').addEventListener('click', () => {
  $('#intro').hidden = true;
  mode = 'walk';
  $('#cross').hidden = TOUCH;
  $('#stick').hidden = !TOUCH;
  $('#jump').hidden = !TOUCH;
  lockPointer();
});
// pointerdown, not click: jumps the instant the thumb lands
$('#jump').addEventListener('pointerdown', (e) => { e.preventDefault(); if (mode === 'walk') jump(); });
$('#resume').addEventListener('click', () => { $('#paused').hidden = true; lockPointer(); });
$('#back').addEventListener('click', () => { closeExhibit(); lockPointer(); });
$('#ba').addEventListener('click', toggleBefore);
$('#stats').addEventListener('click', showStats);
$('#hist').addEventListener('click', showHistogram);
if (TOUCH) $('#keys').textContent = 'Left thumb: walk · Drag: look · JUMP button: jump · Tap a photo to open it';
addEventListener('resize', () => {
  resize();
  if (mode === 'exhibit' && cur.p) { const pose = exhibitPose(cur.p); camera.position.copy(pose.pos); applyViewOffset(pose.offset.x, pose.offset.y); }
});

// ---------------------------------------------------------------- go
buildBuilding();
hangAll();
buildLights();
resize();
placeCamera();
renderer.shadowMap.needsUpdate = true;
$('#start').textContent = 'Enter the museum';
$('#start').disabled = false;

// If a slow GPU can't hold ~45 fps while walking, render at a lower resolution once.
let last = performance.now(), slow = 0, frames = 0;
renderer.setAnimationLoop((now) => {
  const raw = (now - last) / 1000, dt = Math.min(raw, 0.05);
  last = now;
  if (mode === 'walk') {
    walk(dt); aim();
    if (maxPr > 1 && ++frames > 30) { slow = slow * 0.95 + (raw > 1 / 45 ? 0.05 : 0); if (slow > 0.6) { maxPr = 1; resize(); } }
  }
  runTweens(now);
  lazyLoad();
  render();
});

// handy for automated checks
window.museum = { photos, player, get mode() { return mode; }, openExhibit, closeExhibit, selectOp, camera };
