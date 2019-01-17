/*
 * UiRender.re
 *
 * Core render logic for a UI bound toa Window
 */

open Reglfw.Glfw;

open Revery_Core;
open Revery_Math;

open UiContainer;

module Layout = Layout;
module LayoutTypes = Layout.LayoutTypes;

open RenderPass;

let _projection = Mat4.create();

let lastUpdate: ref(option(UiReact.RenderedElement.t)) = ref(None);
let previousElement: ref(option(UiReact.syntheticElement)) = ref(None);

let render = (container: UiContainer.t, component: UiReact.syntheticElement) => {
  let {rootNode, window, options, _} = container;

  AnimationTicker.tick();

  /* Perform reconciliation */
  let latest = switch(lastUpdate^) {
      | None => {
          let updates = UiReact.RenderedElement.render(rootNode, component);
          UiReact.RenderedElement.executeHostViewUpdates(updates) |> ignore;
          updates
      } 
      | Some(v) => {
          let nextElement = UiReact.RenderedElement.flushPendingUpdates(v); 
          UiReact.RenderedElement.executeHostViewUpdates(nextElement) |> ignore;
          nextElement;
      }
  };

  lastUpdate := Some(latest);

  /* Layout */
  let size = Window.getSize(window);
  let pixelRatio = Window.getDevicePixelRatio(window);

  switch (options.autoSize) {
  | false =>
    rootNode#setStyle(
      Style.make(
        ~position=LayoutTypes.Relative,
        ~width=size.width,
        ~height=size.height,
        (),
      ),
    );
    Layout.layout(rootNode, pixelRatio);
  | true =>
    rootNode#setStyle(Style.make());
    Layout.layout(rootNode, pixelRatio);
    let measurements = rootNode#measurements();
    let size: Window.windowSize = {
      width: measurements.width,
      height: measurements.height,
    };
    Window.setSize(window, size.width, size.height);
  };

  /* Render */

  Mat4.ortho(
    _projection,
    0.0,
    float_of_int(size.width),
    float_of_int(size.height),
    0.0,
    1000.0,
    -1000.0,
  );

  Performance.bench("draw", () => {
    /* Do a first pass for all 'opaque' geometry */
    /* This helps reduce the overhead for the more expensive alpha pass, next */

    let drawContext =
      NodeDrawContext.create(int_of_float(pixelRatio), 0, 1.0, size.height);

    let solidPass = SolidPass(_projection);
    rootNode#draw(solidPass, drawContext);

    /* Render all geometry that requires an alpha */
    let alphaPass = AlphaPass(_projection);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rootNode#draw(alphaPass, drawContext);
    glDisable(GL_BLEND);
  });
};
