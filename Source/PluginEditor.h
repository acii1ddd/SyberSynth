/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/OscComponent.h"
#include "UI/FilterComponent.h"
#include "UI/AdsrComponent.h"
#include "UI/ReverbComponent.h"



//==============================================================================
/**
*/
class SyberSynthAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    SyberSynthAudioProcessorEditor (SyberSynthAudioProcessor&);
    ~SyberSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SyberSynthAudioProcessor& audioProcessor;
    OscComponent osc1;
    FilterComponent filter;
    AdsrComponent adsr;
    ReverbComponent reverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SyberSynthAudioProcessorEditor)
};
