type wrapType =
  | NoWrap
  | Wrap
  | WrapIgnoreWhitespace
  | WrapHyphenate;

let wrapText = (~text, ~measureWidth, ~maxWidth, ~mode) => {
  switch (mode) {
  | NoWrap => [text]
  | Wrap => Wrap.wrap(text, ~max_width=maxWidth, ~width_of_char=measureWidth)
  | WrapIgnoreWhitespace =>
    Wrap.wrap(
      text,
      ~max_width=maxWidth,
      ~width_of_char=measureWidth,
      ~ignore_preceding_whitespace=false,
    )
  | WrapHyphenate =>
    Wrap.wrap(
      text,
      ~max_width=maxWidth,
      ~width_of_char=measureWidth,
      ~hyphenate=true,
    )
  };
};
