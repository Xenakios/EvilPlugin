/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

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
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EvilPluginAudioProcessorEditor)
};
