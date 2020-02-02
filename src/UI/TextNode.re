open Revery_Draw;

module Layout = Layout;
module LayoutTypes = Layout.LayoutTypes;

open Revery_Core;

open Style;
open ViewNode;

class textNode (text: string) = {
  as _this;
  val mutable text = text;
  val mutable _isMeasured = false;
  val mutable _lines: list(string) = [];
  inherit (class viewNode)() as _super;
  pub! draw = (parentContext: NodeDrawContext.t) => {
    let style = _super#getStyle();

    let {color, fontFamily, fontSize, lineHeight, _} = style;
    let opacity = parentContext.opacity *. style.opacity;
    let colorWithAppliedOpacity = Color.multiplyAlpha(opacity, color);

    switch (Revery_Font.FontCache.load(fontFamily)) {
    | Error(_) => ()
    | Ok(font) =>
      let paint = Skia.Paint.make();
      Skia.Paint.setColor(paint, Color.toSkia(colorWithAppliedOpacity));
      Skia.Paint.setTypeface(
        paint,
        Revery_Font.FontCache.getSkiaTypeface(font),
      );
      Skia.Paint.setTextEncoding(paint, GlyphId);
      Skia.Paint.setLcdRenderText(paint, true);
      Skia.Paint.setAntiAlias(paint, true);
      Skia.Paint.setSubpixelText(paint, true);
      Skia.Paint.setTextSize(paint, fontSize);

      let ascentPx = Text.getAscent(~fontFamily, ~fontSize, ());
      let lineHeightPx =
        lineHeight *. Text.getLineHeight(~fontFamily, ~fontSize, ());

      /* when style.width & style.height are defined, Layout doesn't call the measure function */
      if (!_isMeasured) {
        _this#measure(style.width, style.height) |> ignore;
      };

      let {canvas, _}: NodeDrawContext.t = parentContext;
      // TODO find a way to only manage the matrix stack in Node
      let world = _this#getWorldTransform();
      Revery_Draw.CanvasContext.setMatrix(canvas, world);

      List.iteri(
        (lineIndex, line) => {
          let baselineY =
            ascentPx *. (-1.0) +. lineHeightPx *. float_of_int(lineIndex);

          let glyphString =
            line
            |> Revery_Font.shape(font)
            |> Revery_Font.ShapeResult.getGlyphString;

          CanvasContext.drawText(
            ~paint,
            ~x=0.,
            ~y=baselineY,
            ~text=glyphString,
            canvas,
          );
        },
        _lines,
      );
    };
  };
  pub! setStyle = style => {
    let lastStyle = _this#getStyle();
    _super#setStyle(style);
    let newStyle = _this#getStyle();

    if (lastStyle.lineHeight != newStyle.lineHeight
        || lastStyle.fontSize != newStyle.fontSize
        || !String.equal(lastStyle.fontFamily, newStyle.fontFamily)) {
      _this#markLayoutDirty();
    };
  };
  pub textOverflow = (maxWidth): LayoutTypes.dimensions => {
    let {fontFamily, fontSize, lineHeight, textOverflow, _}: Style.t =
      _super#getStyle();

    let formattedText = TextOverflow.removeLineBreaks(text);

    let measure = str =>
      Text.measure(~fontFamily, ~fontSize, str) |> (value => value.width);

    let width = measure(formattedText);
    let isOverflowing = width >= maxWidth;

    let handleOverflow =
      TextOverflow.handleOverflow(~maxWidth, ~text=formattedText, ~measure);

    let truncated =
      switch (textOverflow, isOverflowing) {
      | (Ellipsis, true) => handleOverflow()
      | (UserDefined(character), true) => handleOverflow(~character, ())
      | (Clip, true) => handleOverflow(~character="", ())
      | (_, false)
      | (Overflow, _) => text
      };

    _lines = [truncated];

    let lineHeightPx =
      lineHeight *. Text.getLineHeight(~fontFamily, ~fontSize, ());

    {width: int_of_float(width), height: int_of_float(lineHeightPx)};
  };
  pub setText = t =>
    if (!String.equal(t, text)) {
      text = t;
      _isMeasured = false;
      _this#markLayoutDirty();
    };
  pub measure = (width, _height) => {
    _isMeasured = true;
    /**
         If the width value is set to cssUndefined i.e. the user did not
         set a width then do not attempt to use textOverflow
       */
    (
      switch (_super#getStyle()) {
      | {width: textWidth, _} as style
          when textWidth == Layout.Encoding.cssUndefined =>
        _this#handleTextWrapping(width, style)
      | {textOverflow: Ellipsis | UserDefined(_), _} =>
        _this#textOverflow(float_of_int(width))
      | style => _this#handleTextWrapping(width, style)
      }
    );
  };
  pub handleTextWrapping = (width, style) => {
    let {textWrap, fontFamily, fontSize, lineHeight, _}: Style.t = style;
    let lineHeightPx =
      lineHeight *. Text.getLineHeight(~fontFamily, ~fontSize, ());

    let measureWidth = str =>
      Text.measureCharWidth(~fontFamily, ~fontSize, str);
    _lines =
      TextWrapping.wrapText(
        ~text,
        ~measureWidth,
        ~maxWidth=float_of_int(width),
        ~mode=textWrap,
      );

    let pickWiderLine = (leftWidth, right) => {
      let rightWidth = Text.measure(~fontFamily, ~fontSize, right).width;
      max(leftWidth, rightWidth);
    };
    let maxWidthLine = List.fold_left(pickWiderLine, 0., _lines);
    {
      width: int_of_float(maxWidthLine),
      height:
        int_of_float(float_of_int(List.length(_lines)) *. lineHeightPx),
    };
  };
  pub! getMeasureFunction = () => {
    let measure =
        (_mode, width, _widthMeasureMode, height, _heightMeasureMode) =>
      _this#measure(width, height);
    Some(measure);
  };
};
