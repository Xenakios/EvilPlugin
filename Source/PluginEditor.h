/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

class Animator : public Timer
{
public:
	enum class State
	{
		Started,
		Running,
		Finished
	};
	Animator(int updateinterval = 40) : m_update_interval(updateinterval)
	{
		CurveFunc = identity<double>;
	}
	using animfunc = std::function<void(State, double)>;
	// CurveFunc is given a value between 0.0-1.0 and must return a value between 0.0-1.0
	std::function<double(double)> CurveFunc;
	void timerCallback() override
	{
		double elapsed = Time::getMillisecondCounterHiRes()-m_starttime;
		double normpos = 1.0 / m_dur * elapsed;
		if (m_cb)
		{
			if (normpos >= 1.0)
			{
				m_cb(State::Finished, CurveFunc(1.0));
				stopTimer();
			}
			else 
				m_cb(State::Running, CurveFunc(normpos));
		}
	}
	void start(double duration,animfunc f)
	{
		m_cb = f;
		m_dur = jlimit(0.1, 600.0, duration)*1000.0;
		m_starttime = Time::getMillisecondCounterHiRes();
		if (m_cb)
			m_cb(State::Started, CurveFunc(0.0));
		startTimer(m_update_interval);
	}
	void stop()
	{
		stopTimer();
		if (m_cb)
			m_cb(State::Finished, CurveFunc(1.0));
	}
private:
	animfunc m_cb;
	double m_starttime = 0.0;
	double m_dur = 0.1;
	int m_update_interval = 40;
};

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
	void heapTrash();
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
    Image m_kitty;
    bool m_gui_is_sleeping = false;
	Animator m_anim{ 25 };
	double m_devil_transparency = 1.0;
	double m_devil_x_offset = 0.0;
	double m_kitty_transparency = 0.0;
	int m_num_noise_points = 0;
	void accessViolation1();
	OpenGLContext m_ogl;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EvilPluginAudioProcessorEditor)
};
