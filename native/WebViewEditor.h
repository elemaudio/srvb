#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>


//==============================================================================
// A simple juce::AudioProcessorEditor that holds a JUCE WebBrowserComponent and sets the
// WebView instance to cover the entire region of the editor.
class WebViewEditor : public juce::AudioProcessorEditor
{
public:
    //==============================================================================
    WebViewEditor(juce::AudioProcessor* proc, juce::File const& assetDirectory, int width, int height);

    //==============================================================================
    juce::WebBrowserComponent* getWebViewPtr();

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& path) const;
    void handleNativeMessage(const juce::var& args);
    void handleSetParameterValueEvent(const juce::var& e);

    //==============================================================================
    juce::File assetDirectory;
    std::unique_ptr<juce::WebBrowserComponent> webView;
};
