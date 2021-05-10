/*
 * KeyboardInput.re
 */

open Revery_UI;
open Revery_Core;
open Revery_Math;
open Revery_UI_Primitives;

module Hooks = Revery_UI_Hooks;

type keyDownFunction = NodeEvents.keyEventParams => unit;

let noop = _ => ();

type action =
  | Focused(bool)
  | SetRef(node);

type state = {
  ref: option(node),
  hasFocus: bool,
};

let reducer = (action, state) =>
  switch (action) {
  | Focused(v) => {...state, hasFocus: v}
  | SetRef(v) => {...state, ref: Some(v)}
  };

let component = React.component("KeyboardInput");

let make =
    (~onKeyDown: keyDownFunction=noop, children: React.syntheticElement) =>
  component(hooks => {
    let (v, dispatch, hooks) =
      Hooks.reducer(
        ~initialState={ref: None, hasFocus: false},
        reducer,
        hooks,
      );

    let hooks =
      Hooks.effect(
        Always,
        () => {
          if (!v.hasFocus) {
            switch (v.ref) {
            | Some(v) => Focus.focus(v)
            | None => ()
            };
          };
          None;
        },
        hooks,
      );

    let onBlur = () => {
      dispatch(Focused(false));
    };

    let onFocus = () => {
      dispatch(Focused(true));
    };

    (
      hooks,
      <View ref={r => dispatch(SetRef(r))} onBlur onFocus onKeyDown>
        children
      </View>,
    );
  });

let createElement = (~onKeyDown=?, ~children, ()) =>
  make(~onKeyDown?, React.listToElement(children));
