/*
==============================================================================

This file was auto-generated!

It contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PanFeature.h"
#include "BeatDetector.h"
#include "SpectralCentroid.h"
#include <iostream>
#include <queue>
#include <stack>


//==============================================================================
/**
*/
class PluginDajeAudioProcessorEditor : public AudioProcessorEditor,
	public ChangeListener,
	public Slider::Listener,
	private Timer
{
public:
	PluginDajeAudioProcessorEditor(PluginDajeAudioProcessor&);
	~PluginDajeAudioProcessorEditor();

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;
	void changeListenerCallback(ChangeBroadcaster* source) override;
	void sliderValueChanged(Slider* slider) override;

	void setNoteNumber(int noteNumber);
	void logMessage(const String& m);
	void addMessageToList(const MidiMessage& message);

	void drawNextLineOfSpectrogram();
	void timerCallback() override;

    void findRangeValueFunction(float* fftData, int index);
    void scaleFunction(float* data, int index);
    
    //void findRangeValueFunction(float *data);
    //void scaleFunction(float min,float max,float* data);
    
	//Beat detection graphics methods=========================
    void BPMDetection(double timeNow);
    void manualBPM();
    

private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	PluginDajeAudioProcessor& processor;
	TextEditor midiMessagesBox;
    
	TextEditor actualBPM;
	TextEditor actualVar;
	TextEditor minimumVar;
	TextEditor transientAttack;
	TextEditor panCount;

    TextButton tapTempo;
    TextButton manualMode;
    bool onOff = false;

	int midiChannel = 10;
	double startTime;

	MidiOutput *midiOutput;

	dsp::FFT forwardFFT;            // [3]
	Image spectrogramImage;

	
   
    float maxThresh = 0;
    
	double BPMsum = 0;
	double BPMsumq = 0;
	double varianceBeat = 50; //alta all'inizio
	int numBeat = 0;
	int numBeatSize = 12;
    int BPM = 0;
	double prevTime = 0;
	std::queue<double> deltaTQueue;
    
    double timeAverage = 0; //manual mode

    float minAbs=0;
    float maxAbs=0;

    bool beforeTransient = false;
    bool transient = false;
    double transientStartTime = 0;
    
    PanFeature panFeature;
    BeatDetector beatDetector;
    SpectralCentroid spectralCentroid;
   
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginDajeAudioProcessorEditor)
};
