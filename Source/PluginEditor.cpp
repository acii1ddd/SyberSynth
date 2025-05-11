/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SyberSynthAudioProcessorEditor::SyberSynthAudioProcessorEditor (SyberSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      audioProcessor (p),
      osc1(audioProcessor.apvts, "OSC1", "OSC1GAIN", "OSC1PITCH", "OSC1FMFREQ", "OSC1FMDEPTH"),
      filter(audioProcessor.apvts, "FILTERTYPE", "FILTERCUTOFF", "FILTERRESONANCE"),
      adsr(audioProcessor.apvts, "ATTACK", "DECAY", "SUSTAIN", "RELEASE"),
      reverb(audioProcessor.apvts, "REVERBSIZE", "REVERBDAMPING", "REVERBWIDTH", "REVERBDRY", "REVERBWET", "REVERBFREEZE")
{
    addAndMakeVisible(osc1);
    addAndMakeVisible(filter);
    addAndMakeVisible(adsr);
    addAndMakeVisible(reverb);

    osc1.setName("Oscillator 1");
    filter.setName("Filter");
    adsr.setName("ADSR");

    auto oscColour = juce::Colour::fromRGB(180, 140, 255);
    auto filterColour = juce::Colour::fromRGB(220, 160, 170);

    osc1.setBoundsColour(oscColour);
    filter.setBoundsColour(filterColour);

    startTimerHz(30);
    setSize(1080, 525);
}

SyberSynthAudioProcessorEditor::~SyberSynthAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void SyberSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
}

void SyberSynthAudioProcessorEditor::resized()
{
    const auto oscWidth = 420;
    const auto oscHeight = 180;
    osc1.setBounds(0, 0, oscWidth, oscHeight);
    filter.setBounds(osc1.getRight(), 0, 180, 200);
    adsr.setBounds(filter.getRight(), 0, 230, 360);
    reverb.setBounds(0, osc1.getBottom(), oscWidth, 150);
}

void SyberSynthAudioProcessorEditor::timerCallback()
{
    repaint();
}