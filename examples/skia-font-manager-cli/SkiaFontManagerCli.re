open Skia;

let emitPng = (path, surface) => {
  let image = Surface.makeImageSnapshot(surface);
  let data = Image.encodeToData(image);
  let dataString = Data.makeString(data);
  let fileOutputChannel = open_out_bin(path);
  output_string(fileOutputChannel, dataString);
  close_out(fileOutputChannel);
};

let makeSurface = (width, height) => {
  let imageInfo = ImageInfo.make(width, height, Rgba8888, Premul, None);
  Surface.makeRaster(imageInfo, 0, None);
};

let draw = canvas => {
  let fontManager = FontManager.makeDefault();
  let style = FontStyle.make(400, 5, Upright);
  let maybeTypeface =
    FontManager.matchFamilyStyle(fontManager, "Arial", style);
  let fill = Paint.make();
  let font = Font.makeDefault();

  Paint.setColor(fill, Color.makeArgb(0xFFl, 0xFFl, 0xFFl, 0xFFl));
  Font.setTextSize(font, 16.);
  Font.setSubpixelText(font, true);

  switch (maybeTypeface) {
  | None =>
    print_endline("Normal Arial not found. Ensure you have it available.")
  | Some(typeface) =>
    Font.setTypeface(font, typeface);
    Canvas.drawText(canvas, "Arial (System)", GlyphId, 10., 20., font, fill);
    let stream = Typeface.toStream(typeface);
    let length = Stream.getLength(stream);
    Printf.printf("Stream length: %d\n", length);
    let data = Data.makeFromStream(stream, length);
    let oc = open_out("Arial.ttf");
    print_endline("Writing Arial Normal to Arial.ttf...");
    Printf.fprintf(oc, "%s", Data.makeString(data));
    print_endline("Written!");
    close_out(oc);
  };

  let maybeTypeface =
    FontManager.matchFamilyStyle(fontManager, "Times New Roman", style);
  switch (maybeTypeface) {
  | None =>
    print_endline(
      "Normal Times New Roman not found. Ensure you have it available.",
    )
  | Some(typeface) =>
    Font.setTypeface(font, typeface);
    Canvas.drawText(
      canvas,
      "Times New Roman (System)",
      GlyphId,
      10.,
      40.,
      font,
      fill,
    );
  };

  let maybeTypeface =
    FontManager.matchFamilyStyle(fontManager, "Consolas", style);
  switch (maybeTypeface) {
  | None =>
    print_endline(
      "Normal Consolas not found. Ensure your system has it available.",
    )
  | Some(typeface) =>
    Font.setTypeface(font, typeface);
    let metrics = FontMetrics.make();
    let _ret: float = Font.getFontMetrics(font, metrics);
    print_endline("__Consolas__");
    print_endline(
      "-- Average character width: "
      ++ string_of_float(FontMetrics.getAvgCharacterWidth(metrics)),
    );
    print_endline(
      "-- Maximum character width: "
      ++ string_of_float(FontMetrics.getMaxCharacterWidth(metrics)),
    );
  };

  let filePath = Sys.getcwd() ++ "/examples/skia-cli/FiraCode-Regular.ttf";
  print_endline("Loading font: " ++ filePath);
  let maybeTypeface = Typeface.makeFromFile(filePath, 0);
  switch (maybeTypeface) {
  | None => failwith("Unable to load font: " ++ filePath)
  | Some(typeface) =>
    Font.setTypeface(font, typeface);
    let metrics = FontMetrics.make();
    let _ret: float = Font.getFontMetrics(font, metrics);
    print_endline("__Fira Code__");
    print_endline(
      "-- Average character width: "
      ++ string_of_float(FontMetrics.getAvgCharacterWidth(metrics)),
    );
    print_endline(
      "-- Maximum character width: "
      ++ string_of_float(FontMetrics.getMaxCharacterWidth(metrics)),
    );
  };
};

let surface = makeSurface(640l, 480l);
let canvas = Surface.getCanvas(surface);
draw(canvas);
emitPng("skia-font-manager-output.png", surface);
print_endline("Done!");
