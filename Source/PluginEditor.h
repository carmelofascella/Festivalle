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
class PluginDajeAudioProcessorEditor  : public AudioProcessorEditor,
public ChangeListener,
public Slider::Listener,
private Timer
{
public:
    PluginDajeAudioProcessorEditor (PluginDajeAudioProcessor&);
    ~PluginDajeAudioProcessorEditor();
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void sliderValueChanged(Slider* slider) override;
    
    void setNoteNumber(int noteNumber);
    void logMessage(const String& m);
    void addMessageToList(const MidiMessage& message);
    
    void drawNextLineOfSpectrogram();
    void timerCallback() override;
    
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginDajeAudioProcessor& processor;
    TextButton buttonMidi;
    TextEditor midiMessagesBox;
    
    Slider thresholdSlider;
    
    int midiChannel = 10;
    double startTime;
    
    MidiOutput *midiOutput;
    
    dsp::FFT forwardFFT;            // [3]
    Image spectrogramImage;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginDajeAudioProcessorEditor)
};
