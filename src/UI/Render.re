/*
 * UiRender.re
 *
 * Core render logic for a UI bound to a Window
 */

open Revery_Core;
open Revery_Draw;

module Layout = Layout;
module LayoutTypes = Layout.LayoutTypes;
module Log = (val Log.withNamespace("Revery.UI.Render"));

open RenderContainer;

let render =
    (
      ~forceLayout=false,
      renderContainer: RenderContainer.t,
      component: React.element('node),
    ) => {
  Log.trace("BEGIN: Render frame");
  let {rootNode, window, container, _} = renderContainer;

  /* Perform reconciliation */
  Performance.bench("reconcile", () =>
    container := Container.update(container^, component)
  );

  /* Layout */
  let size = Window.getSize(window);

  let pixelRatio = Window.getDevicePixelRatio(window);
  let scaleAndZoomFactor = Window.getScaleAndZoom(window);
  let canvasScalingFactor = pixelRatio *. scaleAndZoomFactor;

  let zoomFactor = Window.getZoom(window);

  Log.tracef(m =>
    m(
      "-- RENDER: pixelRatio: %f scaleAndZoom: %f zoom: %f canvasScaling: %f",
      pixelRatio,
      scaleAndZoomFactor,
      zoomFactor,
      canvasScalingFactor,
    )
  );

  let adjustedHeight =
    float_of_int(size.height) /. zoomFactor |> int_of_float;
  let adjustedWidth = float_of_int(size.width) /. zoomFactor |> int_of_float;
  Log.tracef(m =>
    m(
      "-- RENDER: adjustedWidth: %d adjustedHeight: %d",
      adjustedWidth,
      adjustedHeight,
    )
  );

  RenderContainer.updateCanvas(window, renderContainer);

  rootNode#setStyle(
    Style.make(
      ~position=LayoutTypes.Relative,
      ~width=adjustedWidth,
      ~height=adjustedHeight,
      (),
    ),
  );
  Layout.layout(~force=forceLayout, rootNode);

  /* Recalculate cached parameters */
  Performance.bench("recalculate", () => rootNode#recalculate());

  /* Flush any node callbacks */
  Performance.bench("flush", () => rootNode#flushCallbacks());

  /* Render */
  Performance.bench("draw", () => {
    /* Do a first pass for all 'opaque' geometry */
    /* This helps reduce the overhead for the more expensive alpha pass, next */
    switch (renderContainer.canvas^) {
    | None => ()
    | Some(canvas) =>
      let skiaRoot =
        Skia.Matrix.makeScale(
          canvasScalingFactor,
          canvasScalingFactor,
          0.,
          0.,
        );
      CanvasContext.setRootTransform(skiaRoot, canvas);
      let drawContext =
        NodeDrawContext.create(~canvas, ~zIndex=0, ~opacity=1.0, ());

      let backgroundColor = Window.getBackgroundColor(window);
      CanvasContext.clear(~color=backgroundColor |> Color.toSkia, canvas);
      // let drawContext = NodeDrawContext.create(~zIndex=0, ~opacity=1.0, ());

      rootNode#draw(drawContext);

      CanvasContext.setMatrix(canvas, Skia.Matrix.identity);
      //DebugDraw.enable();
      //DebugDraw.draw(canvas);

      if (Window.shouldShowFPSCounter(window)) {
        let w = float_of_int(adjustedWidth);
        let (x, y) = (w -. 64., 32.);
        let paint = Skia.Paint.make();
        Skia.Paint.setColor(
          paint,
          Skia.Color.makeArgb(255l, 50l, 200l, 50l),
        );
        CanvasContext.drawText(
          ~paint,
          ~x,
          ~y,
          ~text=Printf.sprintf("FPS: %d", Window.getFPS(window)),
          canvas,
        );
      };

      Revery_Draw.CanvasContext.flush(canvas);
    }
  });
  Log.trace("END: Render frame");
};
