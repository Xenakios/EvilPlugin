/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

std::function<void(int)> g_stackoverflowcb;

void stackoverflowfunc1(int x)
{
	++x;
	g_stackoverflowcb(x);
}

void writeEnvTestFile()
{
	WavAudioFormat format;
	File outfile("C:\\MusicAudio\\sourcesamples\\test_signals\\bkenv\\out1.wav");
	outfile.deleteFile();
	double outsr = 44100.0;
	FileOutputStream* stream = outfile.createOutputStream();
	AudioFormatWriter* writer = format.createWriterFor(stream, outsr, 1, 32, {}, 0);
	if (writer)
	{
		int procbufsize = 33;
		AudioBuffer<float> buf(1, procbufsize);
		Envelope env{ {0.0,0.0}, {0.75,1.0},{1.0,0.0},{1.5,0.0},{1.6,0.5},{1.7,0.0} };
		env.scaleTimes(100.0);
		std::vector<double> envbuf(procbufsize);
		int pos = 0;
		while (pos < outsr*200.0)
		{
			double t0 = pos / outsr;
			double t1 = (pos + procbufsize) / outsr;
			env.applyToBuffer(envbuf.data(), procbufsize, t0 - 5.0, t1 - 5.0, { 0.1,0.9 });
			for (int i = 0; i < buf.getNumSamples(); ++i)
			{
				float s = sin(2 * 3.141592 / outsr * (pos+i) *440.0);
				buf.setSample(0, i, s*envbuf[i]);
			}
			writer->writeFromAudioSampleBuffer(buf, 0, procbufsize);
			pos += procbufsize;
		}
		
		delete writer;
	}
	else
		delete stream;
}

//==============================================================================
EvilPluginAudioProcessorEditor::EvilPluginAudioProcessorEditor (EvilPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), m_mutex_thread(&p)
{
	writeEnvTestFile();
	m_devil_transparency = 0.0;
	Envelope env{ {0.0,0.0},{0.5,1.0},{1.0,0.0} };
	m_anim.CurveFunc = [env](double x) 
	{ 
		return env.getValueAtTime(x);
		//return 0.5 + 0.5*cos(2 * 3.141592*x*4.0); 
	};
	StringArray buttexts{ "Access violation type 1", "Access violation type 2",
	"Stack overflow", "Divide by zero","Sleep in GUI thread", "Sleep in audio thread" };
	std::vector<std::function<void(void)>> callbacks
	{
		[]() { float* buf = nullptr; buf[0] = 0.55f; },
		[this]() { heapTrash();  },
		[]() { g_stackoverflowcb = stackoverflowfunc1; volatile int x = 0; stackoverflowfunc1(x); },
		[this]() 
		{ 
			//m_anim.CurveFunc = identity<double>;
			m_anim.start(2.0,[this](Animator::State s,double x) 
			{
				m_devil_transparency = x;
				//m_kitty_transparency = 1.0 - x;
				repaint();
				if (s == Animator::State::Finished)
				{
					//volatile int x = 0; volatile int y = x / 0;
				}
				
			});
			
		},
		[this]()
        {
            m_gui_is_sleeping = true;
            repaint();
            Timer::callAfterDelay(100,[this]()
                                      {
                                          Thread::sleep(5000);
                                          m_gui_is_sleeping = false;
                                          repaint();
                                      });
            
        },
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
	
	tog = std::make_unique<ToggleButton>();
	addAndMakeVisible(tog.get());
	tog->setButtonText("Use global variables (needs multiple plugin instances to break)");
	tog->onClick = [this, togcap = tog.get()]()
	{
		processor.m_use_global_variable = togcap->getToggleState();
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
	m_slider_waste_gui_cpu.setNumDecimalPlacesToDisplay(2);
	m_slider_waste_gui_cpu.setTextValueSuffix(" %");
	m_slider_waste_gui_cpu.onValueChange = [this]() 
	{
		if (m_slider_waste_gui_cpu.getValue() > 0.0)
			startTimer(100);
		else stopTimer();
	};
	
	addAndMakeVisible(m_slider_waste_audio_cpu);
	m_slider_waste_audio_cpu.setRange(0.0, 110.0);
	m_slider_waste_audio_cpu.setValue(0.0, dontSendNotification);
	m_slider_waste_audio_cpu.setNumDecimalPlacesToDisplay(2);
	m_slider_waste_audio_cpu.setTextValueSuffix(" %");
	m_slider_waste_audio_cpu.onValueChange = [this]() 
	{
		processor.m_cpu_waste_amount = m_slider_waste_audio_cpu.getValue();
	};
	
	addAndMakeVisible(m_slider_waste_worker_cpu);
	m_slider_waste_worker_cpu.setRange(0.0, 100.0);
	m_slider_waste_worker_cpu.setValue(0.0, dontSendNotification);
	m_slider_waste_worker_cpu.setNumDecimalPlacesToDisplay(2);
	m_slider_waste_worker_cpu.setTextValueSuffix(" %");
	m_slider_waste_worker_cpu.onValueChange = [this]() 
	{
		m_worker_cpu_waster.m_amount_to_waste = m_slider_waste_worker_cpu.getValue();
	};
	m_worker_cpu_waster.startThread();
#ifdef WIN32
    m_devil = ImageFileFormat::loadFrom(File("C:\\NetDownloads\\03042019\\devil1.png"));
	m_kitty = ImageFileFormat::loadFrom(File("C:\\NetDownloads\\03042019\\kitty1.jpg"));
	jassert(m_kitty.isValid());
#else
    m_devil = ImageFileFormat::loadFrom(File("/Users/teemu/Downloads/19022019/devil.png"));
    m_kitty = ImageFileFormat::loadFrom(File("/Users/teemu/Downloads/19022019/kitty1.jpg"));
    
#endif
	setSize (500, 340);
}

EvilPluginAudioProcessorEditor::~EvilPluginAudioProcessorEditor()
{
	m_mutex_thread.stopThread(1000);
	m_worker_cpu_waster.stopThread(1000);
}

//==============================================================================
void EvilPluginAudioProcessorEditor::paint (Graphics& g)
{
	g.fillAll(Colours::black);
	if (m_devil_transparency > 0.0)
	{
		g.setOpacity(m_devil_transparency);
		g.drawImageWithin(m_devil, 0, 0, getWidth(), getHeight(), RectanglePlacement::xRight);
	}
	if (m_kitty_transparency > 0.0)
	{
		g.setOpacity(m_kitty_transparency);
		g.addTransform(AffineTransform::scale(-1.0f, 1.0f, getWidth() / 2, getHeight() / 2));
		g.drawImageWithin(m_kitty, 0, 0, getWidth(), getHeight(), RectanglePlacement::fillDestination);
	}
	if (m_num_noise_points > 0)
	{
		std::mt19937 rnd;
		std::uniform_int<int> distx(0, getWidth());
		std::uniform_int<int> disty(0, getHeight());
		g.setColour(Colours::white);
		for (int i = 0; i < m_num_noise_points; ++i)
		{
			int xcor = distx(rnd);
			int ycor = disty(rnd);
			g.drawRect(xcor, ycor, 1, 1);
		}
	}
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

void EvilPluginAudioProcessorEditor::heapTrash()
{
	MemoryBlock block;
	MemoryOutputStream stream{ block,true };
	File file("foo.wav");
	stream << file;
	m_anim.CurveFunc = identity<double>;
	m_anim.start(5.0, [this](Animator::State s, double x) 
	{
		m_num_noise_points = x * 10000;
		if (s == Animator::State::Finished)
			m_num_noise_points = 0;
		repaint();
	});
}
