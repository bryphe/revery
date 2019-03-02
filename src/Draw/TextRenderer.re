/*
 * TextRenderer.re
 * 
 * Core logic for rendering text to screen.
 */

open Reglfw.Glfw;

open Revery_Core;
module Geometry = Revery_Geometry;
open Revery_Math;
open Revery_Shaders;

open Fontkit;

/*
 * Get the size of the bitmap we use for rendering text. This is rarely 1:1
 * with the requested fontSize. For example, in a high DPI that has a 3x pixel
 * ratio, we want to render a 3x size bitmap.
 */
let _getScaledFontSize = (fontSize) => {
    let ctx = RenderPass.getContext();

    int_of_float(
        (float_of_int(fontSize) *. ctx.pixelRatio *. float_of_int(ctx.scaleFactor)) +. 0.5
    );
};

let getLineHeight = (~fontFamily, ~fontSize, ~lineHeight, ()) => {
    let font = FontCache.load(fontFamily, fontSize);
    let metrics = FontRenderer.getNormalizedMetrics(font);
    lineHeight *. metrics.height;
}

let identityMatrix = Mat4.create();

let drawText = (
    ~fontFamily: string,
    ~fontSize: int,
    ~color: Color.t=Colors.white,
    ~transform: Mat4.t = identityMatrix,
    x: float,
    y: float,
    text: string,
    ) => {

    let pass = RenderPass.getCurrent();

    switch (pass) {
    | AlphaPass(ctx) => {

        /* let lineHeightPx = getLineHeight(~fontFamily, ~fontSize, ~lineHeight, ()); */
       let m = ctx.projection; 
      let quad = Assets.quad();
      let textureShader = Assets.fontShader();
      CompiledShader.use(textureShader);

      CompiledShader.setUniformMatrix4fv(
        textureShader,
        "uProjection",
        m,
      );

      let font =
        FontCache.load(
          fontFamily,
          _getScaledFontSize(fontSize),
          );

      CompiledShader.setUniform4fv(
        textureShader,
        "uColor",
        Color.toVec4(color),
      );

      let metrics = FontRenderer.getNormalizedMetrics(font);
      let multiplier = ctx.pixelRatio *. float_of_int(ctx.scaleFactor);
      /* Position the baseline */
      let baseline = (metrics.height -. metrics.descenderSize) /. multiplier;
      ();

      let outerTransform = Mat4.create();
      Mat4.fromTranslation(outerTransform, Vec3.create(0.0, baseline, 0.0));
      let render = (s: Fontkit.fk_shape, x: float, y: float) => {
        let glyph = FontRenderer.getGlyph(font, s.glyphId);

        let {width, height, bearingX, bearingY, advance, _} = glyph;

        let width = float_of_int(width) /. multiplier;
        let height = float_of_int(height) /. multiplier;
        let bearingX = float_of_int(bearingX) /. multiplier;
        let bearingY = float_of_int(bearingY) /. multiplier;
        let advance = float_of_int(advance) /. multiplier;

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        let texture = FontRenderer.getTexture(font, s.glyphId);
        glBindTexture(GL_TEXTURE_2D, texture);
        /* TODO: Bind texture */

        let glyphTransform = Mat4.create();
        Mat4.fromTranslation(
          glyphTransform,
          Vec3.create(
            x +. bearingX +. width /. 2.,
            y +. height *. 0.5 -. bearingY,
            0.0,
          ),
        );

        let scaleTransform = Mat4.create();
        Mat4.fromScaling(scaleTransform, Vec3.create(width, height, 1.0));

        let local = Mat4.create();
        Mat4.multiply(local, glyphTransform, scaleTransform);

        let xform = Mat4.create();
        Mat4.multiply(xform, outerTransform, local);
        Mat4.multiply(xform, transform, xform);

        CompiledShader.setUniformMatrix4fv(
          textureShader,
          "uWorld",
          xform,
        );

        Geometry.draw(quad, textureShader);

        x +. advance /. 64.0;
      };

          let shapedText = FontRenderer.shape(font, text);
          let startX = ref(x);

          Array.iter(
            (s) => {
              let nextX =
                render(s, startX^, y);
              startX := nextX;
            },
            shapedText,
          );
    }
    | _ => ();
    }
};
