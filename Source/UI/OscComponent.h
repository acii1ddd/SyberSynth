#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"

class OscComponent : public CustomComponent
{
public:
    OscComponent(juce::AudioProcessorValueTreeState& apvts, juce::String oscId, juce::String gainId, juce::String pitchId, juce::String fmFreqId, juce::String fmDepthId);
    ~OscComponent() override;

    void resized() override;

private:
    juce::ComboBox oscSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oscSelAttachment;

    SliderWithLabel gain;
    SliderWithLabel pitch;
    SliderWithLabel fmFreq;
    SliderWithLabel fmDepth;

    static constexpr int dialWidth = 70;
    static constexpr int dialHeight = 70;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscComponent)
};