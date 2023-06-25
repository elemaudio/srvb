import React, { useState, useEffect, useRef } from 'react';
import { useDrag } from '@use-gesture/react'


export default function DragBehavior(props) {
  const nodeRef = useRef();
  const valueAtDragStartRef = useRef(props.value || 0);

  const {snapToMouseLinearHorizontal, value, onChange, ...other} = props;

  const bindDragHandlers = useDrag((state) => {
    if (state.first && typeof value === 'number') {
      valueAtDragStartRef.current = value;

      if (snapToMouseLinearHorizontal) {
        let [x, y] = state.xy;
        let posInScreen = nodeRef.current.getBoundingClientRect();

        let dx = x - posInScreen.left;
        let dv = dx / posInScreen.width;

        valueAtDragStartRef.current = Math.max(0, Math.min(1, dv));

        if (typeof onChange === 'function') {
          onChange(Math.max(0, Math.min(1, dv)));
        }
      }

      return;
    }

    let [dx, dy] = state.movement;
    let dv = (dx - dy) / 200;

    if (typeof onChange === 'function') {
      onChange(Math.max(0, Math.min(1, valueAtDragStartRef.current + dv)));
    }
  });

  return (
    <div ref={nodeRef} className="touch-none" {...bindDragHandlers()} {...other}>
      {props.children}
    </div>
  );
}
