#include "PluginProcessor.h"
#include "WebViewEditor.h"

#include <choc_javascript_QuickJS.h>


//==============================================================================
// A quick helper for locating bundled asset files
juce::File getAssetsDirectory()
{
#if JUCE_MAC
    auto assetsDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile)
        .getChildFile("Contents/Resources/dist");
#elif JUCE_WINDOWS
    auto assetsDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile) // Plugin.vst3/Contents/<arch>/Plugin.vst3
        .getParentDirectory()  // Plugin.vst3/Contents/<arch>/
        .getParentDirectory()  // Plugin.vst3/Contents/
        .getChildFile("Resources/dist");
#else
#error "We only support Mac and Windows here yet."
#endif

    return assetsDir;
}

//==============================================================================
EffectsPluginProcessor::EffectsPluginProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
     , jsContext(choc::javascript::createQuickJSContext())
{
    // Initialize parameters from the manifest file
#if ELEM_DEV_LOCALHOST
    auto manifestFile = juce::URL("http://localhost:5173/manifest.json");
    auto manifestFileContents = manifestFile.readEntireTextStream().toStdString();
#else
    auto manifestFile = getAssetsDirectory().getChildFile("manifest.json");

    if (!manifestFile.existsAsFile())
        return;

    auto manifestFileContents = manifestFile.loadFileAsString().toStdString();
#endif

    auto manifest = elem::js::parseJSON(manifestFileContents);

    if (!manifest.isObject())
        return;

    auto parameters = manifest.getWithDefault("parameters", elem::js::Array());

    for (size_t i = 0; i < parameters.size(); ++i) {
        auto descrip = parameters[i];

        if (!descrip.isObject())
            continue;

        auto paramId = descrip.getWithDefault("paramId", elem::js::String("unknown"));
        auto name = descrip.getWithDefault("name", elem::js::String("Unknown"));
        auto minValue = descrip.getWithDefault("min", elem::js::Number(0));
        auto maxValue = descrip.getWithDefault("max", elem::js::Number(1));
        auto defValue = descrip.getWithDefault("defaultValue", elem::js::Number(0));

        auto* p = new juce::AudioParameterFloat(
            juce::ParameterID(paramId, 1),
            name,
            {static_cast<float>(minValue), static_cast<float>(maxValue)},
            defValue
        );

        p->addListener(this);
        addParameter(p);

        // Push a new ParameterReadout onto the list to represent this parameter
        paramReadouts.emplace_back(ParameterReadout { static_cast<float>(defValue), false });

        // Update our state object with the default parameter value
        state.insert_or_assign(paramId, defValue);
    }
}

EffectsPluginProcessor::~EffectsPluginProcessor()
{
    for (auto& p : getParameters())
    {
        p->removeListener(this);
    }
}

//==============================================================================
juce::AudioProcessorEditor* EffectsPluginProcessor::createEditor()
{
    return new WebViewEditor(this, getAssetsDirectory(), 800, 704);
}

bool EffectsPluginProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
const juce::String EffectsPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EffectsPluginProcessor::acceptsMidi() const
{
    return false;
}

bool EffectsPluginProcessor::producesMidi() const
{
    return false;
}

bool EffectsPluginProcessor::isMidiEffect() const
{
    return false;
}

double EffectsPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
int EffectsPluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EffectsPluginProcessor::getCurrentProgram()
{
    return 0;
}

void EffectsPluginProcessor::setCurrentProgram (int /* index */) {}
const juce::String EffectsPluginProcessor::getProgramName (int /* index */) { return {}; }
void EffectsPluginProcessor::changeProgramName (int /* index */, const juce::String& /* newName */) {}

//==============================================================================
void EffectsPluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Some hosts call `prepareToPlay` on the real-time thread, some call it on the main thread.
    // To address the discrepancy, we check whether anything has changed since our last known
    // call. If it has, we flag for initialization of the Elementary engine and runtime, then
    // trigger an async update.
    //
    // JUCE will synchronously handle the async update if it understands
    // that we're already on the main thread.
    if (sampleRate != lastKnownSampleRate || samplesPerBlock != lastKnownBlockSize) {
        lastKnownSampleRate = sampleRate;
        lastKnownBlockSize = samplesPerBlock;

        shouldInitialize.store(true);
    }

    // Now that the environment is set up, push our current state
    triggerAsyncUpdate();
}

void EffectsPluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool EffectsPluginProcessor::isBusesLayoutSupported (const AudioProcessor::BusesLayout& layouts) const
{
    return true;
}

void EffectsPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /* midiMessages */)
{
    // Copy the input so that our input and output buffers are distinct
    scratchBuffer.makeCopyOf(buffer, true);

    // Clear the output buffer to prevent any garbage if our runtime isn't ready
    buffer.clear();

    // Process the elementary runtime
    if (runtime != nullptr) {
        runtime->process(
            const_cast<const float**>(scratchBuffer.getArrayOfWritePointers()),
            getTotalNumInputChannels(),
            const_cast<float**>(buffer.getArrayOfWritePointers()),
            buffer.getNumChannels(),
            buffer.getNumSamples(),
            nullptr
        );
    }
}

void EffectsPluginProcessor::parameterValueChanged (int parameterIndex, float newValue)
{
    // Mark the updated parameter value in the dirty list
    auto& pr = *std::next(paramReadouts.begin(), parameterIndex);

    pr.store({ newValue, true });
    triggerAsyncUpdate();
}

void EffectsPluginProcessor::parameterGestureChanged (int, bool)
{
    // Not implemented
}

//==============================================================================
void EffectsPluginProcessor::handleAsyncUpdate()
{
    // First things first, we check the flag to identify if we should initialize the Elementary
    // runtime and engine.
    if (shouldInitialize.exchange(false)) {
        // TODO: This is definitely not thread-safe! It could delete a Runtime instance while
        // the real-time thread is using it. Depends on when the host will call prepareToPlay.
        runtime = std::make_unique<elem::Runtime<float>>(lastKnownSampleRate, lastKnownBlockSize);
        initJavaScriptEngine();
    }

    // Next we iterate over the current parameter values to update our local state
    // object, which we in turn dispatch into the JavaScript engine
    auto& params = getParameters();

    // Reduce over the changed parameters to resolve our updated processor state
    for (size_t i = 0; i < paramReadouts.size(); ++i)
    {
        // We atomically exchange an arbitrary value with a dirty flag false, because
        // we know that the next time we exchange, if the dirty flag is still false, the
        // value can be considered arbitrary. Only when we exchange and find the dirty flag
        // true do we consider the value as having been written by the processor since
        // we last looked.
        auto& current = *std::next(paramReadouts.begin(), i);
        auto pr = current.exchange({0.0f, false});

        if (pr.dirty)
        {
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*>(params[i])) {
                auto paramId = pf->paramID.toStdString();
                state.insert_or_assign(paramId, elem::js::Number(pr.value));
            }
        }
    }

    dispatchStateChange();
}

void EffectsPluginProcessor::initJavaScriptEngine()
{
    jsContext = choc::javascript::createQuickJSContext();

    // Install some native interop functions in our JavaScript environment
    jsContext.registerFunction("__postNativeMessage__", [this](choc::javascript::ArgumentList args) {
        auto const batch = elem::js::parseJSON(args[0]->toString());
        auto const rc = runtime->applyInstructions(batch);

        if (rc != elem::ReturnCode::Ok()) {
            dispatchError("Runtime Error", elem::ReturnCode::describe(rc));
        }

        return choc::value::Value();
    });

    jsContext.registerFunction("__log__", [this](choc::javascript::ArgumentList args) {
        const auto* kDispatchScript = R"script(
(function() {
  console.log(...JSON.parse(%));
  return true;
})();
)script";

        // Forward logs to the editor if it's available; then logs show up in one place.
        //
        // If not available, we fall back to std out.
        if (auto* editor = static_cast<WebViewEditor*>(getActiveEditor())) {
            auto v = choc::value::createEmptyArray();

            for (size_t i = 0; i < args.numArgs; ++i) {
                v.addArrayElement(*args[i]);
            }

            auto expr = juce::String(kDispatchScript).replace("%", elem::js::serialize(choc::json::toString(v))).toStdString();
            editor->getWebViewPtr()->evaluateJavascript(expr);
        } else {
            for (size_t i = 0; i < args.numArgs; ++i) {
                DBG(choc::json::toString(*args[i]));
            }
        }

        return choc::value::Value();
    });

    // A simple shim to write various console operations to our native __log__ handler
    jsContext.evaluate(R"shim(
(function() {
  if (typeof globalThis.console === 'undefined') {
    globalThis.console = {
      log(...args) {
        __log__('[embedded:log]', ...args);
      },
      warn(...args) {
          __log__('[embedded:warn]', ...args);
      },
      error(...args) {
          __log__('[embedded:error]', ...args);
      }
    };
  }
})();
    )shim");

    // Load and evaluate our Elementary js main file
#if ELEM_DEV_LOCALHOST
    auto dspEntryFile = juce::URL("http://localhost:5173/dsp.main.js");
    auto dspEntryFileContents = dspEntryFile.readEntireTextStream().toStdString();
#else
    auto dspEntryFile = getAssetsDirectory().getChildFile("dsp.main.js");

    if (!dspEntryFile.existsAsFile())
        return;

    auto dspEntryFileContents = dspEntryFile.loadFileAsString().toStdString();
#endif
    jsContext.evaluate(dspEntryFileContents);

    // Re-hydrate from current state
    const auto* kHydrateScript = R"script(
(function() {
  if (typeof globalThis.__receiveHydrationData__ !== 'function')
    return false;

  globalThis.__receiveHydrationData__(%);
  return true;
})();
)script";

    auto expr = juce::String(kHydrateScript).replace("%", elem::js::serialize(elem::js::serialize(runtime->snapshot()))).toStdString();
    jsContext.evaluate(expr);
}

void EffectsPluginProcessor::dispatchStateChange()
{
    const auto* kDispatchScript = R"script(
(function() {
  if (typeof globalThis.__receiveStateChange__ !== 'function')
    return false;

  globalThis.__receiveStateChange__(%);
  return true;
})();
)script";

    // Need the double serialize here to correctly form the string script. The first
    // serialize produces the payload we want, the second serialize ensures we can replace
    // the % character in the above block and produce a valid javascript expression.
    auto localState = state;
    localState.insert_or_assign("sampleRate", lastKnownSampleRate);

    auto expr = juce::String(kDispatchScript).replace("%", elem::js::serialize(elem::js::serialize(localState))).toStdString();

    // First we try to dispatch to the UI if it's available, because running this step will
    // just involve placing a message in a queue.
    if (auto* editor = static_cast<WebViewEditor*>(getActiveEditor())) {
        editor->getWebViewPtr()->evaluateJavascript(expr);
    }

    // Next we dispatch to the local engine which will evaluate any necessary JavaScript synchronously
    // here on the main thread
    jsContext.evaluate(expr);
}

void EffectsPluginProcessor::dispatchError(std::string const& name, std::string const& message)
{
    const auto* kDispatchScript = R"script(
(function() {
  if (typeof globalThis.__receiveError__ !== 'function')
    return false;

  let e = new Error(%);
  e.name = @;

  globalThis.__receiveError__(e);
  return true;
})();
)script";

    // Need the serialize here to correctly form the string script.
    auto expr = juce::String(kDispatchScript).replace("@", elem::js::serialize(name)).replace("%", elem::js::serialize(message)).toStdString();

    // First we try to dispatch to the UI if it's available, because running this step will
    // just involve placing a message in a queue.
    if (auto* editor = static_cast<WebViewEditor*>(getActiveEditor())) {
        editor->getWebViewPtr()->evaluateJavascript(expr);
    }

    // Next we dispatch to the local engine which will evaluate any necessary JavaScript synchronously
    // here on the main thread
    jsContext.evaluate(expr);
}

//==============================================================================
void EffectsPluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto serialized = elem::js::serialize(state);
    destData.replaceAll((void *) serialized.c_str(), serialized.size());
}

void EffectsPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    try {
        auto str = std::string(static_cast<const char*>(data), sizeInBytes);
        auto parsed = elem::js::parseJSON(str);
        auto o = parsed.getObject();
        for (auto  &i: o) {
            std::map<std::string, elem::js::Value>::iterator it;
            it = state.find(i.first);
            if (it != state.end()) {
                state.insert_or_assign(i.first, i.second);
            }
        }
    } catch(...) {
        // Failed to parse the incoming state, or the state we did parse was not actually
        // an object type. How you handle it is up to you, here we just ignore it
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EffectsPluginProcessor();
}
