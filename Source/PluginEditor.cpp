#include "PluginProcessor.h"
#include "PluginEditor.h"

std::function<void(int)> g_stackoverflowcb;

void stackoverflowfunc1(int x)
{
    ++x;
    g_stackoverflowcb(x);
}
#ifdef FOOFAA
void writeEnvTestFile()
{
    WavAudioFormat format;
    File outfile("C:\\MusicAudio\\sourcesamples\\test_signals\\bkenv\\out1.wav");
    outfile.deleteFile();
    double outsr = 44100.0;
    FileOutputStream *stream = outfile.createOutputStream();
    AudioFormatWriter *writer = format.createWriterFor(stream, outsr, 1, 32, {}, 0);
    if (writer)
    {
        int procbufsize = 512;
        AudioBuffer<float> buf(1, procbufsize);
        Envelope env{{0.0, 0.0}, {0.75, 1.0}, {1.0, 0.0},  {1.5, 0.0}, {1.6, 0.5},
                     {1.7, 0.0}, {1.8, 0.0},  {1.81, 1.0}, {1.82, 0.0}};
        /*
        env.removePointsConditionally([](const EnvelopePoint& a)
        {
                return a.getY() >= 0.4 && a.getY() <= 0.6;
        });
        */
        // env.removePointsConditionally(ValueBetween(0.4, 0.6));
        env.scaleTimes(10.0);
        env.scaleAndShiftValues(2.0, -1.0);
        std::vector<double> envbuf(procbufsize);
        int pos = 0;
        while (pos < outsr * 20.0)
        {
            double t0 = pos / outsr;
            double t1 = (pos + procbufsize) / outsr;
            env.applyToBuffer(envbuf.data(), procbufsize, t0 - 0.0, t1 - 0.0);
            for (int i = 0; i < buf.getNumSamples(); ++i)
            {
                float s = sin(2 * 3.141592 / outsr * (pos + i) * 440.0);
                buf.setSample(0, i, envbuf[i]);
            }
            writer->writeFromAudioSampleBuffer(buf, 0, procbufsize);
            pos += procbufsize;
        }

        delete writer;
    }
    else
        delete stream;
}
#endif

//==============================================================================
EvilPluginAudioProcessorEditor::EvilPluginAudioProcessorEditor(EvilPluginAudioProcessor &p)
    : AudioProcessorEditor(&p), processor(p), m_mutex_thread(&p)
{
    // writeEnvTestFile();

    StringArray buttexts{"Access violation type 1", "Access violation type 2",
                         "Stack overflow",          "Divide by zero",
                         "Sleep in GUI thread",     "Sleep in audio thread"};
    std::vector<std::function<void(void)>> callbacks{
        [this]() { accessViolation1(); },
        [this]() { heapTrash(); },
        []() {
            g_stackoverflowcb = stackoverflowfunc1;
            volatile int x = 0;
            stackoverflowfunc1(x);
        },
        [this]() {
            // m_anim.CurveFunc = identity<double>;

            volatile int x = 0;
            volatile int y = x / 0;
        }

        ,
        [this]() {
            m_gui_is_sleeping = true;
            repaint();
            Timer::callAfterDelay(100, [this]() {
                Thread::sleep(5000);
                m_gui_is_sleeping = false;
                repaint();
            });
        },
        [this]() { processor.m_sleep_in_audio_thread = true; },
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
    tog->onClick = [this, togcap = tog.get()]() {
        if (m_mutex_thread.isThreadRunning() == false)
            m_mutex_thread.startThread();
        m_mutex_thread.m_lock_mutex = togcap->getToggleState();
    };
    m_buttons.push_back(std::move(tog));

    tog = std::make_unique<ToggleButton>();
    addAndMakeVisible(tog.get());
    tog->setButtonText("Use global variables (needs multiple plugin instances to break)");
    tog->onClick = [this, togcap = tog.get()]() {
        processor.m_use_global_variable = togcap->getToggleState();
    };
    m_buttons.push_back(std::move(tog));

    addAndMakeVisible(m_label_waste_gui_cpu);
    m_label_waste_gui_cpu.setText("Waste CPU in GUI thread", dontSendNotification);
    addAndMakeVisible(m_label_waste_audio_cpu);
    m_label_waste_audio_cpu.setText("Waste CPU in audio thread", dontSendNotification);
    addAndMakeVisible(m_label_waste_worker_cpu);
    m_label_waste_worker_cpu.setText("Waste CPU in worker thread", dontSendNotification);
    addAndMakeVisible(m_slider_waste_gui_cpu);
    m_slider_waste_gui_cpu.setRange(0.0, 100.0);
    m_slider_waste_gui_cpu.setValue(0.0, dontSendNotification);
    m_slider_waste_gui_cpu.setNumDecimalPlacesToDisplay(2);
    m_slider_waste_gui_cpu.setTextValueSuffix(" %");
    m_slider_waste_gui_cpu.onValueChange = [this]() {
        if (m_slider_waste_gui_cpu.getValue() > 0.0)
            startTimer(0, 100);
        else
            stopTimer(0);
    };

    addAndMakeVisible(m_slider_waste_audio_cpu);
    m_slider_waste_audio_cpu.setRange(0.0, 110.0);
    m_slider_waste_audio_cpu.setValue(0.0, dontSendNotification);
    m_slider_waste_audio_cpu.setNumDecimalPlacesToDisplay(2);
    m_slider_waste_audio_cpu.setTextValueSuffix(" %");
    m_slider_waste_audio_cpu.onValueChange = [this]() {
        processor.m_cpu_waste_amount = m_slider_waste_audio_cpu.getValue();
    };

    addAndMakeVisible(m_slider_waste_worker_cpu);
    m_slider_waste_worker_cpu.setRange(0.0, 100.0);
    m_slider_waste_worker_cpu.setValue(0.0, dontSendNotification);
    m_slider_waste_worker_cpu.setNumDecimalPlacesToDisplay(2);
    m_slider_waste_worker_cpu.setTextValueSuffix(" %");
    m_slider_waste_worker_cpu.onValueChange = [this]() {
        m_worker_cpu_waster.m_amount_to_waste = m_slider_waste_worker_cpu.getValue();
    };
    m_worker_cpu_waster.startThread();

    // addAndMakeVisible(m_thcomp);
    setSize(500, 340);
}

EvilPluginAudioProcessorEditor::~EvilPluginAudioProcessorEditor()
{
    m_mutex_thread.stopThread(1000);
    m_worker_cpu_waster.stopThread(1000);
}

//==============================================================================
void EvilPluginAudioProcessorEditor::paint(Graphics &g) { g.fillAll(Colours::red.darker()); }

void EvilPluginAudioProcessorEditor::resized()
{
    // m_thcomp.setBounds(getWidth()-200, 0, 200, getHeight());
    for (int i = 0; i < m_buttons.size(); ++i)
    {
        m_buttons[i]->setBounds(1, 1 + 30 * i, 100, 29);
        auto textbutton = dynamic_cast<TextButton *>(m_buttons[i].get());
        if (textbutton)
        {
            textbutton->changeWidthToFitText();
        }
        auto togglebutton = dynamic_cast<ToggleButton *>(m_buttons[i].get());
        if (togglebutton)
        {
            togglebutton->changeWidthToFitText();
        }
    }
    int yoffs = m_buttons.back()->getBottom() + 1;
    m_label_waste_gui_cpu.setBounds(1, yoffs, 200, 29);
    m_slider_waste_gui_cpu.setBounds(m_label_waste_gui_cpu.getRight() + 1, yoffs, 200, 29);
    yoffs = m_slider_waste_gui_cpu.getBottom() + 1;

    m_label_waste_audio_cpu.setBounds(1, yoffs, 200, 29);
    m_slider_waste_audio_cpu.setBounds(m_label_waste_audio_cpu.getRight(), yoffs, 200, 29);

    yoffs = m_slider_waste_audio_cpu.getBottom() + 1;
    m_label_waste_worker_cpu.setBounds(1, yoffs, 200, 29);
    m_slider_waste_worker_cpu.setBounds(m_label_waste_audio_cpu.getRight(), yoffs, 200, 29);
}

void EvilPluginAudioProcessorEditor::timerCallback(int id)
{
    if (id == 0)
    {
        double percent_to_waste = m_slider_waste_gui_cpu.getValue() / 100.0;
        if (percent_to_waste > 0.0)
        {
            double timetowaste = (double)getTimerInterval(0) * percent_to_waste;
            CPU_waster(m_rnd, timetowaste);
        }
    }
    if (id == 1)
    {
    }
}

void EvilPluginAudioProcessorEditor::heapTrash() {}

void EvilPluginAudioProcessorEditor::accessViolation1()
{
    int *ptr = nullptr;
    (*ptr)++;
}


