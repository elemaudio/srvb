import {Renderer, el} from '@elemaudio/core';
import invariant from 'invariant';
import srvb from './srvb';


class RefMap {
  constructor(core) {
    this._map = new Map();
    this._core = core;
  }

  getOrCreate(name, type, props, children) {
    if (!this._map.has(name)) {
      let ref = this._core.createRef(type, props, children);
      this._map.set(name, ref);
    }

    return this._map.get(name)[0];
  }

  update(name, props) {
    invariant(this._map.has(name), "Oops, trying to update a ref that doesn't exist");

    let [node, setter] = this._map.get(name);
    setter(props);
  }
}

// This example demonstrates writing a very simple chorus effect in Elementary, with a
// custom Renderer instance that marshals our instruction batches through the __postNativeMessage__
// function injected into the environment from C++.
//
// Below, we establish a __receiveStateChange__ callback function which will be invoked by the
// native PluginProcessor when any relevant state changes and we need to update our signal graph.
let core = new Renderer((batch) => {
  __postNativeMessage__(JSON.stringify(batch));
});

let refs = new RefMap(core);

// Holding onto the previous state allows us a simple way to differentiate
// when we need to fully re-render versus when we can just update refs
let prevState = null;

function shouldRender(prevState, nextState) {
  return (prevState === null) || (prevState.sampleRate !== nextState.sampleRate);
}

// Our state change callback
globalThis.__receiveStateChange__ = (serializedState) => {
  const state = JSON.parse(serializedState);

  if (shouldRender(prevState, state)) {
    let stats = core.render(...srvb({
      key: 'srvb',
      sampleRate: state.sampleRate,
      size: refs.getOrCreate('size', 'const', {value: state.size}, []),
      decay: refs.getOrCreate('decay', 'const', {value: state.decay}, []),
      mod: refs.getOrCreate('mod', 'const', {value: state.mod}, []),
      mix: refs.getOrCreate('mix', 'const', {value: state.mix}, []),
    }, el.in({channel: 0}), el.in({channel: 1})));

    console.log(stats);
  } else {
    refs.update('size', {value: state.size});
    refs.update('decay', {value: state.decay});
    refs.update('mod', {value: state.mod});
    refs.update('mix', {value: state.mix});
  }

  prevState = state;
};

// And an error callback
globalThis.__receiveError__ = (err) => {
  console.log(`[Error: ${err.name}] ${err.message}`);
};
