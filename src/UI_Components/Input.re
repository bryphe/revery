open Revery_UI;
open Revery_Core;

type state = {
  isFocused: bool,
  internalValue: string,
  cursorOffset: int,
  cursorTimer: Time.t,
};

// type textUpdate = {
//   internalValue: string,
//   cursorOffset: int,
// };

type changeEvent = {
  value: string,
  character: string,
  key: Key.t,
  altKey: bool,
  ctrlKey: bool,
  shiftKey: bool,
  superKey: bool,
};

type action =
  | SetOffset(int)
  | CursorTimer
  | SetFocus(bool)
  | SetValue(string)
  | ResetCursorTimer;

let getTextBeforeCursor = (cursorOffset, text) =>
  String.sub(text, 0, String.length(text) + cursorOffset);
let getTextAfterCursor = (cursorOffset, text) =>
  String.sub(
    text,
    String.length(text) + cursorOffset,
    Pervasives.abs(cursorOffset),
  );

let removeCharacterBefore = (offset, word) => {
  let wordLength = String.length(word);
  switch (wordLength, offset) {
  | (0, offset)
  | (_, offset) when Pervasives.abs(offset) == wordLength => word
  | (_, _) =>
    getTextBeforeCursor(offset - 1, word) ++ getTextAfterCursor(offset, word)
  };
};

let removeCharacterAfter = (offset, word) => {
  let wordLength = String.length(word);
  switch (wordLength, offset) {
  | (0, _)
  | (_, 0) => word
  | (_, _) =>
    getTextBeforeCursor(offset, word) ++ getTextAfterCursor(offset + 1, word)
  };
};

// let getStringParts = (index, str) =>
//   switch (index) {
//   | 0 => ("", str)
//   | _ =>
//     let strBeginning = Str.string_before(str, index);
//     let strEnd = Str.string_after(str, index);
//     (strBeginning, strEnd);
//   };

// let getSafeStringBounds = (str, cursorPosition, change) => {
//   let nextPosition = cursorPosition + change;
//   let currentLength = String.length(str);
//   nextPosition > currentLength
//     ? currentLength : nextPosition < 0 ? 0 : nextPosition;
// };

// let removeCharacterBefore = (word, cursorPosition) => {
//   let (startStr, endStr) = getStringParts(cursorPosition, word);
//   let nextPosition = getSafeStringBounds(startStr, cursorPosition, -1);
//   let newString = Str.string_before(startStr, nextPosition) ++ endStr;
//   {newString, cursorPosition: nextPosition};
// };

// let removeCharacterAfter = (word, cursorPosition) => {
//   let (startStr, endStr) = getStringParts(cursorPosition, word);
//   let newString =
//     startStr
//     ++ (
//       switch (endStr) {
//       | "" => ""
//       | _ => Str.last_chars(endStr, String.length(endStr) - 1)
//       }
//     );
//   {newString, cursorPosition};
// };

// let addCharacter = (word, char, index) => {
//   let (startStr, endStr) = getStringParts(index, word);
//   {
//     newString: startStr ++ char ++ endStr,
//     cursorPosition: String.length(startStr) + 1,
//   };
// };

let addCharacter = (word, char, offset) =>
  getTextBeforeCursor(offset, word)
  ++ char
  ++ getTextAfterCursor(offset, word);

let reducer = (action, state) =>
  /*
     TODO: Handle Cursor position changing via keyboard input e.g. arrow keys
     potentially draw the cursor Inside the text element and render the text around the cursor
   */
  switch (action) {
  | SetFocus(isFocused) => {...state, isFocused}
  | SetOffset(cursorOffset) => {...state, cursorOffset}
  | SetValue(internalValue) => {...state, internalValue}
  // | CursorPosition(pos) => {
  //     ...state,
  //     cursorPosition:
  //       getSafeStringBounds(state.inputString, state.cursorPosition, pos),
  //   }
  | CursorTimer => {
      ...state,
      cursorTimer:
        state.cursorTimer >= Time.Seconds(1.0)
          ? Time.Seconds(0.0)
          : Time.increment(state.cursorTimer, Time.Seconds(0.1)),
    }
  // | UpdateText({newString, cursorPosition}) =>
  //   state.isFocused
  //     ? {...state, cursorPosition, isFocused: true, inputString: newString}
  //     : state
  | ResetCursorTimer => {...state, cursorTimer: Time.Seconds(0.0)}
  };

// let noop = (~value as _value) => ();
let defaultHeight = 50;
let defaultWidth = 200;
let inputTextMargin = 10;

let defaultStyles =
  Style.[
    color(Colors.black),
    width(defaultWidth),
    height(defaultHeight),
    border(
      /*
         The default border width should be 5% of the full input height
       */
      ~width=float_of_int(defaultHeight) *. 0.05 |> int_of_float,
      ~color=Colors.black,
    ),
    backgroundColor(Colors.transparentWhite),
  ];

let component = React.component("Input");
let make =
    (
      ~style,
      ~autofocus,
      ~placeholder,
      ~cursorColor,
      ~placeholderColor,
      ~onChange,
      ~onKeyDown,
      ~value as valueAsProp,
      (),
    ) =>
  component(slots => {
    let (state, dispatch, slots) =
      React.Hooks.reducer(
        ~initialState={
          internalValue: "",
          cursorOffset: 0,
          cursorTimer: Time.Seconds(0.0),
          isFocused: false,
        },
        reducer,
        slots,
      );

    let slots =
      React.Hooks.effect(
        OnMount,
        () => {
          let clear =
            Tick.interval(_ => dispatch(CursorTimer), Seconds(0.1));
          Some(clear);
        },
        slots,
      );

    let valueToDisplay =
      switch (valueAsProp) {
      | Some(v) => v
      | None => state.internalValue
      };

    let handleKeyPress = (event: NodeEvents.keyPressEventParams) => {
      let createChangeEvent = value => {
        value,
        key: Key.fromString(event.character),
        character: event.character,
        altKey: false,
        ctrlKey: false,
        shiftKey: false,
        superKey: false,
      };

      dispatch(ResetCursorTimer);

      switch (valueAsProp) {
      | Some(v) =>
        addCharacter(v, event.character, state.cursorOffset)
        |> createChangeEvent
        |> onChange
      | None =>
        let newValue =
          addCharacter(
            state.internalValue,
            event.character,
            state.cursorOffset,
          );
        dispatch(SetValue(newValue));
        onChange(createChangeEvent(newValue));
      };
    };

    let handleKeyDown = (event: NodeEvents.keyEventParams) => {
      let createChangeEvent = inputString => {
        value: inputString,
        character: Key.toString(event.key),
        key: event.key,
        altKey: event.altKey,
        ctrlKey: event.ctrlKey,
        shiftKey: event.shiftKey,
        superKey: event.superKey,
      };

      dispatch(ResetCursorTimer);

      switch (event.key) {
      | Key.KEY_LEFT =>
        onKeyDown(event);
        dispatch(
          SetOffset(
            Pervasives.max(
              - String.length(valueToDisplay),
              state.cursorOffset - 1,
            ),
          ),
        );
      | Key.KEY_RIGHT =>
        onKeyDown(event);
        dispatch(SetOffset(Pervasives.min(0, state.cursorOffset + 1)));
      | Key.KEY_DELETE =>
        switch (valueAsProp) {
        | Some(v) =>
          removeCharacterAfter(state.cursorOffset, v)
          |> createChangeEvent
          |> onChange
        | None =>
          let newValue =
            removeCharacterAfter(state.cursorOffset, state.internalValue);
          dispatch(SetValue(newValue));
          onChange(createChangeEvent(newValue));
        };
        dispatch(SetOffset(Pervasives.min(0, state.cursorOffset + 1)));
      | Key.KEY_BACKSPACE =>
        switch (valueAsProp) {
        | Some(v) =>
          removeCharacterBefore(state.cursorOffset, v)
          |> createChangeEvent
          |> onChange
        | None =>
          let newValue =
            removeCharacterBefore(state.cursorOffset, state.internalValue);
          dispatch(SetValue(newValue));
          onChange(createChangeEvent(newValue));
        }
      | Key.KEY_ESCAPE =>
        onKeyDown(event);
        Focus.loseFocus();
      | _ => onKeyDown(event)
      };
    };

    let hasPlaceholder = String.length(valueToDisplay) < 1;

    // let content = hasPlaceholder ? placeholder : valueToDisplay;

    /*
       computed styles
     */

    let allStyles =
      Style.(
        merge(
          ~source=[
            flexDirection(`Row),
            alignItems(`Center),
            justifyContent(`FlexStart),
            overflow(`Hidden),
            cursor(MouseCursors.text),
            ...defaultStyles,
          ],
          ~target=style,
        )
      );

    let viewStyles = Style.extractViewStyles(allStyles);

    let inputFontSize = Selector.select(style, FontSize, 18);
    let inputColor = Selector.select(style, Color, Colors.black);
    let inputFontFamily =
      Selector.select(style, FontFamily, "Roboto-Regular.ttf");

    let cursorOpacity =
      state.isFocused
      |> (
        fun
        | true => state.cursorTimer <= Time.Seconds(0.5) ? 1.0 : 0.0
        | false => 0.0
      );

    /**
      We place these in a list so we change the order later to
      render the cursor before the text if placeholder is present
      otherwise to the cursor after
     */
    let cursor = {
      // let (startStr, _) = getStringParts(cursorPosition, inputString);
      let startStr = getTextBeforeCursor(state.cursorOffset, valueToDisplay);
      let dimension =
        Revery_Draw.Text.measure(
          ~fontFamily=inputFontFamily,
          ~fontSize=inputFontSize,
          startStr,
        );
      <View
        style=Style.[
          width(2),
          marginTop((defaultHeight - dimension.height) / 2),
          height(inputFontSize),
          position(`Absolute),
          marginLeft(dimension.width + inputTextMargin + 1),
          opacity(cursorOpacity),
          backgroundColor(cursorColor),
        ]
      />;
    };

    let makeTextComponent = content =>
      <Text
        text=content
        style=Style.[
          color(hasPlaceholder ? placeholderColor : inputColor),
          fontFamily(inputFontFamily),
          fontSize(inputFontSize),
          alignItems(`Center),
          justifyContent(`FlexStart),
          marginLeft(inputTextMargin),
        ]
      />;

    let placeholderText = makeTextComponent(placeholder);
    let inputText = makeTextComponent(valueToDisplay);

    /*
       component
     */
    (
      slots,
      <Clickable
        onFocus={() => {
          dispatch(ResetCursorTimer);
          dispatch(SetFocus(true));
        }}
        onBlur={() => {
          dispatch(ResetCursorTimer);
          dispatch(SetFocus(false));
        }}
        componentRef={autofocus ? Focus.focus : ignore}
        onKeyDown=handleKeyDown
        onKeyPress=handleKeyPress>
        <View style=viewStyles>
          cursor
          {hasPlaceholder ? placeholderText : inputText}
        </View>
      </Clickable>,
    );
  });

let createElement =
    (
      ~children as _,
      ~style=defaultStyles,
      ~placeholderColor=Colors.grey,
      ~cursorColor=Colors.black,
      ~autofocus=false,
      ~placeholder="",
      ~onKeyDown=_ => (),
      ~onChange=_ => (),
      ~value=?,
      (),
    ) =>
  make(
    ~value,
    ~style,
    ~placeholder,
    ~autofocus,
    ~cursorColor,
    ~placeholderColor,
    ~onKeyDown,
    ~onChange,
    (),
  );