#include "PluginProcessor.h"
#include "WebViewEditor.h"

#include <cstddef>
#include <cstring>
#include <unordered_map>

juce::String getMimeType(const juce::String& ext)
{
    static const std::unordered_map<juce::String, juce::String> mimeTypes {
        { ".html",   "text/html" },
        { ".js",     "application/javascript" },
        { ".css",    "text/css" },
    };

    if (mimeTypes.count(ext) > 0)
        return mimeTypes.at(ext);

    return "application/octet-stream";
}

std::vector<std::byte> toByteVector(const juce::MemoryBlock& block)
{
    std::vector<std::byte> result(block.getSize());
    std::memcpy(result.data(), block.getData(), block.getSize());
    return result;
}

//==============================================================================
WebViewEditor::WebViewEditor(juce::AudioProcessor* proc, juce::File const& assets, int width, int height)
    : juce::AudioProcessorEditor(proc),
      assetDirectory(assets)
{
    const auto nativeBridgeScript = juce::String(R"script(
(function() {
  globalThis.__postNativeMessage__ = function(message, payload) {
    window.__JUCE__.backend.emitEvent("postNativeMessage", [message, payload ?? null]);
  };
})();
)script");

    auto options = juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled()
        .withUserScript(nativeBridgeScript)
        .withEventListener("postNativeMessage", [this](const juce::var& args) {
            handleNativeMessage(args);
        })
        .withResourceProvider([this](const juce::String& path) {
            return getResource(path);
        }
#if ELEM_DEV_LOCALHOST
        , juce::URL("http://localhost:5173").getOrigin()
#endif
        );

#if JUCE_WINDOWS
    options = options
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
            .withUserDataFolder(juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)));
#endif

    webView = std::make_unique<juce::WebBrowserComponent>(options);
    addAndMakeVisible(*webView);

    setSize(width, height);

#if ELEM_DEV_LOCALHOST
    webView->goToURL("http://localhost:5173");
#else
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif
}

juce::WebBrowserComponent* WebViewEditor::getWebViewPtr()
{
    return webView.get();
}

void WebViewEditor::paint (juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void WebViewEditor::resized()
{
    webView->setBounds(getLocalBounds());
}

//==============================================================================
std::optional<juce::WebBrowserComponent::Resource> WebViewEditor::getResource(const juce::String& path) const
{
    auto relPath = path == "/" ? juce::String("index.html") : path.trimCharactersAtStart("/");
    auto f = assetDirectory.getChildFile(relPath);
    juce::MemoryBlock mb;

    if (!f.existsAsFile() || !f.loadFileAsData(mb))
        return {};

    return juce::WebBrowserComponent::Resource {
        toByteVector(mb),
        getMimeType(f.getFileExtension())
    };
}

void WebViewEditor::handleNativeMessage(const juce::var& args)
{
    const auto* array = args.getArray();

    if (array == nullptr || array->isEmpty())
        return;

    const auto eventName = array->getReference(0).toString();

    // When the webView loads it should send a message telling us that it has established
    // its message-passing hooks and is ready for a state dispatch.
    if (eventName == "ready")
    {
        if (auto* ptr = dynamic_cast<EffectsPluginProcessor*>(getAudioProcessor()))
            ptr->dispatchStateChange();
    }

#if ELEM_DEV_LOCALHOST
    if (eventName == "reload")
    {
        if (auto* ptr = dynamic_cast<EffectsPluginProcessor*>(getAudioProcessor()))
        {
            ptr->initJavaScriptEngine();
            ptr->dispatchStateChange();
        }
    }
#endif

    if (eventName == "setParameterValue" && array->size() > 1)
        handleSetParameterValueEvent(array->getReference(1));
}

void WebViewEditor::handleSetParameterValueEvent(const juce::var& e)
{
    auto* obj = e.getDynamicObject();

    if (obj == nullptr)
        return;

    const auto paramId = obj->getProperty("paramId").toString();
    const auto v = static_cast<double>(obj->getProperty("value"));

    // When setting a parameter value, we simply tell the host. This will in turn fire
    // a parameterValueChanged event, which will catch and propagate through dispatching
    // a state change event
    for (auto& p : getAudioProcessor()->getParameters())
    {
        if (auto* pf = dynamic_cast<juce::AudioParameterFloat*>(p))
        {
            if (pf->paramID == paramId)
            {
                pf->setValueNotifyingHost(static_cast<float>(v));
                break;
            }
        }
    }
}
