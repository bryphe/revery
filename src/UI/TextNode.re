open Revery_Draw;

module Layout = Layout;
module LayoutTypes = Layout.LayoutTypes;

open Revery_Core;
open Revery_Font;

open Style;
open ViewNode;

open {
       let int_of_float_ceil = f => int_of_float(f +. 1.);
     };

class textNode (text: string) = {
  as _this;
  val mutable text = text;
  val mutable _isMeasured = false;
  val mutable _lines: list(string) = [];
  val mutable _smoothing = Smoothing.default;
  val mutable _fontFamily = Family.default;
  val mutable _fontWeight = Weight.Normal;
  val mutable _italicized = false;
  val mutable _fontSize = 14.;
  val mutable _underlined = false;
  val _textPaint = {
    let paint = Skia.Paint.make();
    Skia.Paint.setTextEncoding(paint, GlyphId);
    Skia.Paint.setLcdRenderText(paint, true);
    Skia.Paint.setAntiAlias(paint, true);
    paint;
  };
  inherit (class viewNode)() as _super;
  pub! draw = (parentContext: NodeDrawContext.t) => {
    let style = _super#getStyle();

    let {color, lineHeight, _} = style;
    let opacity = parentContext.opacity *. style.opacity;
    let colorWithAppliedOpacity = Color.multiplyAlpha(opacity, color);

    switch (Family.resolve(~italic=_italicized, _fontWeight, _fontFamily)) {
    | Error(_) => ()
    | Ok(font) =>
      Revery_Font.Smoothing.setPaint(~smoothing=_smoothing, _textPaint);
      Skia.Paint.setColor(_textPaint, Color.toSkia(colorWithAppliedOpacity));
      Skia.Paint.setTypeface(
        _textPaint,
        Revery_Font.FontCache.getSkiaTypeface(font),
      );
      Skia.Paint.setTextSize(_textPaint, _fontSize);

      let ascentPx =
        Text.ascent(~italic=_italicized, _fontFamily, _fontSize, _fontWeight);
      let lineHeightPx =
        lineHeight
        *. Text.lineHeight(
             ~italic=_italicized,
             _fontFamily,
             _fontSize,
             _fontWeight,
           );

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
            ~paint=_textPaint,
            ~x=0.,
            ~y=baselineY,
            ~text=glyphString,
            canvas,
          );
          if (_underlined) {
            let {underlinePosition, underlineThickness, _}: FontMetrics.t =
              FontCache.getMetrics(font, _fontSize);

            let width =
              FontRenderer.measure(
                ~smoothing=_smoothing,
                font,
                _fontSize,
                line,
              ).
                width;

            let rect =
              Skia.Rect.makeLtrb(
                0.,
                baselineY +. underlinePosition -. underlineThickness /. 2.,
                width,
                baselineY +. underlinePosition +. underlineThickness /. 2.,
              );
            CanvasContext.drawRect(~rect, ~paint=_textPaint, canvas);
          };
        },
        _lines,
      );
    };
  };
  pub! setStyle = style => {
    let lastStyle = _this#getStyle();
    _super#setStyle(style);
    let newStyle = _this#getStyle();

    if (lastStyle.lineHeight != newStyle.lineHeight) {
      _this#markLayoutDirty();
    };
  };
  pub textOverflow = (maxWidth): LayoutTypes.dimensions => {
    let {lineHeight, textOverflow, _}: Style.t = _super#getStyle();

    let formattedText = TextOverflow.removeLineBreaks(text);

    let measure = str =>
      Text.dimensions(
        ~smoothing=_smoothing,
        ~italic=_italicized,
        ~fontFamily=_fontFamily,
        ~fontSize=_fontSize,
        ~fontWeight=_fontWeight,
        str,
      )
      |> (value => value.width);

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
      lineHeight
      *. Text.lineHeight(
           ~italic=_italicized,
           _fontFamily,
           _fontSize,
           _fontWeight,
         );

    {
      width: int_of_float_ceil(width),
      height: int_of_float_ceil(lineHeightPx),
    };
  };
  pub setText = t =>
    if (!String.equal(t, text)) {
      text = t;
      _isMeasured = false;
      _this#markLayoutDirty();
    };
  pub setSmoothing = smoothing => _smoothing = smoothing;
  pub setFontFamily = fontFamily =>
    if (_fontFamily !== fontFamily) {
      _fontFamily = fontFamily;
      _isMeasured = false;
      _this#markLayoutDirty();
    };
  pub setFontWeight = fontWeight =>
    if (_fontWeight != fontWeight) {
      _fontWeight = fontWeight;
      _isMeasured = false;
      _this#markLayoutDirty();
    };
  pub setItalicized = italicized =>
    if (_italicized != italicized) {
      _italicized = italicized;
      _isMeasured = false;
      _this#markLayoutDirty();
    };
  pub setFontSize = fontSize =>
    if (_fontSize != fontSize) {
      _fontSize = fontSize;
      _isMeasured = false;
      _this#markLayoutDirty();
    };
  pub setUnderlined = underlined => {
    if (_underlined != underlined) {
      _this#markLayoutDirty();
    };
    _underlined = underlined;
  };
  pub measure = (width, _height): LayoutTypes.dimensions => {
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
    let {textWrap, lineHeight, _}: Style.t = style;
    let lineHeightPx =
      lineHeight
      *. Text.lineHeight(
           ~italic=_italicized,
           _fontFamily,
           _fontSize,
           _fontWeight,
         );

    let measureWidth = str =>
      Text.charWidth(
        ~smoothing=_smoothing,
        ~italic=_italicized,
        ~fontFamily=_fontFamily,
        ~fontSize=_fontSize,
        ~fontWeight=_fontWeight,
        str,
      );
    _lines =
      TextWrapping.wrapText(
        ~text,
        ~measureWidth,
        ~maxWidth=float_of_int(width),
        ~mode=textWrap,
      );

    let pickWiderLine = (leftWidth, right) => {
      let rightWidth =
        Text.dimensions(
          ~smoothing=_smoothing,
          ~italic=_italicized,
          ~fontFamily=_fontFamily,
          ~fontSize=_fontSize,
          ~fontWeight=_fontWeight,
          right,
        ).
          width;
      max(leftWidth, rightWidth);
    };
    let maxWidthLine = List.fold_left(pickWiderLine, 0., _lines);
    {
      width: int_of_float_ceil(maxWidthLine),
      height:
        int_of_float_ceil(
          float_of_int(List.length(_lines)) *. lineHeightPx,
        ),
    };
  };
  pub! getMeasureFunction = () => {
    let measure =
        (_mode, width, _widthMeasureMode, height, _heightMeasureMode) =>
      _this#measure(width, height);
    Some(measure);
  };
};
