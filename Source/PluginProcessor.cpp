/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthVoice.h"

//==============================================================================
SyberSynthAudioProcessor::SyberSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParams())
#endif
{
    synth.addSound(new SynthSound());

    for (int i = 0; i < 5; i++)
    {
        synth.addVoice(new SynthVoice());
    }
}

SyberSynthAudioProcessor::~SyberSynthAudioProcessor()
{
}

//==============================================================================
const juce::String SyberSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SyberSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SyberSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SyberSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SyberSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SyberSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SyberSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SyberSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SyberSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SyberSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SyberSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);

    // prepare all voices of synthesiser
    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();

    reverbParams.roomSize = 0.5f;
    reverbParams.width = 1.0f;
    reverbParams.damping = 0.5f;
    reverbParams.freezeMode = 0.0f;
    reverbParams.dryLevel = 1.0f;
    reverbParams.wetLevel = 0.0f;

    reverb.setParameters(reverbParams);
}

void SyberSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SyberSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SyberSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // clear unused output channels that are not used as inputs
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    setParams();

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    juce::dsp::AudioBlock<float> block { buffer };
    reverb.process(juce::dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool SyberSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SyberSynthAudioProcessor::createEditor()
{
    return new SyberSynthAudioProcessorEditor (*this);
}

//==============================================================================
void SyberSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SyberSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SyberSynthAudioProcessor();
}

// implementation my methods
juce::AudioProcessorValueTreeState::ParameterLayout SyberSynthAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // OSC select
    params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC1", "Oscillator 1", juce::StringArray{ "Sine", "Saw", "Square" }, 0));

    // OSC Gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1GAIN", "Oscillator 1 Gain", juce::NormalisableRange<float> { -40.0f, 0.2f, 0.1f }, 0.2f, "dB"));

    // OSC Pitch value in semitones (+- 4 octaves)
    params.push_back(std::make_unique<juce::AudioParameterInt>("OSC1PITCH", "Oscillator 1 Pitch", -48, 48, 0));

    // FM Osc Freq
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1FMFREQ", "Oscillator 1 FM Frequency", juce::NormalisableRange<float> { 0.0f, 1000.0f, 0.1f }, 0.0f, "Hz"));

    // FM Osc Depth
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1FMDEPTH", "Oscillator 1 FM Depth", juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 0.0f, ""));

    //Filter
    params.push_back(std::make_unique<juce::AudioParameterChoice>("FILTERTYPE", "Filter Type", juce::StringArray{ "Low Pass" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERCUTOFF", "Filter Cutoff", juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.1f, 0.6f }, 20000.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERRESONANCE", "Filter Resonance", juce::NormalisableRange<float> { 0.1f, 2.0f, 0.1f }, 0.1f, ""));

    // ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release", juce::NormalisableRange<float> { 0.1f, 3.0f, 0.1f }, 0.0f));

    // Reverb
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERBSIZE", "Reverb Size", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERBWIDTH", "Reverb Width", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERBDAMPING", "Reverb Damping", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.5f, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERBDRY", "Reverb Dry", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 1.0f, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERBWET", "Reverb Wet", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERBFREEZE", "Reverb Freeze", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));

    return { params.begin(), params.end() };
}

void SyberSynthAudioProcessor::setParams()
{
    setVoiceParams();
    setFilterParams();
    setReverbParams();
}

void SyberSynthAudioProcessor::setVoiceParams()
{
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            auto& attack = *apvts.getRawParameterValue("ATTACK");
            auto& decay = *apvts.getRawParameterValue("DECAY");
            auto& sustain = *apvts.getRawParameterValue("SUSTAIN");
            auto& release = *apvts.getRawParameterValue("RELEASE");

            auto& osc1Choice = *apvts.getRawParameterValue("OSC1");
            auto& osc1Gain = *apvts.getRawParameterValue("OSC1GAIN");
            auto& osc1Pitch = *apvts.getRawParameterValue("OSC1PITCH");
            auto& osc1FmFreq = *apvts.getRawParameterValue("OSC1FMFREQ");
            auto& osc1FmDepth = *apvts.getRawParameterValue("OSC1FMDEPTH");

            auto& osc1 = voice->getOscillator1();
            auto& adsr = voice->getAdsr();

            // osc1 consists of 2 OscData (for left and right channels)
            for (int i = 0; i < getTotalNumOutputChannels(); i++)
            {
                osc1[i].setParams(osc1Choice, osc1Gain, osc1Pitch, osc1FmFreq, osc1FmDepth);
            }

            adsr.update(attack.load(), decay.load(), sustain.load(), release.load());
        }
    }
}

void SyberSynthAudioProcessor::setFilterParams()
{
    auto& filterType = *apvts.getRawParameterValue("FILTERTYPE");
    auto& filterCutoff = *apvts.getRawParameterValue("FILTERCUTOFF");
    auto& filterResonance = *apvts.getRawParameterValue("FILTERRESONANCE");

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->updateModParams(filterType, filterCutoff, filterResonance);
        }
    }
}

void SyberSynthAudioProcessor::setReverbParams()
{
    // get values of parameters from knobs
    reverbParams.roomSize = *apvts.getRawParameterValue("REVERBSIZE");
    reverbParams.width = *apvts.getRawParameterValue("REVERBWIDTH");
    reverbParams.damping = *apvts.getRawParameterValue("REVERBDAMPING");
    reverbParams.dryLevel = *apvts.getRawParameterValue("REVERBDRY");
    reverbParams.wetLevel = *apvts.getRawParameterValue("REVERBWET");
    reverbParams.freezeMode = *apvts.getRawParameterValue("REVERBFREEZE");

    reverb.setParameters(reverbParams);
}
