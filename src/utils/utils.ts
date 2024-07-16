import type { Vec } from "@thi.ng/vectors";
import type { UI_Slider, NEL_ControlsMap, NEL_Preset } from "../../types";
import type { InstancedMesh, Object3D } from "three";


// This function extracts the 'value' property from each UI_Slider
// object in the UI_ControlsMap and returns them as an array of type Vec.
export function extractValuesFrom(view: NEL_ControlsMap): Vec {
  return Array.from(view.values()).map((slider) => slider.value) as Vec;
}

 // Helper function to get the value from the tuple that results from deserializing the host state object
 export function getValueFromEntries(entries: any, key: string): number {
  let result = 0;
  try {
    result = entries.find(([k]: [string]) => k === key)[1];
  } catch { (e:any) =>
    console.warn("Error setting state entries on key ", key, e.message);
  }
  return result;
}

// filter out params that are not registered with the host
export function onlyRegisteredParams(
  view: NEL_ControlsMap
): Map<string, UI_Slider> {
  let onlyRegisteredParams = new Map(
    [...view].filter(([key, value]) => value.isRegistered)
  );
  return onlyRegisteredParams;
}

// Convert an array of UI_ControlsMap to 
// serializable structure (array of objects)
export function serialisePresets(arrayOfPresets: Array<NEL_Preset>) {
  if (!arrayOfPresets) return {};
  let parametersMap: Map<string, UI_Slider>
  const result = arrayOfPresets.map((preset) => {
    if (preset === undefined) return {};
    parametersMap = preset.parameters;
    let obj: { [key: string]: UI_Slider } = {};
    preset.parameters.forEach((settings, key, map) => {
      obj[key] = settings;
    });
    return obj;
  });
  return result;
}

// Convert an array of objects to an array of UI_ControlsMap (Map<string, UI_Slider>)
export function deserialisePresets(
  arrayOfObjects: { [key: string]: UI_Slider }[]
) {
  const result = arrayOfObjects.map((presetObj) => {
    let presetMap = new Map<string, UI_Slider>();
    for (let key in presetObj) {
      presetMap.set(key, presetObj[key]);
    }
    return presetMap;
  });
  return result;
}

// sample and hold function
export function sampleAndHold(value: number, hold: boolean) {
  let heldValue: number = 0;
  return function () {
    if (hold) {
      heldValue = value;
    }
    return heldValue;
  };
}

// utility functions for generating the grid array and an iterator for it
export const fillArrayUsing = (
  arr: any[],
  start: number,
  end: number,
  step: number,
  value: number
) => {
  for (let i = start; i <= end; i += step) {
    arr.push(value);
  }
  return arr;
};

export const arrayIterator = (arr: any[]) => arr.values();

// utility functions for BufferGeometry transformations
export function scaleXYZ(vertices: Float32Array, scale: number) {
  const resizedVertices = vertices.map((v) => v * scale);
  return resizedVertices;
}

export function scaleWidth(vertices: Float32Array, scale: number) {
  const resizedVertices = new Float32Array(vertices.length);
  for (let i = 0; i < vertices.length; i += 3) {
    resizedVertices[i] = vertices[i] * scale;
    resizedVertices[i + 1] = vertices[i + 1];
    resizedVertices[i + 2] = vertices[i + 2];
  }
  return resizedVertices;
}

export function scaleHeight(vertices: Float32Array, scale: number) {
  const resizedVertices = new Float32Array(vertices.length);
  for (let i = 0; i < vertices.length; i += 3) {
    resizedVertices[i] = vertices[i];
    resizedVertices[i + 1] = vertices[i + 1] * scale;
    resizedVertices[i + 2] = vertices[i + 2];
  }
  return resizedVertices;
}

export function offsetXY(
  vertices: Float32Array,
  offsetX: number,
  offsetY: number
) {
  const offsetVertices = new Float32Array(vertices.length);

  for (let i = 0; i < vertices.length; i += 3) {
    offsetVertices[i] = vertices[i] + offsetX;
    offsetVertices[i + 1] = vertices[i + 1] + offsetY;
    offsetVertices[i + 2] = vertices[i + 2];
  }
  return offsetVertices;
}

export function retrieveNodeFromInstancedMesh (o: any | null, instanceId: number) : Object3D | null {
  if (o) {
        const parentMesh: InstancedMesh = o;
        const node: Object3D = parentMesh.instances.current[instanceId];
        return node;
  } 
    return null;
}

class ShiftRegister<T> {
  currentState: T | null;
  previousState: T | null;

  constructor(initialState: T | null) {
    this.currentState = initialState;
    this.previousState = null;
  }

  updateState(newState: T) {
    this.previousState = this.currentState;
    this.currentState = newState;
  }

  isStateChanged() {
    return this.currentState !== this.previousState;
  }

  fastCompare(): boolean {
    if (this.currentState === null || this.previousState === null) { return true; }
    return JSON.stringify(this.currentState) !== JSON.stringify(this.previousState);
  }
}

export function shiftRegister<T>(initialState: T | null) {
  return new ShiftRegister<T>(initialState);
}