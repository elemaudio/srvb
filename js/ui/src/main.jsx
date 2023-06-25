import React from 'react'
import ReactDOM from 'react-dom/client'
import Interface from './Interface.jsx'

import createHooks from 'zustand'
import createStore from 'zustand/vanilla'

import './index.css'


// Initial state management
const store = createStore(() => {});
const useStore = createHooks(store);


// Mount the interface
function App(props) {
  let state = useStore();

  let requestStateUpdate = (callback) => store.setState(callback(state));
  let requestParamValueUpdate = (name, value) => core.dispatch('setParameterValue', name, value);

  return (
    <Interface
      {...state}
      requestParamValueUpdate={requestParamValueUpdate}
      requestStateUpdate={requestStateUpdate} />
  );
}

ReactDOM.createRoot(document.getElementById('root')).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
)
