#include "SynthVoice.h"

// this SynthVoice can play given sound or not
bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    for (int i = 0; i < 2; i++)
    {
        osc1[i].setFreq(midiNoteNumber);
    }

    adsr.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    adsr.noteOff();

    if (!allowTailOff || !adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{

}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{

}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels)
{
    reset();

    adsr.setSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;

    for (int channel = 0; channel < numChannelsToProcess; channel++)
    {
        osc1[channel].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
        filter[channel].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
        lfo[channel].prepare(spec);
        lfo[channel].initialise([](float x) { return std::sin(x); });
    }

    gain.prepare(spec);
    
    // decrease volume on 7% from the original
    gain.setGainLinear(0.07f);

    isPrepared = true;
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // should been prepared
    jassert(isPrepared);

    if (!isVoiceActive())
        return;

    // sound
    synthBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
    synthBuffer.clear();

    for (int channel = 0; channel < synthBuffer.getNumChannels(); ++channel)
    {
        auto* buffer = synthBuffer.getWritePointer(channel, 0);

        for (int s = 0; s < synthBuffer.getNumSamples(); ++s)
        {
            buffer[s] = osc1[channel].processNextSample(buffer[s]) /*+ osc2[ch].processNextSample(buffer[s]) */;
        }
    }

    // filter
    juce::dsp::AudioBlock<float> audioBlock { synthBuffer };
    gain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    adsr.applyEnvelopeToBuffer(synthBuffer, 0, synthBuffer.getNumSamples());

    for (int channel = 0; channel < synthBuffer.getNumChannels(); ++channel)
    {
        auto* buffer = synthBuffer.getWritePointer(channel, 0);

        for (int s = 0; s < synthBuffer.getNumSamples(); ++s)
        {
            //lfoOutput[ch] = lfo[ch].processSample (synthBuffer.getSample (ch, s));
            buffer[s] = filter[channel].processNextSample(channel, synthBuffer.getSample(channel, s));
        }
    }

    // copy from synthBuffer to outputBuffer for 2 channels (left and right)
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom(channel, startSample, synthBuffer, channel, 0, numSamples);

        if (!adsr.isActive())
            clearCurrentNote();
    }
}

void SynthVoice::reset()
{
    gain.reset();
    adsr.reset();
}

void SynthVoice::updateModParams(const int filterType, const float filterCutoff, const float filterResonance)
{
    auto cutoff = filterCutoff;
    cutoff = std::clamp<float>(cutoff, 20.0f, 20000.0f);

    for (int channel = 0; channel < numChannelsToProcess; ++channel)
    {
        filter[channel].setParams(filterType, cutoff, filterResonance);
    }
}
