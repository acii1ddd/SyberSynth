#pragma once

#include <JuceHeader.h>
#include "Data/OscData.h"
#include "Data/AdsrData.h"
#include "Data/FilterData.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
	// imp
	bool canPlaySound(juce::SynthesiserSound* sound) override;
	void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
	void stopNote(float velocity, bool allowTailOff) override;
	void controllerMoved(int controllerNumber, int newControllerValue) override;
	void pitchWheelMoved(int newPitchWheelValue) override;
	void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
	void renderNextBlock(juce::AudioBuffer< float >& outputBuffer, int startSample, int numSamples) override;
	void reset();

	std::array<OscData, 2>& getOscillator1() { return osc1; }
	AdsrData& getAdsr() { return adsr; }
	void updateModParams(const int filterType, const float filterCutoff, const float filterResonance);

private:
	static constexpr int numChannelsToProcess { 2 };
	std::array<OscData, numChannelsToProcess> osc1;
	std::array<FilterData, numChannelsToProcess> filter;
	//std::array<juce::dsp::Oscillator<float>, numChannelsToProcess> lfo;
	AdsrData adsr;
	juce::AudioBuffer<float> synthBuffer;

	juce::dsp::Gain<float> gain;
	bool isPrepared { false };
};