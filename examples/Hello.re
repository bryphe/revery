open Revery;
open Revery.Math;
open Revery.UI;
open Revery.UI.Components;

module Logo = {
  let%component make = () => {
    let%hook (logoOpacity, setOpacity) = Hooks.state(1.0);

    let%hook rotation =
      Hooks.animation(
        Animated.floatValue(0.),
        Animated.options(
          ~toValue=6.28,
          ~duration=Seconds(8.),
          ~delay=Seconds(1.0),
          ~repeat=true,
          (),
        ),
      );

    let%hook rotationY =
      Hooks.animation(
        Animated.floatValue(0.),
        Animated.options(
          ~toValue=6.28,
          ~duration=Seconds(4.),
          ~delay=Seconds(0.5),
          ~repeat=true,
          (),
        ),
      );

    let onMouseDown = _ => setOpacity(_ => 0.5);

    let onMouseUp = _ => setOpacity(_ => 1.0);

    <View onMouseDown onMouseUp>
      <Opacity opacity=logoOpacity>
        <Image
          src="outrun-logo.png"
          style=Style.[
            width(512),
            height(256),
            transform([
              Transform.RotateY(Angle.from_radians(rotationY)),
              Transform.RotateX(Angle.from_radians(rotation)),
            ]),
          ]
        />
      </Opacity>
    </View>;
  };
};

module AnimatedText = {
  let%component make = (~text: string, ~delay: float, ()) => {
    let%hook animatedOpacity =
      Hooks.animation(
        Animated.floatValue(0.),
        Animated.options(
          ~toValue=1.0,
          ~duration=Seconds(1.),
          ~delay=Seconds(delay),
          (),
        ),
      );

    let%hook translate =
      Hooks.animation(
        Animated.floatValue(50.),
        Animated.options(
          ~toValue=1.0,
          ~duration=Seconds(0.5),
          ~delay=Seconds(delay),
          (),
        ),
      );

    let textHeaderStyle =
      Style.[
        color(Colors.white),
        fontFamily("Roboto-Regular.ttf"),
        fontSize(24),
        marginHorizontal(8),
        transform([Transform.TranslateY(translate)]),
      ];

    <Opacity opacity=animatedOpacity>
      <Text style=textHeaderStyle text />
    </Opacity>;
  };
};

let render = () => {
  <Center>
    <Row>
      <AnimatedText delay=0.0 text="Welcome" />
      <AnimatedText delay=0.5 text="to" />
      <AnimatedText delay=1. text="Revery" />
    </Row>
    <Logo />
  </Center>;
};
