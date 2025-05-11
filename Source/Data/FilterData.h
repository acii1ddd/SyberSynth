#pragma once

#include "OscData.h"
#include <JuceHeader.h>

class FilterData : public juce::dsp::StateVariableTPTFilter<float>
{
public:
	FilterData();
	void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
	void setParams(const int filterType, const float filterCutoff, const float filterResonance);
	void processNextBlock (juce::AudioBuffer<float>& buffer);
	float processNextSample (int channel, float inputValue);
    void resetAll();

private:
	void selectFilterType(const int filterType);
	juce::dsp::Oscillator<float> lfo { [](float x) { return std::sin(x); } };
};
