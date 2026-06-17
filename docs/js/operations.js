/* The exhibit's instruments.
 *
 * Each entry maps a museum control to the EXACT command line of the original
 * C++ program (compiled untouched to WASM). `args(p)` builds argv exactly as
 * the program's main() expects it; `out` is the file the program saves to in
 * the in-memory filesystem; `work` constrains the working image so the original
 * code behaves (Fourier wants a square power-of-two greyscale; morphology wants
 * a square binary image; etc.).
 *
 *   IN is the input path in MEMFS. Task4's save() helper derives output names
 *   from the input name, so Task4 ops use a short, fixed input path '/i.bmp'.
 */
(function (root) {
  const IN = '/input.bmp';
  const I4 = '/i.bmp';

  const CATEGORIES = [
    {
      id: 'tone', name: 'Tone & Color', task: 'Task1',
      blurb: 'Point operations — every pixel mapped on its own, the darkroom’s most basic gestures.',
      ops: [
        { id: 'negative', label: 'Negative', task: 'Task1', out: 'newOne.bmp',
          args: () => ['--negative', IN],
          blurb: 'Invert every channel: g = 255 − f. The photographic negative, in one line of arithmetic.' },
        { id: 'brightness', label: 'Brightness', task: 'Task1', out: 'newOne.bmp',
          args: p => ['--brightness', IN, String(p.value | 0)],
          params: [{ id: 'value', label: 'shift', min: -120, max: 120, step: 1, def: 45 }],
          blurb: 'Add a constant to every pixel, clamped to [0,255].' },
        { id: 'contrast', label: 'Contrast', task: 'Task1', out: 'newOne.bmp',
          args: p => ['--contrast', IN, String(p.value)],
          params: [{ id: 'value', label: 'factor', min: 0.2, max: 2.6, step: 0.05, def: 1.5 }],
          blurb: 'Stretch around mid-grey: g = k·(f − 128) + 128.' },
        { id: 'gamma', label: 'Gamma', task: 'Task2', out: 'newOne.bmp',
          args: p => ['--new_contrast', IN, String(p.value)],
          params: [{ id: 'value', label: 'γ', min: 0.3, max: 2.6, step: 0.05, def: 0.55 }],
          blurb: 'Power-law response: g = 255·(f/255)^γ. Below 1 lifts shadows, above 1 deepens them.' },
      ],
    },
    {
      id: 'filter', name: 'Filters', task: 'Task1',
      blurb: 'Neighbourhood operators — each pixel rewritten from the company it keeps.',
      ops: [
        { id: 'median', label: 'Median filter', task: 'Task1', out: 'newOne.bmp',
          args: p => ['--median', IN, String(p.radius | 0)],
          params: [{ id: 'radius', label: 'radius', min: 1, max: 3, step: 1, def: 1 }],
          work: { maxEdge: 600 },
          blurb: 'Replace each pixel with the median of its window — erases salt-and-pepper noise while keeping edges crisp.' },
        { id: 'hmean', label: 'Harmonic mean', task: 'Task1', out: 'newOne.bmp',
          args: p => ['--hmean', IN, String(p.radius | 0)],
          params: [{ id: 'radius', label: 'radius', min: 1, max: 3, step: 1, def: 1 }],
          work: { maxEdge: 600 },
          blurb: 'A mean that bends toward the darkest neighbours — gentle on bright detail, fierce on bright noise.' },
      ],
    },
    {
      id: 'geometry', name: 'Geometry', task: 'Task1',
      blurb: 'Reflections — the same pixels, re-seated on the grid.',
      ops: [
        { id: 'hflip', label: 'Flip horizontal', task: 'Task1', out: 'newOne.bmp', args: () => ['--hflip', IN], blurb: 'Mirror left↔right.' },
        { id: 'vflip', label: 'Flip vertical', task: 'Task1', out: 'newOne.bmp', args: () => ['--vflip', IN], blurb: 'Mirror top↔bottom.' },
        { id: 'dflip', label: 'Rotate 180°', task: 'Task1', out: 'newOne.bmp', args: () => ['--dflip', IN], blurb: 'Diagonal flip — a half-turn of the frame.' },
      ],
    },
    {
      id: 'edges', name: 'Edges & Light', task: 'Task2',
      blurb: 'Where intensity changes fastest — the contours a camera never draws but the eye always finds.',
      ops: [
        { id: 'roberts', label: 'Roberts edges', task: 'Task2', out: 'newOne.bmp',
          args: () => ['--orobertsi', IN], work: { maxEdge: 700 },
          blurb: 'The Roberts cross operator — a 2×2 gradient, the oldest edge detector in the book (1963).' },
        { id: 'lines', label: 'Line detector', task: 'Task2', out: 'newOne.bmp',
          args: () => ['--oslineid', IN], work: { maxEdge: 700 },
          blurb: 'Convolution with a directional line mask, lifting linear structure out of the scene.' },
        { id: 'rayleigh', label: 'Rayleigh equalise', task: 'Task2', out: 'newOne.bmp',
          args: p => ['--hraleigh', IN, String(p.gmin | 0), String(p.gmax | 0)],
          params: [
            { id: 'gmin', label: 'g min', min: 0, max: 120, step: 2, def: 10 },
            { id: 'gmax', label: 'g max', min: 140, max: 255, step: 2, def: 240 },
          ], work: { maxEdge: 700 },
          blurb: 'Histogram equalisation under a Rayleigh density — redistributes tones to reveal buried detail.' },
      ],
    },
    {
      id: 'frequency', name: 'Frequency Domain', task: 'Task4',
      blurb: 'Leave the image plane entirely. The Fast Fourier Transform rewrites the picture as a sky of spatial frequencies; filter that sky and transform back.',
      ops: [
        { id: 'fft', label: 'FFT spectrum', task: 'Task4', inPath: I4, out: '/i_fft.bmp',
          args: () => ['--fft', I4], work: { square: 256, gray: true }, kind: 'spectrum',
          blurb: 'The magnitude spectrum (log-scaled, centred). Bright cross = strong horizontal/vertical structure; the halo = texture.' },
        { id: 'lowp', label: 'Low-pass', task: 'Task4', inPath: I4, out: '/i_lowp.bmp',
          args: p => ['--lowp', I4, String(p.r | 0)], params: [{ id: 'r', label: 'radius', min: 6, max: 120, step: 2, def: 36 }],
          work: { square: 256, gray: true },
          blurb: 'Keep only frequencies inside a disc of this radius — a soft, dreamlike blur done in frequency space.' },
        { id: 'highp', label: 'High-pass', task: 'Task4', inPath: I4, out: '/i_highp.bmp',
          args: p => ['--highp', I4, String(p.r | 0)], params: [{ id: 'r', label: 'radius', min: 2, max: 80, step: 2, def: 14 }],
          work: { square: 256, gray: true },
          blurb: 'Discard the low frequencies — only the sharp transitions survive, an etching of pure edge.' },
        { id: 'bpass', label: 'Band-pass', task: 'Task4', inPath: I4, out: '/i_bpass.bmp',
          args: p => ['--bpass', I4, String(p.r | 0), String(p.bw | 0)],
          params: [{ id: 'r', label: 'radius', min: 8, max: 110, step: 2, def: 40 }, { id: 'bw', label: 'width', min: 4, max: 80, step: 2, def: 24 }],
          work: { square: 256, gray: true },
          blurb: 'Keep a single annulus of frequencies — isolates texture at one scale.' },
      ],
    },
    {
      id: 'morphology', name: 'Morphology', task: 'Task3',
      blurb: 'Mathematical morphology — grow, shrink and probe shapes with a structuring element. Shown on a binarised plate.',
      binary: true,
      ops: [
        { id: 'erosion', label: 'Erosion', task: 'Task3', out: 'image_eroded.bmp', args: () => ['--erosion', IN], work: { square: 256, binary: true }, blurb: 'Shrink the white set — thin strokes vanish.' },
        { id: 'dilation', label: 'Dilation', task: 'Task3', out: 'image_dilated.bmp', args: () => ['--dilation', IN], work: { square: 256, binary: true }, quiet: true, blurb: 'Grow the white set — gaps close, forms thicken.' },
        { id: 'opening', label: 'Opening', task: 'Task3', out: 'image_dilated.bmp', args: () => ['--opening', IN], work: { square: 256, binary: true }, quiet: true, blurb: 'Erosion then dilation — removes specks, keeps overall shape.' },
        { id: 'closing', label: 'Closing', task: 'Task3', out: 'image_eroded.bmp', args: () => ['--closing', IN], work: { square: 256, binary: true }, quiet: true, blurb: 'Dilation then erosion — fills pinholes and hairline cracks.' },
        { id: 'hmt', label: 'Hit-or-miss', task: 'Task3', out: 'HMT.bmp', args: p => ['--HMT', IN, String(p.mask | 0)], params: [{ id: 'mask', label: 'kernel', min: 0, max: 3, step: 1, def: 0 }], work: { square: 256, binary: true }, blurb: 'Match an exact local pattern — corner and stroke detection at the pixel level.' },
      ],
    },
  ];

  // Statistics readout — original Task2 commands that print numbers to stdout.
  const STATS = {
    task: 'Task2', inPath: IN,
    items: [
      { id: 'cmean', cmd: '--cmean', label: 'Channel means' },
      { id: 'cvariance', cmd: '--cvariance', label: 'Variance' },
      { id: 'cstdev', cmd: '--cstdev', label: 'Std. deviation' },
      { id: 'cvarcoi', cmd: '--cvarcoi', label: 'Variation coeff.' },
      { id: 'casyco', cmd: '--casyco', label: 'Skewness' },
      { id: 'flaco', cmd: '--flaco', label: 'Kurtosis' },
    ],
  };

  // Histogram plot — produces a 256×256 graph image rather than a transformed photo.
  const HISTOGRAM = {
    id: 'histogram', task: 'Task2', out: 'histogram.bmp', kind: 'plot',
    channels: [{ v: 0, label: 'Red' }, { v: 1, label: 'Green' }, { v: 2, label: 'Blue' }],
    args: ch => ['--histogram', IN, String(ch)],
  };

  const api = { CATEGORIES, STATS, HISTOGRAM, IN, I4 };
  if (typeof module !== 'undefined' && module.exports) module.exports = api;
  else root.OPS = api;
})(typeof globalThis !== 'undefined' ? globalThis : this);
