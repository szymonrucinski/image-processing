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

  // Five-sentence wall-text shown when an instrument is applied. Written to match
  // what each original algorithm actually computes.
  const LESSONS = {
    negative: "A negative inverts every pixel's intensity with g = 255 − f, applied independently to the red, green and blue channels. Dark areas turn light and light areas turn dark, exactly like a film negative held up to a lamp. Because the mapping is its own inverse, applying it twice returns the original image untouched. It is the simplest possible point operation: the output at a pixel depends only on that same pixel, never on its neighbours. Photographers use it to inspect shadow detail and to reason about complementary colours.",
    brightness: "Brightness adds a constant offset to every pixel — g = f + shift — by the same amount everywhere. A positive shift lifts the whole tonal range toward white; a negative shift sinks it toward black. Any value that would leave the valid range is clamped to 0 or 255, so extreme regions can lose detail by 'clipping'. Being a pure point operation, it changes overall exposure but not contrast: the spacing between tones is preserved. Drag the slider to slide the entire histogram left or right.",
    contrast: "Contrast pivots tones around mid-grey using g = k·(f − 128) + 128. With a factor above 1 the tones spread apart — darks darken, lights brighten — giving the image more punch. With a factor below 1 they collapse toward grey, flattening the picture. Because everything pivots about 128, mid-greys stay roughly put while the extremes move the most. Like brightness it clamps at 0 and 255, so aggressive settings clip both shadows and highlights.",
    gamma: "Gamma applies a power-law tone curve: normalise to [0,1], raise to the power γ, then scale back with g = 255·(f/255)^γ. A γ below 1 lifts shadows and midtones, revealing detail in dark regions; a γ above 1 deepens them. Unlike linear brightness it bends the curve, so darks and lights are affected by different amounts. This mirrors the non-linear way displays emit and human eyes perceive light. It is the everyday tool for matching an image's perceived tonality.",
    median: "The median filter slides a square window over the image and replaces each pixel with the median value of its neighbourhood. Sorting the window and taking the middle value rejects outliers, so isolated 'salt-and-pepper' specks simply vanish. Crucially the median is a real value from the scene, so unlike a blur it keeps edges sharp instead of smearing them. A larger radius cleans more aggressively but starts to erase fine texture. It is the classic non-linear denoiser, computed here by sorting every window, pixel by pixel.",
    hmean: "The harmonic mean filter replaces each pixel with the harmonic mean of its window — the count divided by the sum of reciprocals. That average is pulled toward the smallest values present, making it very good at removing bright noise and Gaussian noise. It copes poorly with dark 'pepper' noise, which it tends to keep or spread. Compared with a plain average it protects edges a little better while still smoothing. A larger radius widens the window and strengthens the effect.",
    hflip: "A horizontal flip mirrors the image left-to-right, swapping each pixel in column x with the one in column (width − 1 − x). The loop walks only the left half of every row and exchanges it with the right, so it runs fully in place. No pixel values change — only their positions — which makes it perfectly reversible. It is a geometric transform: the discrete equivalent of reflecting across a vertical axis. Faces and text come out mirror-reversed.",
    vflip: "A vertical flip mirrors the image top-to-bottom, swapping each row y with row (height − 1 − y). It iterates over the top half and exchanges it with the bottom in place. As with every reflection it only relocates pixels, never alters their colour, so reapplying it restores the original. This is reflection across a horizontal axis. It is often used to correct images stored bottom-up, as many file formats are — including the BMP this engine round-trips.",
    dflip: "This diagonal flip turns the image a full 180°, mapping each pixel (x, y) to (width − 1 − x, height − 1 − y). It is a horizontal and a vertical reflection combined into one half-turn about the image centre. The loop swaps symmetric pairs, so it works in place and is its own inverse. The result is the picture upside-down and mirrored — what you would see rotating the print 180°. It is a pure geometric repositioning that leaves every pixel value unchanged.",
    roberts: "The Roberts cross is one of the oldest edge detectors, introduced by Lawrence Roberts in 1963. At each pixel it measures how fast intensity changes along the two diagonals using tiny 2×2 differences, then combines them as √(d₁² + d₂²). Flat regions give a near-zero response and stay black, while sharp transitions light up bright. Its minuscule kernel makes it fast and keen on fine, high-frequency edges, at the cost of sensitivity to noise. The output is a gradient-magnitude map — an outline drawing of the scene.",
    lines: "This is a directional line detector built on convolution with a 3×3 mask tuned to one orientation. The mask responds strongly where a thin line of that direction passes through and weakly elsewhere, isolating linear structure. Each output pixel is the sum of mask weights times the surrounding neighbourhood, normalised and clamped to [0,255]. Where edge detectors find boundaries, line detectors find one-pixel-wide ridges. It is handy for pulling out wires, roads, cracks and other elongated features.",
    rayleigh: "Rayleigh equalisation redistributes the image's tones to follow a Rayleigh probability density instead of leaving them bunched together. For each pixel it counts how many pixels are darker, then maps that through g = g_min + √(2σ²·ln(N ⁄ cumulative)). This pulls compressed tonal ranges apart, surfacing detail hidden in shadows or flat areas. The g-min and g-max sliders set the output range and, through σ, how forcefully tones are spread. It is a contrast enhancement driven entirely by the image's own histogram.",
    fft: "The Fast Fourier Transform rewrites the image as a sum of 2-D sine waves, trading the spatial grid for a map of spatial frequencies. What you see is the magnitude spectrum, log-scaled and shifted so the lowest frequencies sit at the centre. The bright central cross marks strong horizontal and vertical structure; the surrounding halo encodes texture and fine detail. Bright points far from the centre mean sharp, repeating patterns at that scale and orientation. This view is the doorway to frequency-domain filtering — every filter below works by editing this very spectrum.",
    lowp: "An ideal low-pass filter keeps only the frequencies inside a disc of the chosen radius and zeroes everything outside it. Low frequencies carry broad tones and gentle shading, so keeping them yields a soft, smooth, blurred image. The smaller the radius, the more high-frequency detail is thrown away and the dreamier the result. Because the cut-off is a hard circle you may see faint 'ringing' near sharp edges — the signature of ideal filters. The picture is transformed to frequency space, masked, and transformed back.",
    highp: "An ideal high-pass filter is the mirror of the low-pass: it discards the low frequencies inside the radius and keeps the high ones outside. Smooth regions fall toward black while edges, texture and fine detail survive, giving an etched, outline-like image. The radius sets how much of the smooth 'base' is removed — a larger radius strips away more. It is a close cousin of sharpening and edge detection, since edges are exactly where high frequencies live. As with all ideal filters, the hard cut-off can introduce ringing.",
    bpass: "A band-pass filter keeps only an annulus of frequencies — a ring at a chosen radius with a chosen width — and removes everything else. This isolates structure that exists at one particular scale, suppressing both the broadest tones and the very finest detail. The radius selects which scale you keep; the width sets how wide a band around it survives. It is the instrument for extracting periodic patterns, like the weave of cloth or scan-line interference. The image is filtered in frequency space and transformed back so you can see the result.",
    erosion: "Erosion is a fundamental morphological operation that shrinks the white foreground of a binary image. A small structuring element slides over the picture, and a pixel stays white only if its neighbours are white too. Thin protrusions, isolated specks and one-pixel bridges are eaten away. It is shown here on a binarised version of the photograph, because morphology reasons about shapes, not tones. Erosion is the basic building block beneath the opening and closing operations.",
    dilation: "Dilation is the dual of erosion: it grows the white foreground of a binary image. The structuring element passes over the picture and any pixel touching a white neighbour becomes white itself. Small holes fill in, gaps bridge over and shapes thicken. Like erosion it works on a binarised plate, since morphology cares about geometry rather than grey levels. Together, erosion and dilation form the algebra of mathematical morphology.",
    opening: "An opening is an erosion followed by a dilation with the same structuring element. The erosion strips away small bright specks and thin connections, then the dilation restores the surviving shapes to roughly their original size. The net effect is to clean up speckle noise and smooth outlines without shrinking the main forms. It tends to snap thin necks between blobs. Opening is the standard way to remove small bright artefacts from a binary image.",
    closing: "A closing reverses the order: a dilation followed by an erosion. The dilation fills small dark holes and hairline cracks, and the erosion then pulls the result back to its original scale. The net effect is to seal gaps and join nearby shapes without permanently bloating them. It is the natural partner of opening, which removes specks rather than filling holes. Closing is used to consolidate fragmented regions in a binary image.",
    hmt: "The hit-or-miss transform searches for an exact local pixel pattern rather than simply growing or shrinking shapes. A small template says which neighbours must be foreground and which must be background; a pixel matches only when the whole pattern fits perfectly. Matching pixels are marked, isolating very specific features such as corners, line-ends and junctions. Choosing a different kernel detects a different configuration. It is morphology's tool for precise, pixel-level shape recognition.",
  };

  const api = { CATEGORIES, STATS, HISTOGRAM, LESSONS, IN, I4 };
  if (typeof module !== 'undefined' && module.exports) module.exports = api;
  else root.OPS = api;
})(typeof globalThis !== 'undefined' ? globalThis : this);
