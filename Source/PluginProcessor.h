#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <random>
#include <atomic>
#include "xen_utilities.h"
#include "containers/choc_SingleReaderSingleWriterFIFO.h"

inline void leakMemory(size_t bytes)
{
    unsigned char *ptr = new unsigned char[bytes];
    // might even want to fill with random bytes...
    memset(ptr, 63, bytes);
}

struct ThreadMessage
{
    enum class Opcode
    {
        None,
        Sleep,
        WasteCPU,
        UseGlobalVariable,
        LeakMemory,
        MixInputAudio
    };
    Opcode opcode = Opcode::None;
    int i0 = 0;
    double v0 = 0.0;
};

class EvilPluginAudioProcessor : public AudioProcessor
{
  public:
    EvilPluginAudioProcessor();
    ~EvilPluginAudioProcessor();
    choc::fifo::SingleReaderSingleWriterFIFO<ThreadMessage> m_to_audio_fifo;
    choc::fifo::SingleReaderSingleWriterFIFO<ThreadMessage> m_to_gui_fifo;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(AudioBuffer<float> &, MidiBuffer &) override;

    AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String &newName) override;

    void getStateInformation(MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    CriticalSection m_cs;
    void pushStateToGUI();

  private:
    bool m_use_global_variable = false;
    double m_cpu_waste_amount = 0.0;
    bool m_mixInputAudio = false;
    std::mt19937 m_rnd;
    double m_osc_phase = 0.0;
    double m_osc_frequency = 440.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilPluginAudioProcessor)
};
