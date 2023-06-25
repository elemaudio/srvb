import React, { useState } from 'react';

import Knob from './Knob.jsx';
import Lockup from './Lockup_Dark2.svg';

import manifest from '../../manifest.json';


// The interface of our plugin, exported here as a React.js function
// component. The host may use this component to render the interface of our
// plugin where appropriate.
//
// We use the `props.requestParamValueUpdate` callback provided by the caller
// of this function to propagate new parameter values to the host.
export default function Interface(props) {
  const colorProps = {
    meterColor: '#EC4899',
    knobColor: '#64748B',
    thumbColor: '#F8FAFC',
  };

  let params = manifest.parameters.map(({paramId, name, min, max, defaultValue}) => {
    let currentValue = props[name] || 0;

    return {
      paramId,
      name,
      value: currentValue,
      readout: `${Math.round(currentValue * 100)}%`,
      setValue: (v) => props.requestParamValueUpdate(name, v),
    };
  });

  return (
    <div className="w-full h-screen min-w-[492px] min-h-[238px] bg-slate-800 bg-mesh p-8">
      <div className="h-1/5 flex justify-between items-center text-md text-slate-400">
        <img src={Lockup} className="h-8 w-auto" />
        <span className="font-bold">SRVB</span>
      </div>
      <div className="flex h-4/5">
        {params.map(({name, value, readout, setValue}) => (
          <div key={name} className="flex flex-col flex-1 justify-center items-center">
            <Knob className="h-20 w-20 m-4" value={value} onChange={setValue} {...colorProps} />
            <div className="flex-initial mt-2">
              <div className="text-sm text-slate-50 text-center font-light">{name}</div>
              <div className="text-sm text-pink-500 text-center font-light">{readout}</div>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
