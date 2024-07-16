//@ts-nocheck
/** typescript disabled for now */

//#region IMPORTS 
import type {
  LocalManifest,
  HostParameter,
  NEL_ControlsMap,
  NEL_NodeUserData,
} from "../types/index";


//#endregion


//#region CONSTANTS 
export const UNCHECKED_LICENSE_STATUS = "Checking";
export const LICENSE_VALIDATED = "Valid";
 // UI MANIFEST
  // 📍 Extended version of public/manifest.json --- keep them in sync!
export const manifest: LocalManifest = {
  window: { width: 575, height: 930 },
  parameters: [
    { paramId: "blend", name: "Blend", min: 0.0, max: 1.0, defaultValue: 0.0, group: "shifter" },
    { paramId: "shift", name: "Shift", min: -1.0, max: 1.0, defaultValue: 0.0, group: "shifter" },
    { paramId: "type", name: "Type", min: -1.0, max: 1.0, defaultValue: -1.0, group: "shifter" },
    { paramId: "climb", name: "Climb", min: -1.0, max: 1.0, defaultValue: -1.0, group: "feedback" },
    { paramId: "ladder", name: "Ladder", min: 0.0, max: 1.0, defaultValue: 0.0, group: "feedback" },
    { paramId: "stride", name: "Stride", min: 0.0, max: 1.0, defaultValue: 0.25, group: "feedback" },
    { paramId: "gain", name: "Gain", min: 0.0, max: 1.0, defaultValue: 0.25, group: "feedback" },
    { paramId: "size", name: "Size", min: 0.0, max: 1.0, defaultValue: 0.125, group: "reverb" },
    { paramId: "decay", name: "Decay", min: 0.0, max: 1.0, defaultValue: 0.15, group: "reverb" },
    { paramId: "mod", name: "Mod", min: 0.0, max: 1.0, defaultValue: 0.125, group: "reverb" },
    { paramId: "mix", name: "Mix", min: 0.0, max: 1.0, defaultValue: 0.25, group: "reverb" },
  ]
};
export const NUMBER_HOSTPARAMS = manifest.parameters.length;
//#endregion



//#region LICENSE ACTIVATION 

export const LicenseStatus: string = $state(UNCHECKED_LICENSE_STATUS);

//#endregion


//#region AUDIO PARAMS 
//---- registered audio parameters -------------------
export const ParamDefsHost: HostParameter[] = $state( manifest.parameters);
// registered audio parameters for UI
export const ParamIds: string[] = $state(manifest.parameters.map((p: HostParameter) => p.paramId));

//#endregion

//#region UI STATE MACHINES,  CONTROLS 


// we only need to store the user data, not the whole mesh
export const UI_NodesData: NEL_NodeUserData[] = $state([]); 
export const focusedNodeParams: NEL_ControlsMap = $state(new Map());


// Explicit refresh of the stored presets
export const UI_StoredPresetsRefresh: boolean = $state(false);
export const UI_needsUpdate: boolean = $state(true);
export const UI_asyncReady: boolean = $state(false);
// Sidebar controls
export const UI_Controls: NEL_ControlsMap = $state(new Map());

//#endregion

//#region RAYCASTER & PICKING 

// Global export of current RayCast target
export const CurrentPickedId: number = $state(0);
export const CurrentFocusId: number = $state(0);
export const ActiveId: number = $state(0);
export const MouseDownWindow: boolean = $state(false);

//#endregion

//#region USER INTERACTION 

export const ConsoleText: string = $state("Console:");
export const ShiftPressed: boolean = $state(false);
export const BillboardText: string = $state("");  

//#endregion

//#region HOST STATE 
export const MeshStateIncoming: any = $state();
export const HostState: Map<string, any> = $state( new Map());
export const ErrorStore: any = $state();

//#endregion

//#region MISC 

export const PixelDensity: number = $state(2);
export const HostInfo: string = $state('');

//#endregion