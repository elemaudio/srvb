import React from 'react'
import ReactDOM from 'react-dom/client'
import Interface from './Interface.jsx'

import createHooks from 'zustand'
import createStore from 'zustand/vanilla'

import './index.css'


// Initial state management
const store = createStore(() => {});
const useStore = createHooks(store);

// Interop bindings
function requestParamValueUpdate(paramId, value) {
  if (typeof globalThis.__postNativeMessage__ === 'function') {
    globalThis.__postNativeMessage__("setParameterValue", {
      paramId,
      value,
    });
  }
}

globalThis.__receiveStateChange__ = function(state) {
  store.setState(JSON.parse(state));
};

// Mount the interface
function App(props) {
  let state = useStore();

  return (
    <Interface {...state} requestParamValueUpdate={requestParamValueUpdate} />
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
