#include "OscData.h"

void OscData::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels)
{
    resetAll();

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;

    // init main oscillator
    prepare(spec);

    // init fm oscillator
    fmOsc.prepare(spec);    

    gain.prepare(spec);
}

void OscData::setType(const int oscSelection)
{
    switch (oscSelection)
    {
        // sine
        case 0:
            initialise([](float x) { return std::sin(x); });
            break;

        // saw
        case 1:
            initialise([](float x) { return x / juce::MathConstants<float>::pi; });
            break;
        
        // square
        case 2:
            initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });
            break;

        default:
            jassertfalse;
            break;
    }
}

void OscData::setGain(const float levelInDecibels)
{
    gain.setGainDecibels(levelInDecibels);
}

void OscData::setOscPitch(const int pitch)
{
    lastPitch = pitch;
    setFrequency(juce::MidiMessage::getMidiNoteInHertz((lastMidiNote + lastPitch) + fmModulator));
}

void OscData::setFreq(const int midiNoteNumber)
{
    // setFrequency(getMidiNoteInHertz((60 + 2) + 0.5)) -> getMidiNoteInHertz(62.5)
    setFrequency(juce::MidiMessage::getMidiNoteInHertz((midiNoteNumber + lastPitch) + fmModulator));  // final midi note number
    lastMidiNote = midiNoteNumber;
}

void OscData::setFmOsc(const float freq, const float depth)  // 440 + 1000 = 1440   800
{
    fmDepth = depth;
    fmOsc.setFrequency(freq);

    setFrequency(juce::MidiMessage::getMidiNoteInHertz((lastMidiNote + lastPitch) + fmModulator));
}

void OscData::renderNextBlock(juce::dsp::AudioBlock<float>& audioBlock)
{
    // should not empty audioBlock
    jassert(audioBlock.getNumSamples() > 0);
    process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    gain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
}

float OscData::processNextSample(float input)
{
    // fmOsc.processSample(input) - sin value 0.5 (amplitude)
    fmModulator = fmOsc.processSample(input) * fmDepth;
    return gain.processSample(processSample(input));
}

void OscData::setParams(const int oscChoice, const float oscGain, const int oscPitch, const float fmFreq, const float fmDepth)
{
    setType(oscChoice);
    setGain(oscGain);
    setOscPitch(oscPitch);
    setFmOsc(fmFreq, fmDepth);
}

void OscData::resetAll()
{
    reset();
    fmOsc.reset();
    gain.reset();
}