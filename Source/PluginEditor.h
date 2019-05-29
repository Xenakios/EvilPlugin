/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

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
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EvilPluginAudioProcessorEditor)
};
