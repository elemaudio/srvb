#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <choc_WebView.h>


//==============================================================================
// A simple juce::AudioProcessorEditor that holds a choc::WebView and sets the
// WebView instance to cover the entire region of the editor.
class WebViewEditor : public juce::AudioProcessorEditor
{
public:
    //==============================================================================
    WebViewEditor(juce::AudioProcessor* proc, juce::File const& assetDirectory, int width, int height);

    //==============================================================================
    choc::ui::WebView* getWebViewPtr();

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    choc::value::Value handleSetParameterValueEvent(const choc::value::ValueView& e);

    //==============================================================================
    std::unique_ptr<choc::ui::WebView> webView;

#if JUCE_MAC
    juce::NSViewComponent viewContainer;
#elif JUCE_WINDOWS
    juce::HWNDComponent viewContainer;
#else
 #error "We only support MacOS and Windows here yet."
#endif
};
