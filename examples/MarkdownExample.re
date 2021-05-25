open Revery;
open Revery.UI;
open Revery.UI.Components;

module Styles = {
  open Style;

  let outer = [
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
    margin(6),
    justifyContent(`FlexStart),
    alignItems(`FlexStart),
  ];

  let whiteText = [color(Colors.white)];
  let linkActive = [color(Colors.blue)];
  let linkInactive = [color(Colors.lightBlue)];
};

let markdown =
  Markdown.make(
    ~paragraphStyle=Styles.whiteText,
    ~h1Style=Styles.whiteText,
    ~h2Style=Styles.whiteText,
    ~h3Style=Styles.whiteText,
    ~h4Style=Styles.whiteText,
    ~h5Style=Styles.whiteText,
    ~h6Style=Styles.whiteText,
    ~activeLinkStyle=Styles.linkActive,
    ~inactiveLinkStyle=Styles.linkInactive,
  );

let example = () =>
  <View style=Styles.outer>
    <markdown
      markdown={markdown|
An h2 header
------------

![Test Image](https://raw.githubusercontent.com/revery-ui/revery/master/assets/logo.png)

Here's a numbered list:

 1. first item
 2. second item
 3. third item

Note again how the actual text starts at 4 columns in (4 characters
from the left side). Here's a code sample:

    # Let me re-iterate ...
    for i in 1 .. 10 { do-something(i) }

As you probably guessed, indented 4 spaces. By the way, instead of
indenting the block, you can use delimited blocks, if you like:

~~~
define foobar() {
    print "Welcome to flavor country!";
}
~~~

(which makes copying & pasting easier). You can optionally mark the
delimited block for Pandoc to syntax highlight it:

~~~python
import time
# Quick, count to ten!
for i in range(10):
    # (but not *too* quick)
    time.sleep(0.5)
    print(i)
~~~
    |markdown}
    />
  </View>;

let render = () => <example />;
