# effects-plugin

This project is a small, simple example to demonstrate using Elementary Audio to write an audio plugin. Here we tie in three different projects to facilitate a great developer experience.

* **Elementary** for handling your dynamic, functional audio processing
* **JUCE** for compiling to various favorite plugin formats on various platforms
* **CHOC** for a simple QuickJS JavaScript engine wrapper

If you're new to Elementary Audio, [**Elementary**](https://elementary.audio) is a JavaScript/C++ library for building audio applications.

* **Declarative:** Elementary makes it simple to create interactive audio processes through functional, declarative programming. Describe your audio process as a function of your application state, and Elementary will efficiently update the underlying audio engine as necessary.
* **Dynamic:** Most audio processing frameworks and tools facilitate building static processes. But what happens as your audio requirements change throughout the user journey? Elementary is designed to facilitate and adapt to the dynamic nature of modern audio applications.
* **Portable:** By decoupling the JavaScript API from the underlying audio engine (the "what" from the "how"), Elementary enables writing portable applications. Whether the underlying engine is running in the browser, an audio plugin, or an embedded device, the JavaScript layer remains the same.

Find more in the [Elementary repository on GitHub](https://github.com/elemaudio/elementary) and the documentation [on the website](https://elementary.audio/).

## Quick Start

```bash
# Clone the project with its submodules
git clone --recurse-submodules https://github.com/elemaudio/effects-plugin.git
cd effects-plugin

# Next we build the JavaScript assets
cd js/
npm install
npm run build
cd ../

# Finally we can build the plugin binaries themselves
mkdir -p build/MacOS/
cd build/MacOS/

cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 ../..
cmake --build .
```

At this point, thanks to JUCE's helpful CMake API, you should have local plugin binaries built and copied into the correct audio plugin directories on your machine. You should now be able to open your favorite plugin host and see your new plugins.

**Note**: especially on MacOS, certain plugin hosts such as Ableton Live have strict security settings that prevent them from recognizing your local binaries. To address this, you'll want to either add a codesign step to your build or address the security settings of your host.

## Overview

This example is primarily intended for those who have some level of familiarity with using JUCE to build audio plugins. If you're new to JUCE, consider reading up on some of the [basic JUCE tutorials](https://docs.juce.com/master/tutorial_code_basic_plugin.html) for audio plugins.

Next, assuming your familiarity with JUCE, we break down this example project as follows:
* We use JUCE's boilerplate AudioProcessor class for abstracting over the various audio plugin formats (VST3, AU, AAX, ...) and JUCE's CMake utilities for building
* Within the AudioProcessor, we first initialize the parameter set by reading from the provided `manifest.json` from the `js/` directory
* Next, in the `prepareToPlay` method, we construct an instance of CHOC's JavaScript engine and Elementary's audio engine runtime, with native interop methods injected into the JavaScript environment so that we can communicate the Elementary instruction set (rendered by the front-end) to the underlying audio engine runtime, where the actual audio processing takes place.
* Finally, in the `processBlock` method, we simply forward the relevant audio data into Elementary's audio engine `process` method to perform the graph rendering step.

With this architecture, we have all of the necessary boilerplate code in C++ to build a plugin, but
all of the details around _what_ this plugin does (in terms of audio processing) is provided by theJavaScript bundle.

So to hack around and explore this example plugin, you can simply edit `js/main.js` and `js/manifest.json`, rebuild the project, and test your plugins!

## License

[MIT](./LICENSE.md)
