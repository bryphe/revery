type t;

let create: (float, float, float, float) => t;

let equals: (t, t) => bool;

let getBounds: t => (float, float, float, float);

let intersects: (t, t) => bool;

let intersect: (t, t) => t;

let isPointInside: (~x: float, ~y: float, t) => bool;

let transform: (t, Skia.Matrix.t) => t;

let toString: t => string;
