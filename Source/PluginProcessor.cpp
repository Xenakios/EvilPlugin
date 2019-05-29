/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EvilPluginAudioProcessor::EvilPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

EvilPluginAudioProcessor::~EvilPluginAudioProcessor()
{
}

//==============================================================================
const String EvilPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EvilPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EvilPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EvilPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EvilPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EvilPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EvilPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EvilPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String EvilPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void EvilPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void EvilPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void EvilPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EvilPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void EvilPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	ScopedLock locker(m_cs);
	ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	if (m_sleep_in_audio_thread)
	{
		Thread::sleep(1000);
		m_sleep_in_audio_thread = false;
	}
	if (m_cpu_waste_amount > 0.0)
	{
		std::uniform_real_distribution<double> dist(-1.0, 1.0);
		volatile double acc = 0.0;
		double bufferclocklen = 1000.0 / getSampleRate()*buffer.getNumSamples();
		double timetospend = bufferclocklen * m_cpu_waste_amount / 100.0;
		double t0 = Time::getMillisecondCounterHiRes();
		while (true)
		{
			acc += dist(m_rnd);
			double t1 = Time::getMillisecondCounterHiRes();
			if (t1 >= t0 + timetospend)
				break;
		}
	}
}

//==============================================================================
bool EvilPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* EvilPluginAudioProcessor::createEditor()
{
    return new EvilPluginAudioProcessorEditor (*this);
}

//==============================================================================
void EvilPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EvilPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EvilPluginAudioProcessor();
}
