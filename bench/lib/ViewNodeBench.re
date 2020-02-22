open BenchFramework;

open Revery.Draw;
open Revery.UI;

let options = Reperf.Options.create(~iterations=10000, ());

let createViewNode = () => {
  let viewNode = (new viewNode)();
  viewNode#recalculate();
  viewNode;
};

let setup = () => {
  let surface = SurfaceUtility.makeSurface(800l, 600l);
  let canvas = CanvasContext.createFromSurface(surface);

  NodeDrawContext.create(~canvas, ~zIndex=0, ~opacity=1.0, ());
};

module Data = {
  let viewNode = createViewNode();
};

let draw = context => {
  Data.viewNode#draw(context);
};

bench(~name="ViewNode: draw single node", ~options, ~setup, ~f=draw, ());
