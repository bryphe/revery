module SyntaxHighlight: {
  type highlight;
  type t = (~language: string, list(string)) => list(list(highlight));

  let makeHighlight:
    (
      ~byteIndex: int,
      ~color: Revery_Core.Color.t,
      ~bold: bool,
      ~italicized: bool
    ) =>
    highlight;
};

let make:
  (
    ~key: Brisk_reconciler.Key.t=?,
    ~markdown: string=?,
    ~fontFamily: Revery_Font.Family.t=?,
    ~codeFontFamily: Revery_Font.Family.t=?,
    ~baseFontSize: float=?,
    ~paragraphStyle: list(Revery_UI.Style.textStyleProps)=?,
    ~activeLinkStyle: list(Revery_UI.Style.textStyleProps)=?,
    ~inactiveLinkStyle: list(Revery_UI.Style.textStyleProps)=?,
    ~h1Style: list(Revery_UI.Style.textStyleProps)=?,
    ~h2Style: list(Revery_UI.Style.textStyleProps)=?,
    ~h3Style: list(Revery_UI.Style.textStyleProps)=?,
    ~h4Style: list(Revery_UI.Style.textStyleProps)=?,
    ~h5Style: list(Revery_UI.Style.textStyleProps)=?,
    ~h6Style: list(Revery_UI.Style.textStyleProps)=?,
    ~inlineCodeStyle: list(Revery_UI.Style.textStyleProps)=?,
    ~codeBlockBackgroundColor: Revery_Core.Color.t=?,
    ~syntaxHighlighter: SyntaxHighlight.t=?,
    unit
  ) =>
  Brisk_reconciler.element(Revery_UI.React.node);
