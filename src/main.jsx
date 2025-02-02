import React from 'react'
import ReactDOM from 'react-dom/client'
import Interface from './Interface.jsx'

import { useStore } from 'zustand'
import { createStore } from 'zustand/vanilla'

import './index.css'


// Initial state management
const store = createStore(() => { });
const eventStore = createStore(() => { });
const errorStore = createStore(() => ({ error: null }));

// Interop bindings
function requestParamValueUpdate(paramId, value) {
  if (typeof globalThis.__postNativeMessage__ === 'function') {
    globalThis.__postNativeMessage__("setParameterValue", {
      paramId,
      value,
    });
  }
}

if (process.env.NODE_ENV !== 'production') {
  import.meta.hot.on('reload-dsp', () => {
    console.log('Sending reload dsp message');

    if (typeof globalThis.__postNativeMessage__ === 'function') {
      globalThis.__postNativeMessage__('reload');
    }
  });
}

globalThis.__receiveStateChange__ = function (state) {
  store.setState(JSON.parse(state));
};

globalThis.__receiveGraphEvents__ = function(eventBatch) {
  console.log(JSON.parse(eventBatch));
};
globalThis.__receiveError__ = (err) => {
  errorStore.setState({ error: err });
};

// Mount the interface
function App(props) {
  let state = useStore(store);
  let { error } = useStore(errorStore);

  return (
    <Interface
      {...state}
      error={error}
      requestParamValueUpdate={requestParamValueUpdate}
      resetErrorState={() => errorStore.setState({ error: null })} />
  );
}

ReactDOM.createRoot(document.getElementById('root')).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
)

// Request initial processor state
if (typeof globalThis.__postNativeMessage__ === 'function') {
  globalThis.__postNativeMessage__("ready");
}
