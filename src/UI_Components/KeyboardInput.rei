open Revery_UI;

type keyDownFunction = NodeEvents.keyEventParams => unit;

/**
{2 Description:}

This component is a helper for getting input from the keyboard, outside of a `<Input />`
component.

{2 Usage:}

{[
<KeyboardInput onKeyDown={(key) => print_endline("Key: " ++ Key.toString(key))}>>
  <Container width=100 height=100 />
</KeyboardInput>
]}
 */
let createElement:
  (
    ~onKeyDown: keyDownFunction=?,
    ~children: list(Revery_UI.React.syntheticElement),
    unit
  ) =>
  Revery_UI.React.syntheticElement;
