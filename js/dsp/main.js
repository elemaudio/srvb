import {Renderer, el} from '@elemaudio/core';
import srvb from './srvb';


// This example demonstrates writing a very simple chorus effect in Elementary, with a
// custom Renderer instance that marshals our instruction batches through the __postNativeMessage__
// function injected into the environment from C++.
//
// Below, we establish a __receiveStateChange__ callback function which will be invoked by the
// native PluginProcessor when any relevant state changes and we need to update our signal graph.
let core = new Renderer(__getSampleRate__(), (batch) => {
  __postNativeMessage__(JSON.stringify(batch));
});

// Our state change callback
globalThis.__receiveStateChange__ = (state) => {
  const props = JSON.parse(state);

  let stats = core.render(...srvb({
    key: 'srvb',
    ...props,
  }, el.in({channel: 0}), el.in({channel: 1})));

  console.log(stats);
};

// And an error callback
globalThis.__receiveError__ = (err) => {
  console.log(`[Error: ${err.name}] ${err.message}`);
};
