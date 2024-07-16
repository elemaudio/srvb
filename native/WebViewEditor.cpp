#include "PluginProcessor.h"
#include "WebViewEditor.h"

// A helper for reading numbers from a choc::Value, which seems to opportunistically parse
// JSON numbers into ints or 32-bit floats whenever it wants.
double numberFromChocValue(const choc::value::ValueView &v)
{
    return (
        v.isFloat32() ? (double)v.getFloat32()
                      : (v.isFloat64() ? v.getFloat64()
                                       : (v.isInt32() ? (double)v.getInt32()
                                                      : (double)v.getInt64())));
}

std::string getMimeType(std::string const &ext)
{
    static std::unordered_map<std::string, std::string> mimeTypes{
        {".html", "text/html"},
        {".js", "application/javascript"},
        {".css", "text/css"},
    };

    if (mimeTypes.count(ext) > 0)
        return mimeTypes.at(ext);

    return "application/octet-stream";
}

//==============================================================================
WebViewEditor::WebViewEditor(juce::AudioProcessor *proc, juce::File const &assetDirectory, int width, int height)
    : juce::AudioProcessorEditor(proc)
{
    setSize(width, height);
    setResizable(true, false);

    choc::ui::WebView::Options opts;

#if JUCE_DEBUG
    opts.enableDebugMode = false;
#endif

#if !ELEM_DEV_LOCALHOST
    opts.enableDebugMode = false;
    opts.fetchResource = [=](const choc::ui::WebView::Options::Path &p) -> std::optional<choc::ui::WebView::Options::Resource>
    {
        auto relPath = "." + (p == "/" ? "/index.html" : p);
        auto f = assetDirectory.getChildFile(relPath);
        juce::MemoryBlock mb;

        if (!f.existsAsFile() || !f.loadFileAsData(mb))
            return {};

        return choc::ui::WebView::Options::Resource{
            std::string(mb.begin(), mb.end()),
            getMimeType(f.getFileExtension().toStdString())};





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
        viewContainer.setBounds({0, 0, 905, 600});

        // Install message passing handlers
        webView->bind(POST_NATIVE_MESSAGE, [=](const choc::value::ValueView &args) -> choc::value::Value
                      {
        if (args.isArray()) {
            const auto eventName = args[0].getString();

            // When the webView loads it should send a message telling us that it has established
            // its message-passing hooks and is ready for a state dispatch
            if (eventName == READY_EVENT) {
                ready();
            }

            if (eventName == RELOAD_EVENT) {
                reload();
            }

            if (eventName == SET_PARAMETER_VALUE && args.size() > 1) {
                return handleSetParameterValueEvent(args[1]);
            }

            if (eventName == SET_MESH_STATE && args.size() > 1) {
                // Take a copy of the value and pass it to the view state change handler.
                auto value = choc::value::Value(args[1]);
                setMeshState(value);
            }

            // For Keyzy send an unlock event with an object containing the serial number
            if (eventName == UNLOCK_EVENT && args.size() > 1) {
                auto value = choc::value::Value(args[1]);
                handleUnlockEvent(value);
            }
        }

        return {}; 
        });

#if ELEM_DEV_LOCALHOST
        webView->navigate("http://localhost:5173");
#endif
    }

    void  WebViewEditor::paint(juce::Graphics &g)
    {
    }

    void WebViewEditor::resized()
    {
        viewContainer.setBounds(getLocalBounds());
    }

    void WebViewEditor::executeJavascript(const std::string &script) const
    {
        webView->evaluateJavascript(script);
    }

    //==============================================================================
    choc::value::Value WebViewEditor::handleSetParameterValueEvent(const choc::value::ValueView &e) const
    {
        if (e.isObject() && e.hasObjectMember("paramId") && e.hasObjectMember("value"))
        {
            auto const &paramId = e["paramId"].getString();
            double const v = numberFromChocValue(e["value"]);

            setParameterValue(std::string{paramId}, static_cast<float>(v));
        }

        return {};
    }
