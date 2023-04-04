import {Renderer, el} from '@elemaudio/core';


// This example demonstrates writing a very simple chorus effect in Elementary, with a
// custom Renderer instance that marshals our instruction batches through the __postNativeMessage__
// function injected into the environment from C++.
//
// Below, we establish a __receiveStateChange__ callback function which will be invoked by the
// native PluginProcessor when any relevant state changes and we need to update our signal graph.
let core = new Renderer(__getSampleRate__(), (batch) => {
  __postNativeMessage__(JSON.stringify(batch));
});

// Our main signal processing function
function chorus(props, xn) {
  let rate = el.sm(el.const({key: 'rate', value: 0.001 + props.rate}));
  let depth = el.sm(el.const({key: 'depth', value: 10 + 20 * props.depth}));

  let sr = __getSampleRate__();
  let wet = el.delay(
    {size: sr * 100 / 1000},
    el.ms2samps(el.add(20, el.mul(depth, 0.5), el.mul(0.5, depth, el.triangle(rate)))),
    0,
    xn,
  );

  return el.mul(Math.sqrt(2) / 2, el.add(xn, wet));
}

// Our state change callback
globalThis.__receiveStateChange__ = (state) => {
  const props = JSON.parse(state);

  let stats = core.render(
    chorus(props, el.in({channel: 0})),
    chorus(props, el.in({channel: 1})),
  );

  console.log(stats);
};

