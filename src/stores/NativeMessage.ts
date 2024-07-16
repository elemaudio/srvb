export declare var globalThis: any;

import { get } from "svelte/store";
import type {
  NEL_Preset,
  MessagesToHost,
} from "../../types";

import { getValueFromEntries } from "../utils/utils";
import {
  HostState,
  LicenseStatus,
  HostInfo,
  LICENSE_VALIDATED,
} from "./stores";


/** ━━━━━━━ generated doc ━━━━━━━
 * `processHostState` is a function that processes and denormalizes parameter values sent from the host.
 * It receives these values via the `__receiveStateChange__` global function.
 * @param state - The state object received from the host.
 *
 * `MessageToHost` is an object that contains callable functions for interfacing with the Host code.
 *
 * `stashMeshState` is a function that manually gets the current state key of each storage slot and serializes it for persistentState storage in the host plugin.
 *
 * `requestParamValueUpdate` is a function that updates parameter values in the host.
 * @param paramId - The ID of the parameter to update.
 * @param value - The new value of the parameter.
 *
 * `requestReady` is a function that sends a ready message to the host.
 *
 * `requestUnlock` is a function that sends an unlock message to the KeyZey handler.
 * @param serial - The serial number for unlocking.
 *
 * `__setMeshStateInHost` is a function that stores any persistent UI state in the host.
 * @param dataToPersist - The data to persist in the host.
 *
 * `__bindHotReload` is a function that hot reloads the DSP during development.
 *
 * `RegisterMessagesFromHost` is a function that registers the message handlers for receiving and processing messages from the host.
 * It sets up global functions that handle state changes, mesh state changes, license activation, error handling, and host information.
 */ 


/** ━━━━━━━
 * Main method for processing and denormalizing parameter values sent from the host
 * received via the `__receiveStateChange__` global function.
 * @param state - The state object received from the host.
 */

function processHostState(state: any) {
  // ━━━━━━━
  // Parse the state object and convert it to an array of key-value pairs
  let parsedEntries: Array<[string, any]> = [];
  try {
    parsedEntries = Object.entries(JSON.parse(state));
  } catch (e) {
    console.warn("Bad state received", parsedEntries);
  }
  const processedEntries: Map<string, number> = new Map(parsedEntries);
  // ━━━━━━━
  // denormalize the full range type, shift, and climb parameters
  processedEntries.set(
    "type",
    getValueFromEntries(processedEntries, "type") * 2 - 1
  );
  processedEntries.set(
    "shift",
    getValueFromEntries(processedEntries, "shift") * 2 - 1
  );
  processedEntries.set(
    "climb",
    getValueFromEntries(processedEntries, "climb") * 2 - 1
  );

  /** ━━━━━━━
   * Finally, assign the processed entries as Map<string, number> )
   * to the HostState store which triggers observer / subscribers
   * across the View code updating parameter values.
   **/
  HostState.set(new Map(processedEntries));
}

/** ━━━━━━━
 * Callable functions for interfacing with the Host code.
 **/
export const MessageToHost: MessagesToHost = {
  /** ━━━━━━━
   * Manually get the current state key of each storage slot and serialize for
   * persistentState storage in the host plugin. We need to stash and retrieve the view state
   * as the WebView is destroyed when the user closes the plugin window.
   * The solution must also work for preset store/recall originating from DAW Host.
   **/
  stashMeshState: function () {
    let dataToPersist = {
      nodeStates: get(UI_NodesData).map((data) => get(data.state) as string),
      presets: get(UI_NodesData).map((data) => {
        if (!data.preset) return {} as NEL_Preset;
        let parameters = Array.from(data.preset.parameters.entries());
        return { ...data.preset, parameters } as unknown as NEL_Preset;
      }),
      license: get(LicenseStatus),
      viewOptions: get(ViewOptions),
    };
    this.__setMeshStateInHost(dataToPersist);
  },

  /** ━━━━━━━
   * Update parameter values in the host.
   * @param paramId - The ID of the parameter to update.
   * @param value - The new value of the parameter.
   */
  requestParamValueUpdate: function (paramId: string, value: number) {
    if (typeof globalThis.__postNativeMessage__ === "function") {
      if (paramId === "type" || paramId === "shift" || paramId === "climb") {
        value = (value + 1) / 2;
      }
      globalThis.__postNativeMessage__("setParameterValue", {
        paramId,
        value,
      });
    }
  },

  /** ━━━━━━━
   * Send a ready message to the host.
   */
  requestReady: function () {
    if (typeof globalThis.__postNativeMessage__ === "function") {
      globalThis.__postNativeMessage__("ready", {});
    }
  },

  /** ━━━━━━━
   * Send an unlock message to the KeyZey handler.
   * @param serial - The serial number for unlocking.
   */
  requestUnlock: function (serial: string = "") {
    if (get(LicenseStatus) !== LICENSE_VALIDATED) {
      console.count("Requesting unlock");
      if (typeof globalThis.__postNativeMessage__ === "function") {
        globalThis.__postNativeMessage__("unlock", { serial });
      }
    }
  },

  /** ━━━━━━━
   * Store any persistent UI state in the host.
   * @param dataToPersist - The data to persist in the host.
   */
  __setMeshStateInHost: function (dataToPersist: any) {
    if (typeof globalThis.__postNativeMessage__ === "function") {
      globalThis.__postNativeMessage__(
        "setMeshState",
        JSON.stringify(dataToPersist)
      );
    }
  },

  /** ━━━━━━━
   * Hot reload the DSP during development.
   */
  __bindHotReload: function () {
    if (process.env.NODE_ENV !== "production") {
      import.meta.hot?.on("reload-dsp", () => {
        // console.log("Sending reload dsp message");

        if (typeof globalThis.__postNativeMessage__ === "function") {
          globalThis.__postNativeMessage__("reload");
        }
      });
    }
  },
};

/** ━━━━━━━
 * Registers the message handlers for receiving and processing messages from the host.
 * This function sets up global functions that handle state changes, mesh state changes,
 * license activation, error handling, and host information.
 */

export function RegisterMessagesFromHost() {
  /** ━━━━━━━
   * Handles the state change received from the host.
   * @param state - The host state change object.
   */
  globalThis.__receiveStateChange__ = function (state: any) {
    processHostState(state);
  };

  /** ━━━━━━━
   * Handles the mesh state change received from the host.
   * @param meshState - The mesh state object.
   */
  globalThis.__receiveMeshStateChange__ = function (meshState: any) {
    let incomingMeshState: IncomingMeshState;
    try {
      incomingMeshState = JSON.parse(JSON.parse(meshState).meshState);
    } catch (e) {
      // console.log("Invalid mesh state (rcv)", meshState, e);
      LicenseStatus.set("Mesh state error. Contact support.");
      return;
    }
    if (typeof incomingMeshState !== "object") return;

    if (!incomingMeshState.license) {
      MessageToHost.requestUnlock();
    } else {
      LicenseStatus.set(incomingMeshState.license);
      MessageToHost.stashMeshState();
    }

    if (incomingMeshState.nodeStates && incomingMeshState.presets) {
      MeshStateIncoming.set(incomingMeshState);
    }

    UI_needsUpdate.set(true);
  };

  /** ━━━━━━━
   * Handles the unlock status received from the host.
   * @param status - The unlock status string.
   */
  globalThis.__onUnlock__ = function (status: string | undefined) {
    LicenseStatus.set(status || "License not valid");
    if (status === LICENSE_VALIDATED) {
      globalThis.__postNativeMessage__("ready", {});
      MessageToHost.stashMeshState();
    }
  };

  /** ━━━━━━━
   * Handles the DAW name received from the host.
   * @param message - The host information object.
   */
  globalThis.__hostInfo__ = function (message: string) {
    HostInfo.set(message);
    // console.log("Host Info: ", message);
  };

  /** ━━━━━━━
   * DEV: Handles an error received from the host.
   * @param error - The error object.
   */
  globalThis.__receiveError__ = function (error: any) {
    ConsoleText.set("Error: " + error);
  };
}
 