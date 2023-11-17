import {Renderer, el} from '@elemaudio/core';
import {RefMap} from './RefMap';
import srvb from './srvb';


// This project demonstrates writing a small FDN reverb effect in Elementary.
//
// First, we initialize a custom Renderer instance that marshals our instruction
// batches through the __postNativeMessage__ function to direct the underlying native
// engine.
let core = new Renderer((batch) => {
  __postNativeMessage__(JSON.stringify(batch));
});

// Next, a RefMap for coordinating our refs
let refs = new RefMap(core);

// Holding onto the previous state allows us a quick way to differentiate
// when we need to fully re-render versus when we can just update refs
let prevState = null;

function shouldRender(prevState, nextState) {
  return (prevState === null) || (prevState.sampleRate !== nextState.sampleRate);
}

// The important piece: here we register a state change callback with the native
// side. This callback will be hit with the current processor state any time that
// state changes.
//
// Given the new state, we simply update our refs or perform a full render depending
// on the result of our `shouldRender` check.
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

// Finally, an error callback which just logs back to native
globalThis.__receiveError__ = (err) => {
  console.log(`[Error: ${err.name}] ${err.message}`);
};
