/*
 * Slider.re
 *
 * This module provides a `<Slider />` component, loosely inspired by the
 * `<Slider />` component in React-Native, but geared towards the desktop.
 */

open Revery_Math;
open Revery_Core;
open Revery_UI;
open Revery_UI_Primitives;

module Hooks = Revery_UI_Hooks;

type clickFunction = unit => unit;
let noop = () => ();

type valueChangedFunction = float => unit;
let noopValueChanged = _f => ();

let defaultBoundingBox = BoundingBox2d.create(0., 0., 1., 1.);

let%component make =
              (
                ~onValueChanged=noopValueChanged,
                ~minimumValue=0.,
                ~maximumValue=1.,
                ~initialValue=0.,
                ~value=?,
                ~vertical=false,
                ~thumbLength=15,
                ~sliderLength=100,
                ~thumbThickness=15,
                ~trackThickness=5,
                ~maximumTrackColor=Colors.darkGray,
                ~minimumTrackColor=Color.hex("#90f7ff"),
                ~thumbColor=Colors.gray,
                (),
              ) => {
  /*let%hook (slideRef, setSlideRefOption) = Hooks.state(None);
    let%hook (thumbRef, setThumbRefOption) = Hooks.state(None);*/
  let%hook isActive = Hooks.ref(false);
  let%hook (sliderBoundingBox, setSliderBoundingBox) =
    Hooks.state(defaultBoundingBox);
  let%hook (v, setV) = Hooks.state(initialValue);

  let origV = v;
  let v =
    switch (value) {
    | Some(controlledValue) => controlledValue
    | None => origV
    };

  let availableWidth = {
    let (sliderX0, sliderY0, sliderX1, sliderY1) =
      BoundingBox2d.getBounds(sliderBoundingBox);

    let sliderWidth = vertical ? sliderY1 -. sliderY0 : sliderX1 -. sliderX0;
    let thumbWidth = float(thumbLength);
    sliderWidth -. thumbWidth;
  };

  let sliderUpdate = (w, startPosition, endPosition, mouseX, mouseY) => {
    let mousePosition = vertical ? mouseY : mouseX;
    let thumbPosition =
      clamp(
        mousePosition -. float(thumbLength) /. 2.,
        startPosition,
        endPosition,
      )
      -. startPosition;

    let normalizedValue =
      thumbPosition /. w *. (maximumValue -. minimumValue) +. minimumValue;
    setV(_ => normalizedValue);
    onValueChanged(normalizedValue);
  };

  let sliderComplete = () => {
    isActive := false;
    setV(v => v);
  };

  let%hook () =
    Hooks.effect(
      Always,
      () => {
        let isCaptured = isActive^;

        if (isCaptured) {
          let (x0, y0, _x1, _y1) =
            BoundingBox2d.getBounds(sliderBoundingBox);
          let startPosition = vertical ? y0 : x0;
          let endPosition = startPosition +. availableWidth;

          Mouse.setCapture(
            ~onMouseMove=
              evt =>
                sliderUpdate(
                  availableWidth,
                  startPosition,
                  endPosition,
                  evt.mouseX,
                  evt.mouseY,
                ),
            ~onMouseUp=_evt => sliderComplete(),
            (),
          );
        };

        Some(
          () =>
            if (isCaptured) {
              Mouse.releaseCapture();
            },
        );
      },
    );

  let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    let (x0, y0, _, _) = BoundingBox2d.getBounds(sliderBoundingBox);
    let startPosition = vertical ? y0 : x0;
    let endPosition = startPosition +. availableWidth;
    isActive := true;
    sliderUpdate(
      availableWidth,
      startPosition,
      endPosition,
      evt.mouseX,
      evt.mouseY,
    );
  };

  let sliderBackgroundColor = maximumTrackColor;

  let sliderOpacity = isActive^ ? 1.0 : 0.8;

  let sliderHeight = max(thumbThickness, trackThickness);
  let trackHeight = trackThickness;
  let thumbHeight = thumbThickness;
  let trackMargins = (sliderHeight - trackHeight) / 2;

  let thumbPosition =
    int_of_float(
      (v -. minimumValue) /. (maximumValue -. minimumValue) *. availableWidth,
    )
    - thumbLength
    / 2;

  let style =
    Style.[
      width(vertical ? sliderHeight : sliderLength),
      height(vertical ? sliderLength : sliderHeight),
      cursor(MouseCursors.pointer),
    ];

  let thumbWidth = thumbLength;

  let beforeTrackStyle =
    Style.[
      top(vertical ? 0 : trackMargins),
      bottom(
        vertical
          ? sliderLength - thumbPosition - thumbWidth / 2 : trackMargins,
      ),
      left(vertical ? trackMargins : 0),
      right(
        vertical
          ? trackMargins : sliderLength - thumbPosition - thumbWidth / 2,
      ),
      position(`Absolute),
      backgroundColor(minimumTrackColor),
    ];

  let afterTrackStyle =
    Style.[
      top(vertical ? thumbPosition + thumbWidth * 3 / 2 : trackMargins),
      bottom(vertical ? 0 : trackMargins),
      left(vertical ? trackMargins : thumbPosition + thumbWidth * 3 / 2),
      right(vertical ? trackMargins : 0),
      position(`Absolute),
      backgroundColor(sliderBackgroundColor),
    ];

  <Opacity opacity=sliderOpacity>
    <View
      onMouseDown
      style
      onBoundingBoxChanged={bbox => setSliderBoundingBox(_ => bbox)}>
      <View style=beforeTrackStyle />
      <View
        onBoundingBoxChanged={bbox => setThumbBoundingBox(_ => bbox)}
        style=Style.[
          position(`Absolute),
          height(vertical ? thumbWidth : thumbHeight),
          width(vertical ? thumbHeight : thumbWidth),
          left(vertical ? 0 : thumbPosition + thumbWidth / 2),
          top(vertical ? thumbPosition + thumbWidth / 2 : 0),
          backgroundColor(thumbColor),
        ]
      />
      <View style=afterTrackStyle />
    </View>
  </Opacity>;
};
