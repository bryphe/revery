module Color = Color;
module Colors = Colors;

module Window = Window;
module App = App;
module Time = Time;
module Monitor = Monitor;

module Event = Reactify.Event;

module Performance = Performance;

module Memoize = {
  type t('a, 'b) = 'a => 'b;

  let make = (f: t('a, 'b)): t('a, 'b) => {
    let tbl: Hashtbl.t('a, 'b) = Hashtbl.create(16);

    let ret = (arg: 'a) => {
      let cv = Hashtbl.find_opt(tbl, arg);
      switch (cv) {
      | Some(x) => x
      | None =>
        let r = f(arg);
        Hashtbl.add(tbl, arg, r);
        r;
      };
    };
    ret;
  };
};

module Lazy = {
  type t('a) = Memoize.t(unit, 'a);

  let make = (f: t('a)): t('a) => Memoize.make(f);
};
