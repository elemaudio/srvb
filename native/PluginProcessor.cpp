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
                         .getParentDirectory()                                                              // Plugin.vst3/Contents/<arch>/
                         .getParentDirectory()                                                              // Plugin.vst3/Contents/
                         .getChildFile("Resources/dist");
#else
#error "We only support Mac and Windows here yet."
#endif

    return assetsDir;
}

//==============================================================================
EffectsPluginProcessor::EffectsPluginProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      jsContext(choc::javascript::createQuickJSContext()),
      jsContext2(choc::javascript::createQuickJSContext())
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

    const auto manifest = elem::js::parseJSON(manifestFileContents);

    if (!manifest.isObject())
        return;

    const auto parameters = manifest.getWithDefault("parameters", elem::js::Array());
    createParameters(parameters);

    // The view state property has to have some value so that when state is loaded
    // from the host, the key exists and is populated.
   meshState.insert_or_assign(MESH_STATE_PROPERTY, "{}" );
}

EffectsPluginProcessor::~EffectsPluginProcessor()
{
    for (auto &p : getParameters())
    {
        p->removeListener(this);
    }
}

//==============================================================================
void EffectsPluginProcessor::createParameters(const std::vector<elem::js::Value> &parameters)
{
    for (const auto &parameter : parameters)
    {
        if (!parameter.isObject())
            continue;

        auto paramId = parameter.getWithDefault("paramId", elem::js::String("unknown"));
        auto name = parameter.getWithDefault("name", elem::js::String("Unknown"));
        auto minValue = parameter.getWithDefault("min", elem::js::Number(0));
        auto maxValue = parameter.getWithDefault("max", elem::js::Number(1));
        auto defaultValue = parameter.getWithDefault("defaultValue", elem::js::Number(0));
        auto step = parameter.getWithDefault("step", elem::js::Number(0));

        auto *p = new juce::AudioParameterFloat(
            juce::ParameterID(paramId, 1),
            name,
            {static_cast<float>(minValue), static_cast<float>(maxValue), static_cast<float>(step)},
            static_cast<float>(defaultValue));

        // Keep a map from parameter ID to the juce audio parameter
        // to avoid looping over the parameter list every time one changes
        parameterMap.insert({paramId, p});

        p->addListener(this);
        addParameter(p);

        // Push a new ParameterReadout onto the list to represent this parameter
        parameterReadouts.emplace_back(ParameterReadout{static_cast<float>(defaultValue), false});

        // Update our state object with the default parameter value
        state.insert_or_assign(paramId, defaultValue);
    }
}

juce::AudioProcessorEditor *EffectsPluginProcessor::createEditor()
{
    const auto editor = new WebViewEditor(this, getAssetsDirectory(), 905, 600);
    

    // -----------
    // semi-online license activation
    // -----------
    // From docs:
    // You may need to use semi-online activation without giving a serial number.
    // First two cases are:
    // 1- You may choose to renew weekly the offline license of the users with Semi Online activation.
    // You don't want to ask the serial number again and again.
    // 2- You may set Offline License Life (recommended) to 30 days. After 30 days, the offline license
    // will be INVALID. Then you will need to activate it again without a serial number.
    editor->handleUnlockEvent = [this](choc::value::Value &v)
    {
        const bool hasSerial = v.hasObjectMember("serial") && v["serial"].isString() && v["serial"].getString().length() > 0;
        bool shouldActivate = licenseStatus != Keyzy::LicenseStatus::VALID;

        if (!hasSerial && shouldActivate)
        {
            licenseStatus = licenseActivator.activateSemiOnline();
        }
        else if (hasSerial && shouldActivate && hasSerial)
        {
            const auto serial = v["serial"].getString();
            licenseStatus = licenseActivator.activateSemiOnline(serial.data());
        }

        sendJavascriptToUI("globalThis.__onUnlock__('" + unlock::errorStatuses(licenseStatus) + "')");
        // also send back the current host info
        juce::PluginHostType hostType;
        std::string hostDescription = static_cast<const char *>(hostType.getHostDescription());
        sendJavascriptToUI("globalThis.__hostInfo__('" + hostDescription + "')");
    };

    editor->ready = [this]()
    {
        // Flush the error log queue to the UI
        while (!errorLogQueue.empty())
        {
            auto sent = sendJavascriptToUI(errorLogQueue.front());
            errorLogQueue.pop();
        }
        dispatchStateChange();
        dispatchMeshStateChange();
    };

    
    // When setting a parameter value, we simply tell the host. This will in turn fire
    // a parameterValueChanged event, which will catch and propagate through dispatching
    // a state change event
    editor->setParameterValue = [this](const std::string &paramId, float value)
    {
        if (parameterMap.count(paramId) > 0)
        {
            parameterMap[paramId]->setValueNotifyingHost(value);
        }
    };

    editor->setMeshState = [this](choc::value::Value &v)
    {
        meshState.insert_or_assign(MESH_STATE_PROPERTY, v.toString());
        dispatchStateChange();
      //  dispatchMeshStateChange();   <--- was causing feedback loop! 
    };




#if ELEM_DEV_LOCALHOST
    editor->reload = [this]()
    {
        initJavaScriptEngine();
        initSecondJavaScriptEngine();
        dispatchStateChange();
        dispatchMeshStateChange();
    };

#endif

    return editor;
}

bool EffectsPluginProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
const juce::String EffectsPluginProcessor::getName() const
{
    return "NELShifterMesh";
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
    return 3.0;
}

//==============================================================================
int EffectsPluginProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int EffectsPluginProcessor::getCurrentProgram()
{
    return 0;
}

void EffectsPluginProcessor::setCurrentProgram(int /* index */) {}
const juce::String EffectsPluginProcessor::getProgramName(int /* index */) { return {}; }
void EffectsPluginProcessor::changeProgramName(int /* index */, const juce::String & /* newName */) {}

//==============================================================================
void EffectsPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Some hosts call `prepareToPlay` on the real-time thread, some call it on the main thread.
    // To address the discrepancy, we check whether anything has changed since our last known
    // call. If it has, we flag for initialization of the Elementary engine and runtime, then
    // trigger an async update.
    //
    // JUCE will synchronously handle the async update if it understands
    // that we're already on the main thread.
    if (sampleRate != lastKnownSampleRate || samplesPerBlock != lastKnownBlockSize)
    {
        lastKnownSampleRate = sampleRate;
        lastKnownBlockSize = samplesPerBlock;

        runtimeSwapRequired.store(true);
    }

    // Now that the environment is set up, push our current state
    triggerAsyncUpdate();
}

void EffectsPluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool EffectsPluginProcessor::isBusesLayoutSupported(const AudioProcessor::BusesLayout &layouts) const
{
    return true;
}

void EffectsPluginProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer & /* midiMessages */)
{

    // If the license is invalid, we clear the buffer and return
    if (licenseStatus != Keyzy::LicenseStatus::VALID)
    {
        buffer.clear();
        return;
    }

    // Copy the input so that our input and output buffers are distinct
    scratchBuffer.makeCopyOf(buffer, true);

    // Process the elementary runtime
    if (runtime != nullptr && !runtimeSwapRequired)

    {
        runtime->process(
            const_cast<const float **>(scratchBuffer.getArrayOfWritePointers()),
            getTotalNumInputChannels(),
            const_cast<float **>(buffer.getArrayOfWritePointers()),
            buffer.getNumChannels(),
            buffer.getNumSamples(),
            nullptr);
    }
    else
    {
        // Clear the output buffer to prevent any garbage if our runtime isn't ready
        buffer.clear();
    }

    if (runtimeSwapRequired)
    {
        shouldInitialize.store(true);
        triggerAsyncUpdate();
    }
}

void EffectsPluginProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    // Mark the updated parameter value in the dirty list
    auto &readout = *std::next(parameterReadouts.begin(), parameterIndex);

    readout.store({newValue, true});
    triggerAsyncUpdate();
}

void EffectsPluginProcessor::parameterGestureChanged(int, bool)
{
    // Not implemented
}

//==============================================================================
void EffectsPluginProcessor::handleAsyncUpdate()
{
    // First things first, we check the flag to identify if we should initialize the Elementary
    // runtime and engine.
    if (shouldInitialize.exchange(false))
    {
        runtime = std::make_unique<elem::Runtime<float>>(lastKnownSampleRate, lastKnownBlockSize);
        initJavaScriptEngine();
        initSecondJavaScriptEngine();
        runtimeSwapRequired.store(false);
    }

    // Next we iterate over the current parameter values to update our local state
    // object, which we in turn dispatch into the JavaScript engine
    auto &params = getParameters();

    // Reduce over the changed parameters to resolve our updated processor state
    for (size_t i = 0; i < parameterReadouts.size(); ++i)
    {
        // We atomically exchange an arbitrary value with a dirty flag false, because
        // we know that the next time we exchange, if the dirty flag is still false, the
        // value can be considered arbitrary. Only when we exchange and find the dirty flag
        // true do we consider the value as having been written by the processor since
        // we last looked.
        auto &current = *std::next(parameterReadouts.begin(), i);
        const auto pr = current.exchange({0.0f, false});

        if (pr.dirty)
        {
            if (const auto *pf = dynamic_cast<juce::AudioParameterFloat *>(params[i]))
            {
                auto paramId = pf->paramID.toStdString();
                state.insert_or_assign(paramId, static_cast<elem::js::Number>(pr.value));
            }
        }
    }

    dispatchStateChange();
}

void EffectsPluginProcessor::initJavaScriptEngine()
{
    jsContext = choc::javascript::createQuickJSContext();

    // Install some native interop functions in our JavaScript environment
    jsContext.registerFunction(NATIVE_MESSAGE_FUNCTION_NAME, [this](choc::javascript::ArgumentList args)
                               {
        const auto batch = elem::js::parseJSON(args[0]->toString());
        const auto rc = runtime->applyInstructions(batch);

        if (rc != elem::ReturnCode::Ok()) {
            dispatchError("Runtime Error", elem::ReturnCode::describe(rc));
        }

        return choc::value::Value(); });

    // A simple shim to write various console operations to our native __log__ handler
    jsContext.evaluateExpression(jsFunctions::consoleLogScript);

    const auto dspEntryFileContents = loadDspEntryFileContents();

    if (dspEntryFileContents.has_value())
    {
        jsContext.evaluateExpression(dspEntryFileContents.value());
    }
    else
    {
        return;
    }

    // Re-hydrate from current state
    const auto expr = serialize(jsFunctions::hydrateScript, runtime->snapshot());
    jsContext.evaluateExpression(expr);
}

void EffectsPluginProcessor::initSecondJavaScriptEngine()
{
    jsContext2 = choc::javascript::createQuickJSContext();

    // Install some native interop functions in our JavaScript environment
    jsContext2.registerFunction(NATIVE_MESSAGE_FUNCTION_NAME, [this](choc::javascript::ArgumentList args)
                               {
        const auto batch = elem::js::parseJSON(args[0]->toString());
        
       // const auto rc = runtime->applyInstructions(batch);
        // if (rc != elem::ReturnCode::Ok()) {
        //     dispatchError("Runtime Error", elem::ReturnCode::describe(rc));
        // }

        return choc::value::Value(); });

   
    const auto patchEntryFileContents = loadPatchEntryFileContents();

    if (patchEntryFileContents.has_value())
    {
        jsContext2.evaluateExpression(patchEntryFileContents.value());
    }
    else
    {
        return;
    }

   
}

void EffectsPluginProcessor::dispatchStateChange()
{
    // Need the double serialize here to correctly form the string script. The first
    // serialize produces the payload we want, the second serialize ensures we can replace
    // the % character in the above block and produce a valid javascript expression.
    auto localState = state;
    localState.insert_or_assign(SAMPLE_RATE_PROPERTY, lastKnownSampleRate);
    const auto expr = serialize(jsFunctions::dispatchScript, localState);
    // First we try to dispatch to the UI if it's available, because running this step will
    // just involve placing a message in a queue.
    sendJavascriptToUI(expr);
    // Next we dispatch to the local engine which will evaluate any necessary JavaScript synchronously
    // here on the main thread
    try
    {
        jsContext.evaluateExpression(expr);
    }
    catch (std::exception &e)
    {
        dispatchError("DSP JS:", e.what());
    }
}

void EffectsPluginProcessor::dispatchMeshStateChange()
{
    // Need the double serialize here to correctly form the string script. The first
    // serialize produces the payload we want, the second serialize ensures we can replace
    // the % character in the above block and produce a valid javascript expression.
    auto localState = meshState;
    const auto expr = serialize(jsFunctions::dispatchMeshStateScript, localState);
    // First we try to dispatch to the UI if it's available, because running this step will
    // just involve placing a message in a queue.
    sendJavascriptToUI(expr);
    // Next we dispatch to the local engine which will evaluate any necessary JavaScript synchronously
    // here on the main thread
    try
    {
        jsContext.evaluateExpression(expr);
    }
    catch (std::exception &e)
    {
        dispatchError("DSP JS:", e.what());
    }
}

void EffectsPluginProcessor::dispatchError(std::string const &name, std::string const &message)
{
    // Need the serialize here to correctly form the string script.
    const auto expr = juce::String(jsFunctions::errorScript).replace("@", elem::js::serialize(name)).replace("%", elem::js::serialize(message)).toStdString();

    // First we try to dispatch to the UI if it's available, because running this step will
    // just involve placing a message in a queue.
    if (!sendJavascriptToUI(expr))
    {
        if (errorLogQueue.size() == MAX_ERROR_LOG_QUEUE_SIZE)
        {
            errorLogQueue.pop();
        }
        errorLogQueue.push(expr);
    }

    // Next we dispatch to the local engine which will evaluate any necessary JavaScript synchronously
    // here on the main thread
    jsContext.evaluateExpression(expr);
}

std::optional<std::string> EffectsPluginProcessor::loadDspEntryFileContents() const
{
    // Load and evaluate our Elementary js main file
#if ELEM_DEV_LOCALHOST
    auto dspEntryFile = juce::URL("http://localhost:5173/dsp.main.js");
    auto dspEntryFileContents = dspEntryFile.readEntireTextStream().toStdString();
#else
    auto dspEntryFile = getAssetsDirectory().getChildFile(MAIN_DSP_JS_FILE);

    if (!dspEntryFile.existsAsFile())
        return std::nullopt;

    auto dspEntryFileContents = dspEntryFile.loadFileAsString().toStdString();
#endif

    return dspEntryFileContents;
}

std::optional<std::string> EffectsPluginProcessor::loadPatchEntryFileContents() const
{
    // Load and evaluate our Elementary js main file
#if ELEM_DEV_LOCALHOST
    auto patchEntryFile = juce::URL("http://localhost:5173/patch.main.js");
    auto patchEntryFileContents = patchEntryFile.readEntireTextStream().toStdString();
#else
    auto patchEntryFile = getAssetsDirectory().getChildFile(MAIN_PATCH_JS_FILE);

    if (!patchEntryFile.existsAsFile())
        return std::nullopt;

    auto patchEntryFileContents = patchEntryFile.loadFileAsString().toStdString();
#endif

    return patchEntryFileContents;
}


bool EffectsPluginProcessor::sendJavascriptToUI(const std::string &expr) const
{
    if (const auto *editor = dynamic_cast<WebViewEditor *>(getActiveEditor()))
    {
        editor->executeJavascript(expr);
        return true;
    }
    return false;
}

std::string EffectsPluginProcessor::serialize(const std::string &function, const elem::js::Object &data, const juce::String &replacementChar)
{
    return juce::String(function).replace(replacementChar, elem::js::serialize(elem::js::serialize(data))).toStdString();
}

std::string EffectsPluginProcessor::serialize(const std::string &function, const choc::value::Value &data, const juce::String &replacementChar)
{
    return juce::String(function).replace(replacementChar, choc::json::toString(data)).toStdString();
}

//==============================================================================
void EffectsPluginProcessor::getStateInformation(juce::MemoryBlock &destData)
{

    // serialise the secondary store for meshData
    // then insert it into the data to be stored by the host
    state.insert_or_assign(MESH_STATE_PROPERTY, meshState.at(MESH_STATE_PROPERTY));
    // seriliase the whole package
    const auto serializedState = elem::js::serialize(state);
    // stash
    destData.replaceAll((void *)serializedState.c_str(), serializedState.size());
    // remove the meshData from the active param state updates
    // so the meshData doesn't get sent on every update
    state.erase(MESH_STATE_PROPERTY);
}

void EffectsPluginProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    auto parsed = elem::js::parseJSON("{}");
    // restore the data the host stashed previously
    const auto jsonString = std::string(static_cast<const char *>(data), sizeInBytes);
    // try to deserialise the whole stashed data string
    try
    {
        parsed = elem::js::parseJSON(jsonString);
    }
    catch (...)
    {
        dispatchError("Stored data error:", parsed.toString());
    }

    // try to re-assign the local stores from the stashed data
    try
    {
        auto o = parsed.getObject();
        for (auto &[key, value] : o)
        {
            if (key != MESH_STATE_PROPERTY)
            {
                if (state.count(key) > 0)
                {
                    state.insert_or_assign(key, value);
                }
            }
            else
            {
                meshState.insert_or_assign(MESH_STATE_PROPERTY, value);
            }
        }
          dispatchStateChange();
    dispatchMeshStateChange();
    }

    catch (...)
    {
        // Failed to parse the incoming state, or the state we did parse was not actually
        // an object type. How you handle it is up to you.
        dispatchError("State Error", "Failed to restore mesh!");
    }

  
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new EffectsPluginProcessor();
}
