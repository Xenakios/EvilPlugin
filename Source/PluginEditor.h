/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "containers/choc_SingleReaderSingleWriterFIFO.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include <map>

class CPUWasterThread : public Thread
{
  public:
    CPUWasterThread() : Thread("EvilPluginCPUWasterThread") {}
    double m_amount_to_waste = 0.0;
    void run() override
    {
        while (true)
        {
            if (threadShouldExit())
                break;
            if (m_amount_to_waste > 0.0)
            {
                double millistowaste = m_amount_to_waste * 10.0;
                CPU_waster(m_rnd, millistowaste);
                Thread::sleep(1000.0 - millistowaste);
            }
            else
                Thread::sleep(500);
        }
    }

  private:
    std::mt19937 m_rnd;
};

class MutexLockerThread : public Thread
{
  public:
    MutexLockerThread(EvilPluginAudioProcessor *proc)
        : Thread("EvilPluginMutexLockerThread"), m_proc(proc)
    {
        m_msg_fifo.reset(64);
    }
    ~MutexLockerThread()
    {
        if (m_mutex_is_locked)
            m_proc->m_cs.exit();
    }
    void run() override
    {
        while (true)
        {
            if (threadShouldExit() == true)
                break;
            Message msg;
            while (m_msg_fifo.pop(msg))
            {
                if (msg.opcode == Message::Opcode::LockMutex && !m_mutex_is_locked)
                {
                    m_mutex_is_locked = true;
                    m_proc->m_cs.enter();
                }
                if (msg.opcode == Message::Opcode::UnlockMutex && m_mutex_is_locked)
                {
                    m_mutex_is_locked = false;
                    m_proc->m_cs.exit();
                }
            }
            Thread::yield();
        }
    }

    struct Message
    {
        enum class Opcode
        {
            None,
            LockMutex,
            UnlockMutex
        };
        Opcode opcode = Opcode::None;
    };
    choc::fifo::SingleReaderSingleWriterFIFO<Message> m_msg_fifo;

  private:
    EvilPluginAudioProcessor *m_proc = nullptr;
    std::atomic<bool> m_mutex_is_locked = false;
};

class TimePositionComponent : public juce::Component
{
  public:
    TimePositionComponent() {}
    void paint(juce::Graphics &g) override
    {
        g.fillAll(juce::Colours::black);
        float xpos = juce::jmap<float>(m_currentpos, 0.0, m_maxpos, 0.0, getWidth());
        g.setColour(juce::Colours::white);
        g.drawLine(xpos, 0.0f, xpos, getHeight());
    }
    void setMaxTimePosition(double pos) { m_maxpos = pos; }
    void setCurrentPos(double pos)
    {
        m_currentpos = pos;
        repaint();
    }

  private:
    double m_maxpos = 1.0;
    double m_currentpos = 0.0;
};

class EvilPluginAudioProcessorEditor : public AudioProcessorEditor, public MultiTimer
{
  public:
    EvilPluginAudioProcessorEditor(EvilPluginAudioProcessor &);
    ~EvilPluginAudioProcessorEditor();

    //==============================================================================
    void paint(Graphics &) override;
    void resized() override;
    void timerCallback(int id) override;
    void heapTrash();

  private:
    EvilPluginAudioProcessor &processor;
    std::vector<std::unique_ptr<Button>> m_buttons;
    ToggleButton *m_mixinputButton = nullptr;
    ToggleButton *m_useGlobalsButton = nullptr;
    ToggleButton *m_lockAudioMutexButton = nullptr;
    Label m_label_waste_gui_cpu;
    Label m_label_waste_audio_cpu;
    Label m_label_waste_worker_cpu;
    TimePositionComponent m_timeposComponent;
    Slider m_slider_waste_gui_cpu;
    Slider m_slider_waste_audio_cpu;
    Slider m_slider_waste_worker_cpu;
    std::mt19937 m_rnd;
    MutexLockerThread m_mutex_thread;
    CPUWasterThread m_worker_cpu_waster;

    bool m_gui_is_sleeping = false;

    int m_num_noise_points = 0;
    double m_timepos_seconds = 0.0;
    void accessViolation1();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilPluginAudioProcessorEditor)
};
