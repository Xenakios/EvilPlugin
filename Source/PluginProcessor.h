#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <random>
#include <atomic>
#include "xen_utilities.h"

//==============================================================================
/**
*/
class EvilPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    EvilPluginAudioProcessor();
    ~EvilPluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
	bool m_sleep_in_audio_thread = false;
	CriticalSection m_cs;
	double m_cpu_waste_amount = 0.0;
	bool m_use_global_variable = false;
private:
	std::mt19937 m_rnd;
	double m_osc_phase = 0.0;
	double m_osc_frequency = 440.0;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EvilPluginAudioProcessor)
};
