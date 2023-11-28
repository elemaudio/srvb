# SRVB

SRVB is a small digital reverb audio plugin (VST3/AU) for MacOS and Windows.

This project demonstrates one way to write an audio plugin using JavaScript and
familiar web technologies, and while there are several variants on this approach,
it is meant to be both a compelling example and a good starting point for audio
plugins made with a similar architecture.

## Overview

The software architecture in this plugin is much like [Tauri](https://tauri.app/) and similar to
[Electron](https://www.electronjs.org/). The user interface is a simple Vite, React, and Tailwind app
at the root of the repository, which is packaged into the plugin app bundle and loaded into a native
webview instance owned by the plugin at runtime.

The audio processing algorithm in the `dsp/` directory is also written in
JavaScript using [Elementary](https://elementary.audio), and is run in a separate
engine which directs the underlying native plugin audio processing. The native
plugin itself provides the harness for these two frontend JavaScript bundles,
and interfaces with the plugin host (typically a DAW) to coordinate the user
interface and the audio processing loop.

## Elementary

If you're new to Elementary Audio, [Elementary](https://elementary.audio) is a JavaScript/C++ library for building audio applications.

* **Declarative:** Elementary makes it simple to create interactive audio processes through functional, declarative programming. Describe your audio process as a function of your application state, and Elementary will efficiently update the underlying audio engine as necessary.
* **Dynamic:** Most audio processing frameworks and tools facilitate building static processes. But what happens as your audio requirements change throughout the user journey? Elementary is designed to facilitate and adapt to the dynamic nature of modern audio applications.
* **Portable:** By decoupling the JavaScript API from the underlying audio engine (the "what" from the "how"), Elementary enables writing portable applications. Whether the underlying engine is running in the browser, an audio plugin, or an embedded device, the JavaScript layer remains the same.

Find more in the [Elementary repository on GitHub](https://github.com/elemaudio/elementary) and the documentation [on the website](https://elementary.audio/).

## Getting Started

### Dependencies

Before running the following steps, please make sure you have the following dependencies installed and
available at the command line:

* [CMake](https://cmake.org/)
* [Node.js](https://nodejs.org/en)
* Bash: the build steps below expect to run scripts in a Bash environment. For Windows machines, consider running the following steps in a Git Bash environment, or with WSL.

Next, we fetch the SRVB project and its dependencies,

```bash
# Clone the project with its submodules
git clone --recurse-submodules https://github.com/elemaudio/srvb.git
cd srvb

# Install npm dependencies
npm install
```

### Develop
```bash
npm run dev
```

In develop mode, the native plugin is compiled to fetch its JavaScript assets from localhost, where subsequently we
run the Vite dev server to serve those assets. This arrangement enables Vite's hot reloading behavior for developing
the plugin while it's running inside a host.

### Release
```bash
npm run build
```

In release builds, the JavaScript bundles are packaged into the plugin app bundle so that the resulting bundle
is relocatable, thereby enabling distribution to end users.

### Troubleshooting

* After a successful build with either `npm run dev` or `npm run build`, you
  should have local plugin binaries built and copied into the correct
  audio plugin directories on your machine. If you don't see them, look in
  `./native/build/scripted/SRVB_artefacts` and copy them manually
* **Note**: the CMake build on Windows attempts to copy the VST3 plugin binary
  into `C:\Program Files`, a step that requires admin permissions. Therefore
  you should either run your build as an admin, or disable the copy plugin step
  in `native/CMakeLists.txt` and manually copy the plugin binary after build.
* **Note**: especially on MacOS, certain plugin hosts such as Ableton Live have
  strict security settings that prevent them from recognizing local unsigned
  binaries. You'll want to either add a codesign step to your build, or
  configure the security settings of your host to address this.

## License

[MIT](./LICENSE.md)

This project also uses [JUCE](https://juce.com/), which is licensed GPLv3. Please consult JUCE's license
agreement if you intend to distribute your own plugin based on this template.
