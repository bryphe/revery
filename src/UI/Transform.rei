open Revery_Math;

type t =
  | RotateZ(Angle.t)
  | RotateY(Angle.t)
  | RotateX(Angle.t)
  | Rotate(Angle.t)
  | Scale(float)
  | ScaleX(float)
  | ScaleY(float)
  | TranslateX(float)
  | TranslateY(float);

let toMat4: (float, float, list(t)) => Skia.Matrix.t;
