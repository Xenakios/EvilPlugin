/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

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
	MutexLockerThread(EvilPluginAudioProcessor* proc) : Thread("EvilPluginMutexLockerThread"), m_proc(proc)
	{

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
			if (m_lock_mutex == true && m_mutex_is_locked == false)
			{
				m_mutex_is_locked = true;
				m_proc->m_cs.enter();
			}
			if (m_mutex_is_locked == true && m_lock_mutex == false)
			{
				m_mutex_is_locked = false;
				m_proc->m_cs.exit();
			}
			Thread::yield();
		}
	}
	bool m_lock_mutex = false;
	bool m_mutex_is_locked = false;
private:
	EvilPluginAudioProcessor* m_proc = nullptr;
};

//==============================================================================
/**
*/
class EvilPluginAudioProcessorEditor  : public AudioProcessorEditor, public Timer
{
public:
    EvilPluginAudioProcessorEditor (EvilPluginAudioProcessor&);
    ~EvilPluginAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
	void timerCallback() override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EvilPluginAudioProcessor& processor;
	std::vector<std::unique_ptr<Button>> m_buttons;
	Label m_label_waste_gui_cpu;
	Label m_label_waste_audio_cpu;
	Label m_label_waste_worker_cpu;
	Slider m_slider_waste_gui_cpu;
	Slider m_slider_waste_audio_cpu;
	Slider m_slider_waste_worker_cpu;
	std::mt19937 m_rnd;
	MutexLockerThread m_mutex_thread;
	CPUWasterThread m_worker_cpu_waster;
	Image m_devil;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EvilPluginAudioProcessorEditor)
};
