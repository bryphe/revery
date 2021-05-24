type colorType = SkiaWrapped.colorType;
type alphaType = SkiaWrapped.alphaType;
type data = SkiaWrapped.data;

module Color: {
  type t = int32;

  let makeArgb: (int32, int32, int32, int32) => t;
  let getA: t => int32;
  let getR: t => int32;
  let getG: t => int32;
  let getB: t => int32;

  module Float: {
    let makeArgb: (float, float, float, float) => t;
    let getA: t => float;
    let getR: t => float;
    let getG: t => float;
    let getB: t => float;
  };
};

module FontStyle: {
  type t;
  type slant = SkiaWrapped.FontStyle.slant;

  let make: (int, int, slant) => t;

  let getSlant: t => slant;
  let getWeight: t => int;
  let getWidth: t => int;
};

module FilterQuality: {type t = SkiaWrapped.FilterQuality.t;};
module Hinting: {type t = SkiaWrapped.Hinting.t;};
module TextEncoding: {type t = SkiaWrapped.TextEncoding.t;};

module Stream: {
  type t;

  let hasLength: t => bool;
  let getLength: t => int;
  let makeFileStream: string => option(t);
  let makeMemoryStreamFromString: (string, int) => t;
  let makeMemoryStreamFromData: data => t;
};

module Data: {
  type t = data;

  let makeFromFileName: string => option(t);
  let makeString: t => string;
  let makeFromStream: (Stream.t, int) => t;
};

module Typeface: {
  type t;

  let getFamilyName: t => string;
  let makeFromName: (string, FontStyle.t) => option(t);
  let makeFromFile: (string, int) => option(t);
  let toStream: t => Stream.t;
  let getFontStyle: t => FontStyle.t;
  let getUniqueID: t => int32;
  let equal: (t, t) => bool;
};

module FontManager: {
  type t;

  let makeDefault: unit => t;
  let matchFamilyStyle: (t, string, FontStyle.t) => option(Typeface.t);
  let matchFamilyStyleCharacter:
    (t, string, FontStyle.t, list(string), Uchar.t) => option(Typeface.t);
};

module FontMetrics: {
  type t = SkiaWrapped.FontMetrics.t;

  let make: unit => t;
  let getAscent: t => float;
  let getDescent: t => float;
  let getTop: t => float;
  let getBottom: t => float;
  let getUnderlinePosition: t => float;
  let getUnderlineThickness: t => float;
  let getAvgCharacterWidth: t => float;
  let getMaxCharacterWidth: t => float;
};

module ImageFilter: {
  type t;

  module CropRect: {type t;};

  module DropShadow: {
    type shadowMode = SkiaWrapped.ImageFilter.DropShadow.shadowMode;

    let make:
      (
        float,
        float,
        float,
        float,
        Color.t,
        shadowMode,
        option(t),
        option(CropRect.t)
      ) =>
      t;
  };

  module Blur: {
    let make:
      (~sigmaX: float, ~sigmaY: float, option(t), option(CropRect.t)) => t;
  };

  let makeDropShadow:
    (
      float,
      float,
      float,
      float,
      Color.t,
      DropShadow.shadowMode,
      option(t),
      option(CropRect.t)
    ) =>
    t;
};

module Rect: {
  type t;

  module Mutable: {let setLtrb: (~out: t, float, float, float, float) => unit;
  };

  let makeEmpty: unit => t;
  let makeLtrb: (float, float, float, float) => t;

  let getTop: t => float;
  let getLeft: t => float;
  let getRight: t => float;
  let getBottom: t => float;

  let toString: t => string;
};

module Point: {
  type t;

  let make: (float, float) => t;

  let getX: t => float;
  let getY: t => float;
};

module Vector: {
  type t = Point.t;

  let make: (float, float) => t;

  let getX: t => float;
  let getY: t => float;
};

module Shader: {
  type t;

  type tileMode = SkiaWrapped.Shader.tileMode;

  let makeEmpty: unit => t;

  type colorStop = {
    color: Color.t,
    position: float,
  };

  let makeLinearGradient2:
    (
      ~startPoint: Point.t,
      ~stopPoint: Point.t,
      ~startColor: Color.t,
      ~stopColor: Color.t,
      ~tileMode: tileMode
    ) =>
    t;

  let makeLinearGradient:
    (
      ~startPoint: Point.t,
      ~stopPoint: Point.t,
      ~colorStops: list(colorStop),
      ~tileMode: tileMode
    ) =>
    t;
};

module Path: {
  type t;

  let make: unit => t;

  let addRoundRect:
    (
      t,
      Rect.t,
      float,
      float,
      ~direction: SkiaWrapped.Path.pathDirection=?,
      unit
    ) =>
    unit;
  let addCircle:
    (
      t,
      float,
      float,
      ~radius: float,
      ~direction: SkiaWrapped.Path.pathDirection=?,
      unit
    ) =>
    unit;

  let moveTo: (t, float, float) => unit;
  let rMoveTo: (t, float, float) => unit;
  let lineTo: (t, float, float) => unit;
  let rLineTo: (t, float, float) => unit;
  let cubicTo: (t, float, float, float, float, float, float) => unit;
  let rCubicTo: (t, float, float, float, float, float, float) => unit;
  let quadTo: (t, float, float, float, float) => unit;
  let rQuadTo: (t, float, float, float, float) => unit;
  let arcTo:
    (
      t,
      float,
      float,
      float,
      SkiaWrapped.Path.arcSize,
      SkiaWrapped.Path.pathDirection,
      float,
      float
    ) =>
    unit;
  let rArcTo:
    (
      t,
      float,
      float,
      float,
      SkiaWrapped.Path.arcSize,
      SkiaWrapped.Path.pathDirection,
      float,
      float
    ) =>
    unit;
  let close: t => unit;

  let getLastPoint: (t, Point.t) => bool;
};

module Matrix: {
  type t;

  let identity: t;

  let make: unit => t;
  let makeAll:
    (float, float, float, float, float, float, float, float, float) => t;
  let makeTranslate: (float, float) => t;
  let makeScale: (float, float, float, float) => t;

  let setAll:
    (t, float, float, float, float, float, float, float, float, float) => unit;
  let get: (t, int) => float;
  let getScaleX: t => float;
  let getScaleY: t => float;
  let getSkewX: t => float;
  let getSkewY: t => float;
  let getTranslateX: t => float;
  let getTranslateY: t => float;
  let getPerspX: t => float;
  let getPerspY: t => float;

  let set: (t, int, float) => unit;
  let setScaleX: (t, float) => unit;
  let setScaleY: (t, float) => unit;
  let setSkewX: (t, float) => unit;
  let setSkewY: (t, float) => unit;
  let setTranslateX: (t, float) => unit;
  let setTranslateY: (t, float) => unit;
  let setPerspX: (t, float) => unit;
  let setPerspY: (t, float) => unit;

  let setTranslate: (t, float, float) => unit;
  let setScale: (t, float, float, float, float) => unit;
  let setSkew: (t, float, float, float, float) => unit;
  let setIdentity: t => unit;
  let reset: t => unit;

  let invert: (t, t) => bool;
  let concat: (t, t, t) => unit;
  let preConcat: (t, t) => unit;
  let postConcat: (t, t) => unit;
  let mapRect: (t, Rect.t, Rect.t) => unit;
  let mapPoints: (t, Point.t, Point.t, int) => unit;
  let mapVectors: (t, Vector.t, Vector.t, int) => unit;
  let mapXy: (t, float, float, Point.t) => unit;
  let mapVector: (t, float, float, Vector.t) => unit;
  let mapRadius: (t, float) => float;
};

module PathEffect: {
  module Style: {type t = [ | `translate | `rotate | `morph];};

  type t;

  let create1d: (~style: Style.t, ~advance: float, ~phase: float, Path.t) => t;
  let create2dLine: (~width: float, ~matrix: Matrix.t) => t;
  let create2dPath: (~matrix: Matrix.t, Path.t) => t;
  //let createDiscrete: (~length: float, ~deviation: float, unit) => t;
};

module Paint: {
  type t;
  type style = SkiaWrapped.Paint.style;

  let make: unit => t;

  let setColor: (t, Color.t) => unit;
  let setAntiAlias: (t, bool) => unit;

  let setAutohinted: (t, bool) => unit;
  let isAutohinted: t => bool;

  let setHinting: (t, Hinting.t) => unit;
  let getHinting: t => Hinting.t;

  let setFilterQuality: (t, FilterQuality.t) => unit;
  let getFilterQuality: t => FilterQuality.t;

  let setPathEffect: (t, PathEffect.t) => unit;
  let getPathEffect: t => PathEffect.t;

  let setStyle: (t, style) => unit;
  let setStrokeWidth: (t, float) => unit;
  let setImageFilter: (t, option(ImageFilter.t)) => unit;
  let setTypeface: (t, Typeface.t) => unit;
  let setLcdRenderText: (t, bool) => unit;
  let setSubpixelText: (t, bool) => unit;
  let setTextSize: (t, float) => unit;
  let setAlpha: (t, float) => unit;
  let getFontMetrics: (t, FontMetrics.t, float) => float;
  let measureText: (t, string, option(Rect.t)) => float;

  let setTextEncoding: (t, TextEncoding.t) => unit;
  let getTextEncoding: t => TextEncoding.t;

  let setShader: (t, Shader.t) => unit;
};

module IRect: {
  type t;

  let makeEmpty: unit => t;
  let makeLtrb: (int32, int32, int32, int32) => t;
};

module Matrix44: {
  type t;

  let makeIdentity: unit => t;
  let makeEmpty: unit => t;

  let setRotateAboutDegrees: (t, float, float, float, float) => unit;
  let setRotateAboutRadians: (t, float, float, float, float) => unit;

  let setTranslate: (t, float, float, float) => unit;
  let preTranslate: (t, float, float, float) => unit;
  let postTranslate: (t, float, float, float) => unit;

  let setScale: (t, float, float, float) => unit;
  let preScale: (t, float, float, float) => unit;
  let postScale: (t, float, float, float) => unit;

  let setConcat: (t, t, t) => unit;
  let preConcat: (t, t) => unit;
  let postConcat: (t, t) => unit;

  let get: (t, int, int) => float;
  let set: (t, int, int, float) => unit;
  let toMatrix: (t, Matrix.t) => unit;
};

module RRect: {
  type t;

  type rRectType = SkiaWrapped.RRect.rRectType;
  type corner = SkiaWrapped.RRect.corner;

  let make: unit => t;
  let copy: t => t;

  let getType: t => rRectType;
  let getRect: (t, Rect.t) => unit;
  let getRadii: (t, corner, Vector.t) => unit;
  let getWidth: t => float;
  let getHeight: t => float;
  let setEmpty: t => unit;
  let setRect: (t, Rect.t) => unit;
  let setOval: (t, Rect.t) => unit;
  let setRectXy: (t, Rect.t, float, float) => unit;
  let setNinePatch: (t, Rect.t, float, float, float, float) => unit;
  let setRectRadii:
    (t, Rect.t, Vector.t, Vector.t, Vector.t, Vector.t) => unit;
  let inset: (t, float, float) => unit;
  let outset: (t, float, float) => unit;
  let offset: (t, float, float) => unit;
  let contains: (t, Rect.t) => bool;
  let isValid: t => bool;
  let transform: (t, Matrix.t, t) => bool;
};

module ColorSpace: {type t;};

module ImageInfo: {
  type t;

  let make: (int32, int32, colorType, alphaType, option(ColorSpace.t)) => t;
};

module Image: {
  type t;

  let makeFromEncoded: (Data.t, option(IRect.t)) => option(t);
  let encodeToData: t => Data.t;

  let width: t => int;
  let height: t => int;
};

type pixelGeometry = SkiaWrapped.pixelGeometry;

module Gr: {
  type surfaceOrigin = SkiaWrapped.Gr.surfaceOrigin;

  module Gl: {
    module Interface: {
      type t;

      let makeNative: unit => option(t);
      let makeSdl2: unit => option(t);
      let makeSdl2ES: unit => option(t);
    };

    module FramebufferInfo: {
      type t;

      let make: (Unsigned.UInt.t, Unsigned.UInt.t) => t;
    };
  };

  module Context: {
    type t;

    let makeGl: option(Gl.Interface.t) => option(t);
  };

  module BackendRenderTarget: {
    type t;

    let makeGl: (int, int, int, int, Gl.FramebufferInfo.t) => t;
  };
};

type clipOp = SkiaWrapped.clipOp;

module Canvas: {
  type t;

  let clear: (t, Color.t) => unit;
  let drawPaint: (t, Paint.t) => unit;
  let drawRect: (t, Rect.t, Paint.t) => unit;
  let drawRectLtwh: (t, float, float, float, float, Paint.t) => unit;
  let drawRoundRect: (t, Rect.t, float, float, Paint.t) => unit;
  let drawOval: (t, Rect.t, Paint.t) => unit;
  let drawCircle: (t, float, float, float, Paint.t) => unit;
  let drawRRect: (t, RRect.t, Paint.t) => unit;
  let drawPath: (t, Path.t, Paint.t) => unit;
  let drawText: (t, string, float, float, Paint.t) => unit;
  let drawImage: (t, Image.t, float, float, option(Paint.t)) => unit;
  let drawImageRect:
    (t, Image.t, option(Rect.t), Rect.t, option(Paint.t)) => unit;

  let concat: (t, Matrix.t) => unit;
  let setMatrix: (t, Matrix.t) => unit;
  let translate: (t, float, float) => unit;
  let scale: (t, float, float) => unit;
  let rotate: (t, float) => unit;
  let skew: (t, float, float) => unit;
  let resetMatrix: t => unit;

  let clipRect: (t, Rect.t, clipOp, bool) => unit;
  let clipPath: (t, Path.t, clipOp, bool) => unit;
  let clipRRect: (t, RRect.t, clipOp, bool) => unit;
  let save: t => int;
  let saveLayer: (t, option(Rect.t), option(Paint.t)) => int;
  let restore: t => unit;
  let flush: t => unit;
};

module SurfaceProps: {
  type t;

  let make: (Unsigned.UInt32.t, pixelGeometry) => t;
};

module Surface: {
  type t;

  let makeRaster: (ImageInfo.t, int, option(SurfaceProps.t)) => option(t);
  let makeRenderTarget:
    (
      Gr.Context.t,
      bool,
      ImageInfo.t,
      int,
      Gr.surfaceOrigin,
      option(SurfaceProps.t),
      bool
    ) =>
    option(t);
  let makeFromBackendRenderTarget:
    (
      Gr.Context.t,
      Gr.BackendRenderTarget.t,
      Gr.surfaceOrigin,
      colorType,
      option(ColorSpace.t),
      option(SurfaceProps.t)
    ) =>
    option(t);

  // Draw surface [surface] onto a [canvas] at the specified [x,y] points.
  let draw:
    (~paint: option(Paint.t)=?, ~canvas: Canvas.t, ~x: float, ~y: float, t) =>
    unit;
  let makeImageSnapshot: t => Image.t;
  let getCanvas: t => Canvas.t;
  let getWidth: t => int;
  let getHeight: t => int;
  let getProps: t => SurfaceProps.t;
};

module SVG: {
  type t;

  let makeFromStream: Stream.t => option(t);
  let render: (t, Canvas.t) => unit;
  let setContainerSize: (t, float, float) => unit;
  let getContainerWidth: t => float;
  let getContainerHeight: t => float;
};
