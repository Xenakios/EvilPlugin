/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void stackoverflowfunc(int x)
{
	++x;
	stackoverflowfunc(x);
}

//==============================================================================
EvilPluginAudioProcessorEditor::EvilPluginAudioProcessorEditor (EvilPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), m_mutex_thread(&p)
{
	StringArray buttexts{ "Access violation type 1", "Access violation type 2",
	"Stack overflow", "Divide by zero","Sleep in GUI thread", "Sleep in audio thread" };
	std::vector<std::function<void(void)>> callbacks
	{
		[]() { float* buf = nullptr; buf[0] = 0.55f; },
		[]() {},
		[]() { stackoverflowfunc(0); },
		[]() { volatile int x = 0; volatile int y = x / 0; },
		[]() { Thread::sleep(1000); },
		[this]() { processor.m_sleep_in_audio_thread=true; },
	};
	jassert(callbacks.size() == buttexts.size());
	for (int i = 0; i < buttexts.size(); ++i)
	{
		auto but = std::make_unique<TextButton>();
		but->setButtonText(buttexts[i]);
		but->onClick = callbacks[i];
		addAndMakeVisible(but.get());
		m_buttons.push_back(std::move(but));
	}
	auto tog = std::make_unique<ToggleButton>();
	addAndMakeVisible(tog.get());
	tog->setButtonText("Lock audio thread mutex");
	tog->onClick = [this,togcap=tog.get()]() 
	{
		if (m_mutex_thread.isThreadRunning() == false)
			m_mutex_thread.startThread();
		m_mutex_thread.m_lock_mutex = togcap->getToggleState();
	};
	m_buttons.push_back(std::move(tog));
	addAndMakeVisible(m_label_waste_gui_cpu);
	m_label_waste_gui_cpu.setText("Waste CPU in GUI thread",dontSendNotification);
	addAndMakeVisible(m_label_waste_audio_cpu);
	m_label_waste_audio_cpu.setText("Waste CPU in audio thread", dontSendNotification);
	addAndMakeVisible(m_label_waste_worker_cpu);
	m_label_waste_worker_cpu.setText("Waste CPU in worker thread", dontSendNotification);
	addAndMakeVisible(m_slider_waste_gui_cpu);
	m_slider_waste_gui_cpu.setRange(0.0, 100.0);
	m_slider_waste_gui_cpu.setValue(0.0, dontSendNotification);
	m_slider_waste_gui_cpu.onValueChange = [this]() 
	{
		if (m_slider_waste_gui_cpu.getValue() > 0.0)
			startTimer(100);
		else stopTimer();
	};
	
	addAndMakeVisible(m_slider_waste_audio_cpu);
	m_slider_waste_audio_cpu.setRange(0.0, 110.0);
	m_slider_waste_audio_cpu.setValue(0.0, dontSendNotification);
	m_slider_waste_audio_cpu.onValueChange = [this]() 
	{
		processor.m_cpu_waste_amount = m_slider_waste_audio_cpu.getValue();
	};
	
	addAndMakeVisible(m_slider_waste_worker_cpu);
	m_slider_waste_worker_cpu.setRange(0.0, 100.0);
	m_slider_waste_worker_cpu.setValue(0.0, dontSendNotification);
	m_slider_waste_worker_cpu.onValueChange = [this]() {};

	setSize (400, 310);
}

EvilPluginAudioProcessorEditor::~EvilPluginAudioProcessorEditor()
{
	m_mutex_thread.stopThread(1000);
}

//==============================================================================
void EvilPluginAudioProcessorEditor::paint (Graphics& g)
{
	g.fillAll(Colours::black);
}

void EvilPluginAudioProcessorEditor::resized()
{
	for (int i = 0; i < m_buttons.size(); ++i)
	{
		m_buttons[i]->setBounds(1, 1 + 30 * i, 100, 29);
		auto textbutton = dynamic_cast<TextButton*>(m_buttons[i].get());
		if (textbutton)
		{
			textbutton->changeWidthToFitText();
		}
		auto togglebutton = dynamic_cast<ToggleButton*>(m_buttons[i].get());
		if (togglebutton)
		{
			togglebutton->changeWidthToFitText();
		}
	}
	int yoffs = m_buttons.back()->getBottom() + 1;
	m_label_waste_gui_cpu.setBounds(1, yoffs, 200, 29);
	m_slider_waste_gui_cpu.setBounds(m_label_waste_gui_cpu.getRight()+1, yoffs, 200, 29);
	yoffs = m_slider_waste_gui_cpu.getBottom() + 1;

	m_label_waste_audio_cpu.setBounds(1, yoffs, 200, 29);
	m_slider_waste_audio_cpu.setBounds(m_label_waste_audio_cpu.getRight(), yoffs, 200, 29);
	
	yoffs = m_slider_waste_audio_cpu.getBottom() + 1;
	m_label_waste_worker_cpu.setBounds(1, yoffs, 200, 29);
	m_slider_waste_worker_cpu.setBounds(m_label_waste_audio_cpu.getRight(), yoffs, 200, 29);
	

}

void EvilPluginAudioProcessorEditor::timerCallback()
{
	double percent_to_waste = m_slider_waste_gui_cpu.getValue()/100.0;
	if (percent_to_waste > 0.0)
	{
		double timetowaste = (double)getTimerInterval()*percent_to_waste;
		CPU_waster(m_rnd, timetowaste);
	}
}
