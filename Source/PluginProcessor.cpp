/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "juce_core/juce_core.h"

int instanceCounter = 0;

//==============================================================================
EvilPluginAudioProcessor::EvilPluginAudioProcessor()

    : AudioProcessor(BusesProperties()
                         .withInput("Input", AudioChannelSet::stereo(), true)
                         .withOutput("Output", AudioChannelSet::stereo(), true)

      )

{
    m_to_audio_fifo.reset(256);
    m_to_gui_fifo.reset(256);
    std::array<float, 4> oscFreqs = {440.0, 440 * 2, 440 * 1.5, 440.0 * 3};
    m_osc_frequency = oscFreqs[instanceCounter % oscFreqs.size()];
    ++instanceCounter;
}

EvilPluginAudioProcessor::~EvilPluginAudioProcessor() {}

//==============================================================================
const String EvilPluginAudioProcessor::getName() const { return JucePlugin_Name; }

bool EvilPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool EvilPluginAudioProcessor::producesMidi() const { return false; }

bool EvilPluginAudioProcessor::isMidiEffect() const { return false; }

double EvilPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int EvilPluginAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int EvilPluginAudioProcessor::getCurrentProgram() { return 0; }

void EvilPluginAudioProcessor::setCurrentProgram(int index) {}

const String EvilPluginAudioProcessor::getProgramName(int index) { return {}; }

void EvilPluginAudioProcessor::changeProgramName(int index, const String &newName) {}

double g_osc_phase = 0.0;

//==============================================================================
void EvilPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_osc_phase = 0.0;
    g_osc_phase = 0.0;
    m_time_pos = 0;
}

void EvilPluginAudioProcessor::releaseResources() {}

bool EvilPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void EvilPluginAudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages)
{
    ScopedLock locker(m_cs);
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    int badSample = 0;
    ThreadMessage msg;
    using OC = ThreadMessage::Opcode;
    while (m_to_audio_fifo.pop(msg))
    {
        if (msg.opcode == OC::Sleep)
        {
            Thread::sleep(1000);
        }
        if (msg.opcode == OC::WasteCPU)
        {
            m_cpu_waste_amount = msg.v0;
        }
        if (msg.opcode == OC::UseGlobalVariable)
        {
            if (msg.i0 == 0)
                m_use_global_variable = false;
            else
                m_use_global_variable = true;
        }
        if (msg.opcode == OC::LeakMemory)
        {
            leakMemory(msg.i0);
        }
        if (msg.opcode == OC::MixInputAudio)
        {
            if (msg.i0 == 1)
                m_mixInputAudio = true;
            else
                m_mixInputAudio = false;
        }
        if (msg.opcode == OC::BadSampleValue)
        {
            badSample = msg.i0;
        }
    }
    if (m_cpu_waste_amount > 0.0)
    {
        double bufferclocklen = 1000.0 / getSampleRate() * buffer.getNumSamples();
        double timetospend = bufferclocklen * m_cpu_waste_amount / 100.0;
        CPU_waster(m_rnd, timetospend);
    }
    double *oscphasevariable = &m_osc_phase;
    if (m_use_global_variable)
        oscphasevariable = &g_osc_phase;
    auto bufptrs = buffer.getArrayOfWritePointers();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float s = 0.02 * sin(2 * juce::MathConstants<double>::pi / getSampleRate() *
                             (*oscphasevariable) * m_osc_frequency);
        float dryLeft = bufptrs[0][i];
        float dryRight = bufptrs[1][i];
        for (int j = 0; j < buffer.getNumChannels(); ++j)
        {
            bufptrs[j][i] = s;
        }
        if (m_mixInputAudio)
        {
            bufptrs[0][i] += dryLeft;
            bufptrs[1][i] += dryRight;
        }
        (*oscphasevariable) += 1.0;
    }
    if (badSample == 1)
    {
        bufptrs[0][0] = std::numeric_limits<float>::quiet_NaN();
        bufptrs[1][buffer.getNumSamples() - 1] = std::numeric_limits<float>::quiet_NaN();
    }
    if (m_guiVisible)
    {
        m_to_gui_fifo.push({OC::TimePosition, 0, m_time_pos / getSampleRate()});
    }
    m_time_pos += buffer.getNumSamples();
}

//==============================================================================
bool EvilPluginAudioProcessor::hasEditor() const { return true; }

void EvilPluginAudioProcessor::pushStateToGUI()
{
    using OC = ThreadMessage::Opcode;
    m_to_gui_fifo.push({OC::MixInputAudio, m_mixInputAudio, 0.0});
    m_to_gui_fifo.push({OC::UseGlobalVariable, m_use_global_variable, 0.0});
}

AudioProcessorEditor *EvilPluginAudioProcessor::createEditor()
{
    pushStateToGUI();
    m_guiVisible = true;
    return new EvilPluginAudioProcessorEditor(*this);
}

//==============================================================================
void EvilPluginAudioProcessor::getStateInformation(MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EvilPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new EvilPluginAudioProcessor(); }
