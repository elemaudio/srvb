#include "PluginProcessor.h"
#include "WebViewEditor.h"


// A helper for reading numbers from a choc::Value, which seems to opportunistically parse
// JSON numbers into ints or 32-bit floats whenever it wants.
double numberFromChocValue(const choc::value::ValueView& v) {
    return (
        v.isFloat32() ? (double) v.getFloat32()
            : (v.isFloat64() ? v.getFloat64()
                : (v.isInt32() ? (double) v.getInt32()
                    : (double) v.getInt64())));
}

std::string getMimeType(std::string const& ext) {
    static std::unordered_map<std::string, std::string> mimeTypes {
        { ".html",   "text/html" },
        { ".js",     "application/javascript" },
        { ".css",    "text/css" },
    };

    if (mimeTypes.count(ext) > 0)
        return mimeTypes.at(ext);

    return "application/octet-stream";
}

//==============================================================================
WebViewEditor::WebViewEditor(juce::AudioProcessor* proc, juce::File const& assetDirectory, int width, int height)
    : juce::AudioProcessorEditor(proc)
{
    setSize(720, 444);

    choc::ui::WebView::Options opts;

#if JUCE_DEBUG
    opts.enableDebugMode = true;
#endif

#if ! ELEM_DEV_LOCALHOST
    opts.fetchResource = [=](const choc::ui::WebView::Options::Path& p) -> std::optional<choc::ui::WebView::Options::Resource> {
        auto relPath = "." + (p == "/" ? "/index.html" : p);
        auto f = assetDirectory.getChildFile(relPath);
        juce::MemoryBlock mb;

        if (!f.existsAsFile() || !f.loadFileAsData(mb))
            return {};

        return choc::ui::WebView::Options::Resource {
            std::vector<uint8_t>(mb.begin(), mb.end()),
            getMimeType(f.getFileExtension().toStdString())
        };
    };
#endif

    webView = std::make_unique<choc::ui::WebView>(opts);

#if JUCE_MAC
    viewContainer.setView(webView->getViewHandle());
#elif JUCE_WINDOWS
    viewContainer.setHWND(webView->getViewHandle());
#else
#error "We only support MacOS and Windows here yet."
#endif

    addAndMakeVisible(viewContainer);
    viewContainer.setBounds({0, 0, 720, 440});

    // Install message passing handlers
    webView->bind("__postNativeMessage__", [=](const choc::value::ValueView& args) -> choc::value::Value {
        if (args.isArray()) {
            auto eventName = args[0].getString();

            // When the webView loads it should send a message telling us that it has established
            // its message-passing hooks and is ready for a state dispatch
            if (eventName == "ready") {
                if (auto* ptr = dynamic_cast<EffectsPluginProcessor*>(getAudioProcessor())) {
                    ptr->dispatchStateChange();
                }
            }

#if ELEM_DEV_LOCALHOST
            if (eventName == "reload") {
                if (auto* ptr = dynamic_cast<EffectsPluginProcessor*>(getAudioProcessor())) {
                    ptr->initJavaScriptEngine();
                    ptr->dispatchStateChange();
                }
            }
#endif

            if (eventName == "setParameterValue" && args.size() > 1) {
                return handleSetParameterValueEvent(args[1]);
            }
        }

        return {};
    });

#if ELEM_DEV_LOCALHOST
    webView->navigate("http://localhost:5173");
#endif
}

choc::ui::WebView* WebViewEditor::getWebViewPtr()
{
    return webView.get();
}

void WebViewEditor::paint (juce::Graphics& g)
{
}

void WebViewEditor::resized()
{
    viewContainer.setBounds(getLocalBounds());
}

//==============================================================================
choc::value::Value WebViewEditor::handleSetParameterValueEvent(const choc::value::ValueView& e) {
    // When setting a parameter value, we simply tell the host. This will in turn fire
    // a parameterValueChanged event, which will catch and propagate through dispatching
    // a state change event
    if (e.isObject() && e.hasObjectMember("paramId") && e.hasObjectMember("value")) {
        auto const& paramId = e["paramId"].getString();
        double const v = numberFromChocValue(e["value"]);

        for (auto& p : getAudioProcessor()->getParameters()) {
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*>(p)) {
                if (pf->paramID.toStdString() == paramId) {
                    pf->setValueNotifyingHost(v);
                    break;
                }
            }
        }
    }

    return choc::value::Value();
}
