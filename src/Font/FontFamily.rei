open FontManager;

type t;

type variantSolver =
  (~fontWeight: FontWeight.t, ~italicized: bool, ~monospaced: bool, unit) =>
  string;

let fromFile: (~variant: variantSolver=?, string) => t;
let system: string => t;

let default: t;