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

//==============================================================================
WebViewEditor::WebViewEditor(juce::AudioProcessor* proc, std::string const& indexFilePath, int width, int height)
    : juce::AudioProcessorEditor(proc)
{
    setSize(720, 444);

    choc::ui::WebView::Options opts;

#if JUCE_DEBUG
    opts.enableDebugMode = true;
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

            if (eventName == "setParameterValue") {
                jassert(args.size() > 1);
                return handleSetParameterValueEvent(args[1]);
            }
        }

        return {};
    });

    webView->navigate(indexFilePath);
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
    if (e.isObject() && e.hasObjectMember("name") && e.hasObjectMember("value")) {
        double const v = numberFromChocValue(e["value"]);

        for (auto& p : getAudioProcessor()->getParameters()) {
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*>(p)) {
                if (pf->paramID.toStdString() == e["name"].getString()) {
                    pf->setValueNotifyingHost(v);
                    break;
                }
            }
        }
    }

    return choc::value::Value();
}
