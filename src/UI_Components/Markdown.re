open Revery_Core;
open Revery_UI;
open Revery_UI_Primitives;
open Revery_Font;

module Hooks = Revery_UI_Hooks;

open Omd;

module Log = (val Log.withNamespace("Revery.Components.Markdown"));

module StringSet = Set.Make(String);

// Since we dont have rich text support, sometimes links get broken
// up by style. This causes only part of them to hover/activate at
// a time. We create our own link system to fix this, where all links
// with a certain href are highlighted at a time.
type state = {hoveredLinks: StringSet.t};

type events =
  | LinkHovered(string)
  | LinkUnhovered(string);

let update = (event, state) =>
  switch (event) {
  | LinkHovered(href) => {
      hoveredLinks: StringSet.add(href, state.hoveredLinks),
    }
  | LinkUnhovered(href) => {
      hoveredLinks: StringSet.remove(href, state.hoveredLinks),
    }
  };

type style = {
  paragraph: list(Style.textStyleProps),
  activeLink: list(Style.textStyleProps),
  inactiveLink: list(Style.textStyleProps),
  inlineCode: list(Style.textStyleProps),
  h1: list(Style.textStyleProps),
  h2: list(Style.textStyleProps),
  h3: list(Style.textStyleProps),
  h4: list(Style.textStyleProps),
  h5: list(Style.textStyleProps),
  h6: list(Style.textStyleProps),
  fontFamily: Family.t,
  codeFontFamily: Family.t,
  codeBlockFontSize: float,
  baseFontSize: float,
  codeBlock: list(Style.viewStyleProps),
  imageElement: (~url: string) => Revery_UI.element,
};

module SyntaxHighlight = {
  type highlight = {
    byteIndex: int,
    color: Color.t,
    bold: bool,
    italic: bool,
  }
  and t =
    (~language: option(string), list(string)) => list(list(highlight));

  let makeHighlight = (~byteIndex, ~color, ~bold, ~italic): highlight => {
    byteIndex,
    color,
    bold,
    italic,
  };

  let default: t =
    (~language as _, lines) => {
      List.init(List.length(lines), _ =>
        [{byteIndex: 0, color: Colors.white, bold: false, italic: false}]
      );
    };
};

module Styles = {
  open Style;

  let inline = [
    flexDirection(`Row),
    flexWrap(`Wrap),
    alignItems(`FlexStart),
  ];
  let container = [justifyContent(`FlexStart), alignItems(`FlexStart)];
  let hardBreak = [height(6)];

  module Blockquote = {
    let container = [
      flexDirection(`Row),
      borderLeft(~width=4, ~color=Colors.grey),
    ];
    let contents = [
      paddingLeft(8),
      justifyContent(`FlexStart),
      alignItems(`FlexStart),
      flexGrow(1),
    ];
  };

  module Code = {
    let inlineContainer = [
      backgroundColor(Color.rgba(0., 0., 0., 0.25)),
      borderRadius(3.),
      border(~width=3, ~color=Color.rgba(0., 0., 0., 0.25)),
    ];

    let defaultBlock = [
      backgroundColor(Color.rgba(0., 0., 0., 0.25)),
      border(~width=3, ~color=Color.rgba(0., 0., 0., 0.25)),
      borderRadius(3.),
      padding(2),
      marginTop(4),
      marginBottom(4),
    ];
  };

  module List = {
    let marker = [
      marginRight(6),
      textWrap(TextWrapping.WrapIgnoreWhitespace),
    ];
    let contents = [
      justifyContent(`FlexStart),
      alignItems(`FlexStart),
      flexGrow(1),
    ];
  };

  module ThematicBreak = {
    let hr = [
      flexGrow(1),
      height(2),
      marginTop(2),
      marginBottom(2),
      backgroundColor(Color.rgba(0., 0., 0., 0.25)),
    ];
  };
};

type inlineAttrs =
  | Italicized
  | Bolded
  | Monospaced;

type kind = [
  | `Paragraph
  | `Heading(int)
  | `Image(string)
  | `Link(string)
  | `InlineCode
];

let selectStyleFromKind = (kind: kind, styles) =>
  switch (kind) {
  | `Heading(1) => styles.h1
  | `Heading(2) => styles.h2
  | `Heading(3) => styles.h3
  | `Heading(4) => styles.h4
  | `Heading(5) => styles.h5
  | `Heading(6)
  | `Heading(_) => styles.h6
  | `InlineCode => styles.inlineCode
  | _ => styles.paragraph
  };

// Sourced from: http://zuga.net/articles/html-heading-elements/
let fontSizeFromKind = (kind: kind, styles) =>
  switch (kind) {
  | `Heading(1) => styles.baseFontSize *. 2.
  | `Heading(2) => styles.baseFontSize *. 1.5
  | `Heading(3) => styles.baseFontSize *. 1.17
  | `Heading(4) => styles.baseFontSize *. 1.
  | `Heading(5) => styles.baseFontSize *. 0.83
  | `Heading(6)
  | `Heading(_) => styles.baseFontSize *. 0.67
  | `InlineCode => styles.baseFontSize *. 0.75
  | _ => styles.baseFontSize
  };

type attrs = {
  inline: list(inlineAttrs),
  kind,
};

let isBold = attrs => List.mem(Bolded, attrs.inline);
let isItalicized = attrs => List.mem(Italicized, attrs.inline);
let isMonospaced = attrs => List.mem(Monospaced, attrs.inline);

let generateText = (text, styles, attrs, dispatch, state) => {
  let fontSize = fontSizeFromKind(attrs.kind, styles);
  let fontWeight = {
    isBold(attrs) ? Weight.Bold : Weight.Normal;
  };

  switch (attrs.kind) {
  | `Link(href) =>
    let onMouseEnter = _ => dispatch(LinkHovered(href));
    let onMouseLeave = _ => dispatch(LinkUnhovered(href));
    let onMouseUp = _ => Revery_Native.Shell.openURL(href) |> ignore;
    <View onMouseEnter onMouseLeave onMouseUp>
      <Text
        text
        style=Style.[
          textWrap(TextWrapping.WrapIgnoreWhitespace),
          ...{StringSet.mem(href, state.hoveredLinks)
                ? styles.activeLink : styles.inactiveLink},
        ]
        fontSize
        fontFamily={
          isMonospaced(attrs) ? styles.codeFontFamily : styles.fontFamily
        }
        fontWeight
        italic={isItalicized(attrs)}
        underlined=true
      />
    </View>;
  | `InlineCode =>
    <View style=Styles.Code.inlineContainer>
      <Text
        text
        fontSize
        fontFamily={styles.codeFontFamily}
        fontWeight
        style=Style.[
          textWrap(TextWrapping.WrapIgnoreWhitespace),
          ...{selectStyleFromKind(attrs.kind, styles)},
        ]
        italic={isItalicized(attrs)}
      />
    </View>
  | `Image(url) => styles.imageElement(~url)
  | _ =>
    <Text
      text
      fontSize
      fontFamily={
        isMonospaced(attrs) ? styles.codeFontFamily : styles.fontFamily
      }
      fontWeight
      style=Style.[
        textWrap(TextWrapping.WrapIgnoreWhitespace),
        ...selectStyleFromKind(attrs.kind, styles),
      ]
      italic={isItalicized(attrs)}
    />
  };
};

let rec generateInline' = (inline, styles, attrs, dispatch, state) => {
  switch (inline) {
  | Html(t)
  | Text(t) => generateText(t, styles, attrs, dispatch, state)
  | Emph(e) =>
    generateInline'(
      e.content,
      styles,
      switch (e.style) {
      | Star => {...attrs, inline: [Bolded, ...attrs.inline]}
      | Underscore => {...attrs, inline: [Italicized, ...attrs.inline]}
      },
      dispatch,
      state,
    )
  | Soft_break => generateText(" ", styles, attrs, dispatch, state)
  | Hard_break => <View style=Styles.hardBreak />
  | Ref(r) =>
    generateInline'(
      r.label,
      styles,
      {...attrs, kind: `Link(r.def.destination)},
      dispatch,
      state,
    )
  | Link(l) =>
    generateInline'(
      l.def.label,
      styles,
      {
        ...attrs,
        kind:
          l.kind == Url
            ? `Link(l.def.destination) : `Image(l.def.destination),
      },
      dispatch,
      state,
    )
  | Code(c) =>
    generateText(
      c.content,
      styles,
      {kind: `InlineCode, inline: [Monospaced, ...attrs.inline]},
      dispatch,
      state,
    )
  | Concat(c) =>
    <View style=Styles.inline>
      {c
       |> List.map(il => generateInline'(il, styles, attrs, dispatch, state))
       |> React.listToElement}
    </View>
  | _ => <View />
  };
}
and generateInline = (inline, styles, attrs, dispatch, state) =>
  <Row> {generateInline'(inline, styles, attrs, dispatch, state)} </Row>;

/* Block level elements

     These include things like code blocks, paragraphs, etc.
   */

let thematicBreak =
  <View style=Style.[flexDirection(`Row)]>
    <View style=Styles.ThematicBreak.hr />
  </View>;

let rec generateMarkdown' = (element, styles, highlighter, dispatch, state) =>
  switch (element) {
  | Paragraph(p) =>
    generateInline(
      p,
      styles,
      {inline: [], kind: `Paragraph},
      dispatch,
      state,
    )
  // We don't support HTML rendering as of right now, so we'll just render it
  // as text
  | Html_block(html) =>
    generateInline(
      Text(html),
      styles,
      {inline: [], kind: `Paragraph},
      dispatch,
      state,
    )
  | Blockquote(blocks) =>
    generateBlockquote(blocks, styles, highlighter, dispatch, state)
  | Heading(h) =>
    generateInline(
      h.text,
      styles,
      {inline: [Bolded], kind: `Heading(h.level)},
      dispatch,
      state,
    )
  | Code_block(cb) => generateCodeBlock(cb, styles, highlighter)
  | List(blist) => generateList(blist, styles, highlighter, dispatch, state)
  | Thematic_break => thematicBreak
  | _ => <View />
  }
and generateList =
    (
      blist: Block_list.t(block(inline)),
      styles,
      highlighter,
      dispatch,
      state,
    ) => {
  <View>
    {List.mapi(
       (i, blocks) => {
         let text =
           switch (blist.kind) {
           | Ordered(_, _) => string_of_int(i + 1) ++ "."
           | Unordered(_) => "•"
           };
         <View style=Styles.inline>
           <Text
             text
             style=Styles.List.marker
             fontFamily={styles.fontFamily}
           />
           <View style=Styles.List.contents>
             {List.map(
                block =>
                  generateMarkdown'(
                    block,
                    styles,
                    highlighter,
                    dispatch,
                    state,
                  ),
                blocks,
              )
              |> React.listToElement}
           </View>
         </View>;
       },
       blist.blocks,
     )
     |> React.listToElement}
  </View>;
}
and generateBlockquote = (blocks, styles, highlighter, dispatch, state) => {
  <View style=Styles.Blockquote.container>
    <View style=Styles.Blockquote.contents>
      {List.map(
         block =>
           generateMarkdown'(block, styles, highlighter, dispatch, state),
         blocks,
       )
       |> React.listToElement}
    </View>
  </View>;
}
and generateCodeBlock =
    (codeBlock: Code_block.t, styles, highlighter: SyntaxHighlight.t) => {
  // Not sure why this is an `option` because when there is no label, the
  // Parser gives `Some("")`. For our purposes it's easier to make that `None`
  let label =
    switch (codeBlock.label) {
    | Some("")
    | None => None
    | Some(_) as label => label
    };

  Log.tracef(m =>
    m("Code block has label : %s", Option.value(label, ~default="(none)"))
  );

  <View style={styles.codeBlock}>
    {switch (codeBlock.code) {
     | Some(code) =>
       let lines = String.split_on_char('\n', code);
       let highlights = highlighter(~language=label, lines);
       List.map2(
         (line, highlight) => {
           <View style=Styles.inline>
             {List.mapi(
                (i, block: SyntaxHighlight.highlight) => {
                  // We want to style this block up to the next one.
                  // If there is a next block, stop at it's index
                  // Otherwise, stop at the end of the string.
                  let endIndex =
                    switch (List.nth_opt(highlight, i + 1)) {
                    | Some(blk: SyntaxHighlight.highlight) => blk.byteIndex
                    | None => String.length(line)
                    };
                  let length = endIndex - block.byteIndex;
                  let text = String.sub(line, block.byteIndex, length);
                  <Text
                    text
                    style=Style.[
                      textWrap(TextWrapping.WrapIgnoreWhitespace),
                      color(block.color),
                    ]
                    fontFamily={styles.codeFontFamily}
                    fontWeight={block.bold ? Weight.Bold : Weight.Normal}
                    fontSize={styles.codeBlockFontSize}
                  />;
                },
                highlight,
              )
              |> React.listToElement}
           </View>
         },
         lines,
         highlights,
       )
       |> React.listToElement;

     | None => <View />
     }}
  </View>;
};

let generateMarkdown = (mdText: string, styles, highlighter, dispatch, state) => {
  let md = Omd.of_string(mdText);
  Log.debugf(m => m("Parsed Markdown as: %s", Omd.to_sexp(md)));
  List.map(
    elt => generateMarkdown'(elt, styles, highlighter, dispatch, state),
    md,
  )
  |> React.listToElement;
};

let defaultImageRenderer = (~url as _) => {
  <View
    style=Style.[width(64), height(64), backgroundColor(Colors.gray)]
  />;
};

let%component make =
              (
                ~markdown as mdText="",
                ~fontFamily=Family.default,
                ~codeFontFamily=Family.defaultMono,
                ~baseFontSize=14.0,
                ~codeBlockFontSize=baseFontSize,
                ~paragraphStyle=Style.emptyTextStyle,
                ~activeLinkStyle=Style.emptyTextStyle,
                ~inactiveLinkStyle=Style.emptyTextStyle,
                ~h1Style=Style.emptyTextStyle,
                ~h2Style=Style.emptyTextStyle,
                ~h3Style=Style.emptyTextStyle,
                ~h4Style=Style.emptyTextStyle,
                ~h5Style=Style.emptyTextStyle,
                ~h6Style=Style.emptyTextStyle,
                ~inlineCodeStyle=Style.emptyTextStyle,
                ~codeBlockStyle=Styles.Code.defaultBlock,
                ~imageElement=defaultImageRenderer,
                ~syntaxHighlighter=SyntaxHighlight.default,
                (),
              ) => {
  let%hook (state, setState) = Hooks.state({hoveredLinks: StringSet.empty});

  let dispatch = event => setState(s => update(event, s));

  <View style=Styles.container>
    {generateMarkdown(
       mdText,
       {
         paragraph: paragraphStyle,
         activeLink: activeLinkStyle,
         inactiveLink: inactiveLinkStyle,
         inlineCode: inlineCodeStyle,
         h1: h1Style,
         h2: h2Style,
         h3: h3Style,
         h4: h4Style,
         h5: h5Style,
         h6: h6Style,
         fontFamily,
         codeFontFamily,
         baseFontSize,
         codeBlockFontSize,
         codeBlock: codeBlockStyle,
         imageElement,
       },
       syntaxHighlighter,
       dispatch,
       state,
     )}
  </View>;
};
