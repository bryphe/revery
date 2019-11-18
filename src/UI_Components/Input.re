module ContainerComponent = Container;
open Revery_UI;
open Revery_Core;
open Revery_UI_Primitives;

module Hooks = Revery_UI_Hooks;

module Cursor = {
  type state = {
    time: Time.t,
    isOn: bool,
  };

  type action =
    | Reset
    | Tick(Time.t);

  let use = (~interval, ~isFocused) => {
    let%hook (state, dispatch) =
      Hooks.reducer(
        ~initialState={time: Time.zero, isOn: false}, (action, state) => {
        switch (action) {
        | Reset => {isOn: true, time: Time.zero}
        | Tick(increasedTime) =>
          let newTime = Time.(state.time + increasedTime);

          /* if newTime is above the interval a `Tick` has passed */
          newTime >= interval
            ? {isOn: !state.isOn, time: Time.zero}
            : {...state, time: newTime};
        }
      });

    let%hook () =
      Hooks.effect(
        OnMount,
        () => {
          let clear =
            Tick.interval(time => dispatch(Tick(time)), Time.ms(16));
          Some(clear);
        },
      );

    let cursorOpacity = isFocused && state.isOn ? 1.0 : 0.0;

    (cursorOpacity, () => dispatch(Reset));
  };
};

type state = {
  isFocused: bool,
  internalValue: string,
  cursorPosition: int,
};

type textUpdate = {
  newString: string,
  cursorPosition: int,
};

type cursorUpdate = {
  inputString: string,
  change: int,
};
type changeEvent = {
  value: string,
  character: string,
  keycode: Key.Keycode.t,
  altKey: bool,
  ctrlKey: bool,
  shiftKey: bool,
  superKey: bool,
};

type action =
  | CursorPosition(cursorUpdate)
  | SetFocus(bool)
  | UpdateText(textUpdate);

let getStringParts = (index, str) => {
  switch (index) {
  | 0 => ("", str)
  | _ =>
    let strBeginning =
      try(Str.string_before(str, index)) {
      | _ => str
      };
    let strEnd =
      try(Str.string_after(str, index)) {
      | _ => ""
      };
    (strBeginning, strEnd);
  };
};

let getSafeStringBounds = (str, cursorPosition, change) => {
  let nextPosition = cursorPosition + change;
  let currentLength = String.length(str);
  nextPosition > currentLength
    ? currentLength : nextPosition < 0 ? 0 : nextPosition;
};

let removeCharacterBefore = (word, cursorPosition) => {
  let (startStr, endStr) = getStringParts(cursorPosition, word);
  let nextPosition = getSafeStringBounds(startStr, cursorPosition, -1);
  let newString = Str.string_before(startStr, nextPosition) ++ endStr;
  {newString, cursorPosition: nextPosition};
};

let removeCharacterAfter = (word, cursorPosition) => {
  let (startStr, endStr) = getStringParts(cursorPosition, word);
  let newString =
    startStr
    ++ (
      switch (endStr) {
      | "" => ""
      | _ => Str.last_chars(endStr, String.length(endStr) - 1)
      }
    );
  {newString, cursorPosition};
};

let addCharacter = (word, char, index) => {
  let (startStr, endStr) = getStringParts(index, word);
  {
    newString: startStr ++ char ++ endStr,
    cursorPosition: String.length(startStr) + 1,
  };
};
let reducer = (action, state) =>
  switch (action) {
  | SetFocus(isFocused) => {...state, isFocused}
  | CursorPosition({inputString, change}) => {
      ...state,
      cursorPosition:
        getSafeStringBounds(inputString, state.cursorPosition, change),
    }
  | UpdateText({newString, _}) =>
    state.isFocused
      ? {...state, isFocused: true, internalValue: newString} : state
  };

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

let%component make =
              (
                ~style=defaultStyles,
                ~placeholderColor=Colors.grey,
                ~cursorColor=Colors.black,
                ~autofocus=false,
                ~placeholder="",
                ~onFocus as focusCallback=() => (),
                ~onBlur as blurCallback=() => (),
                ~onKeyDown=_ => (),
                ~onChange=_ => (),
                ~value as valueAsProp=?,
                (),
              ) => {
  let%hook (state, dispatch) =
    Hooks.reducer(
      ~initialState={
        internalValue: "",
        cursorPosition:
          switch (valueAsProp) {
          | Some(v) => String.length(v)
          | None => 0
          },
        isFocused: false,
      },
      reducer,
    );

  let valueToDisplay =
    switch (valueAsProp) {
    | Some(v) => v
    | None => state.internalValue
    };

  let%hook (cursorOpacity, resetCursor) =
    Cursor.use(~interval=Time.ms(500), ~isFocused=state.isFocused);

  let%hook (inputValueRef, setInputValueRef) = Hooks.ref(valueToDisplay);

  let%hook () =
    Hooks.effect(
      If((!=), valueToDisplay),
      () => {
        let oldValueLength = String.length(inputValueRef);
        let newValueLength = String.length(valueToDisplay);
        switch (abs(oldValueLength - newValueLength)) {
        | lengthDiff when lengthDiff != 1 =>
          // Set cursor at the end
          dispatch(
            CursorPosition({
              inputString: valueToDisplay,
              change: String.length(valueToDisplay) - state.cursorPosition,
            }),
          )
        | _ =>
          let (oldStart, _oldEnd) =
            getStringParts(state.cursorPosition, inputValueRef);
          let (newStart, _newEnd) =
            getStringParts(state.cursorPosition, valueToDisplay);

          if (oldStart == newStart) {
            if (newValueLength > oldValueLength)
              {
                // One character was added
                dispatch(
                  CursorPosition({inputString: valueToDisplay, change: 1}),
                );
              };
              // One characted was removed from the right side of the cursor (Delete key)
          } else {
            // One character was removed from the left side of the cursor (Backspace)
            dispatch(
              CursorPosition({inputString: valueToDisplay, change: (-1)}),
            );
          };
        };
        setInputValueRef(valueToDisplay);
        None;
      },
    );

  let handleTextInput = (event: NodeEvents.textInputEventParams) => {
    let createChangeEvent = value => {
      value,
      keycode: 0,
      character: event.text,
      altKey: false,
      ctrlKey: false,
      shiftKey: false,
      superKey: false,
    };

    resetCursor();

    switch (valueAsProp) {
    | Some(v) =>
      let {newString, _} = addCharacter(v, event.text, state.cursorPosition);
      onChange(createChangeEvent(newString));
    | None =>
      let {newString, cursorPosition} =
        addCharacter(state.internalValue, event.text, state.cursorPosition);
      dispatch(UpdateText({newString, cursorPosition}));
      onChange(createChangeEvent(newString));
    };
  };

  let handleKeyDown = (event: NodeEvents.keyEventParams) => {
    let character = Key.Keycode.getName(event.keycode);
    let createChangeEvent = inputString => {
      value: inputString,
      character,
      keycode: event.keycode,
      altKey: Key.Keymod.isAltDown(event.keymod),
      ctrlKey: Key.Keymod.isControlDown(event.keymod),
      shiftKey: Key.Keymod.isShiftDown(event.keymod),
      superKey: Key.Keymod.isGuiDown(event.keymod),
    };

    resetCursor();

    switch (event.keycode) {
    | v when Key.Keycode.left == v =>
      onKeyDown(event);
      dispatch(CursorPosition({inputString: valueToDisplay, change: (-1)}));
    | v when Key.Keycode.right == v =>
      onKeyDown(event);
      dispatch(CursorPosition({inputString: valueToDisplay, change: 1}));
    | v when Key.Keycode.delete == v =>
      // We should manage both cases
      removeCharacterAfter(valueToDisplay, state.cursorPosition)
      |> (
        update => {
          switch (valueAsProp) {
          | Some(_) => ()
          | None => dispatch(UpdateText(update))
          };
          onKeyDown(event);
          onChange(createChangeEvent(update.newString));
        }
      )

    | v when Key.Keycode.backspace == v =>
      removeCharacterBefore(valueToDisplay, state.cursorPosition)
      |> (
        update => {
          switch (valueAsProp) {
          | Some(_) => ()
          | None => dispatch(UpdateText(update))
          };
          onKeyDown(event);
          onChange(createChangeEvent(update.newString));
        }
      )
    | v when Key.Keycode.escape == v =>
      onKeyDown(event);
      Focus.loseFocus();
    | _ => onKeyDown(event)
    };
  };

  let hasPlaceholder = String.length(valueToDisplay) < 1;

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

  let cursor = {
    let (startStr, _) = getStringParts(state.cursorPosition, valueToDisplay);
    let dimension =
      Revery_Draw.Text.measure(
        ~window=Revery_UI.getActiveWindow(),
        ~fontFamily=inputFontFamily,
        ~fontSize=inputFontSize,
        startStr,
      );
    let inputHeight = Selector.select(style, Height, defaultHeight);
    <View
      style=Style.[
        position(`Absolute),
        marginLeft(dimension.width + inputTextMargin + 1),
        marginTop((inputHeight - dimension.height) / 2),
      ]>
      <Opacity opacity=cursorOpacity>
        <ContainerComponent width=2 height=inputFontSize color=cursorColor />
      </Opacity>
    </View>;
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
  <Clickable
    onFocus={() => {
      resetCursor();
      dispatch(SetFocus(true));
      focusCallback();
      Sdl2.TextInput.start();
    }}
    onBlur={() => {
      resetCursor();
      dispatch(SetFocus(false));
      blurCallback();
      Sdl2.TextInput.stop();
    }}
    componentRef={autofocus ? Focus.focus : ignore}
    onKeyDown=handleKeyDown
    onTextInput=handleTextInput>
    <View style=viewStyles>
      cursor
      {hasPlaceholder ? placeholderText : inputText}
    </View>
  </Clickable>;
};
