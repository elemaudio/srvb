import React, { useState, useEffect, useRef } from 'react';
import ResizeObserver from 'resize-observer-polyfill';

import DragBehavior from './DragBehavior';


function cx(...classes) {
  return classes.filter(Boolean).join(' ')
}

function draw(ctx, width, height, value, meterColor, knobColor, thumbColor) {
  ctx.clearRect(0, 0, width, height);

  const hw = width * 0.5;
  const hh = height * 0.5;
  const radius = Math.min(hw, hh) * 0.8;

  // Fill
  ctx.strokeStyle = meterColor;
  ctx.lineWidth = Math.round(width * 0.028);
  ctx.lineCap = 'round';

  const fillStart = 0.75 * Math.PI;
  const fillEnd = fillStart + (1.5 * value * Math.PI);

  ctx.beginPath();
  ctx.arc(hw, hh, radius, fillStart, fillEnd, false);
  ctx.stroke();

  // Knob
  ctx.strokeStyle = knobColor;
  ctx.lineWidth = Math.round(width * 0.028);
  ctx.lineCap = 'round';

  ctx.beginPath();
  ctx.arc(hw, hh, radius * 0.72, 0, 2 * Math.PI, false);
  ctx.stroke();

  // Knob thumb
  ctx.fillStyle = thumbColor;
  ctx.lineWidth = Math.round(width * 0.036);
  ctx.lineCap = 'round';

  ctx.beginPath();
  ctx.arc(hw + 0.5 * radius * Math.cos(fillEnd), hh + 0.5 * radius * Math.sin(fillEnd), radius * 0.08, 0, 2 * Math.PI, false);
  ctx.fill();
}

function Knob(props) {
  const canvasRef = useRef();
  const observerRef = useRef();

  const [bounds, setBounds] = useState({
    width: 0,
    height: 0,
  });

  let {className, meterColor, knobColor, thumbColor, ...other} = props;
  let classes = cx(className, 'relative touch-none');

  useEffect(function() {
    const canvas = canvasRef.current;

    observerRef.current = new ResizeObserver(function(entries) {
      for (let entry of entries) {
        setBounds({
          width: 2 * entry.contentRect.width,
          height: 2 * entry.contentRect.height,
        });
      }
    });

    observerRef.current.observe(canvas);

    return function() {
      observerRef.current.disconnect();
    };
  }, []);

  useEffect(function() {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');

    canvas.width = bounds.width;
    canvas.height = bounds.height;

    draw(ctx, bounds.width, bounds.height, props.value, meterColor, knobColor, thumbColor);
  }, [bounds, props.value, meterColor, knobColor, thumbColor]);

  return (
    <DragBehavior className={classes} {...other}>
      <canvas className="absolute h-full w-full touch-none" ref={canvasRef} />
    </DragBehavior>
  );
}

export default React.memo(Knob);
