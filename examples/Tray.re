open Revery;
open Revery.UI;
open Revery.UI.Components;

let%component make = () => {
  let%hook () =
    Hooks.effect(
      OnMount,
      () => {
        let trayImage =
          Native.Tray.make(
            ~imagePath=Environment.getAssetPath("outrun-logo.png"),
            (),
          );

        let trayText =
          Native.Tray.make() |> Native.Tray.setTitle(~text="Hello Revery!");

        Some(
          () => {
            trayImage |> Native.Tray.remove;
            trayText |> Native.Tray.remove;
          },
        );
      },
    );

  <Center>
    <Text
      text="You should see two items in your tray-bar. One image and one with
  the text \"Hello Revery!\""
    />
  </Center>;
};

let render = () => make();
