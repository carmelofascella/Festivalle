/*
==============================================================================

This file was auto-generated!

It contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
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
    
	//Beat detection methods=========================
	void beatDetection();
	//float performEnergyFFT();
    float performEnergyFFT(int index);
	void fillEnergyHistory(float energy);
	void thresholdCalculus();
	float averageEnergyHistory();
	float varianceEnergyHistory(float average, std::queue<std::vector<float>> tempQueue, int index);

    float averageQueue(std::queue<std::vector<float>> temporalQueue, int index);
    void fillBPMQueue(float firstTime);
    
    void BPMDetection(float timeNow);
    void medianFilterFunction();
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

	//44100 / fftSize (modificarlo se serve)
	int dim = round(processor.getSampleRate() / processor.fftSize);
    
	//float energyHistory[43];
    //float energyHistoryOld[43];
    
  
    
    int oldCount=0;
    int energyIndex = 0;
    
    std::queue<std::vector<float>> energyHistory;
    
    std::priority_queue<float> bpmQueue;
	
    
    float BPMthreshold[2];
    float maxThresh = 0;
    
	float BPMsum = 0;
	float BPMsumq = 0;
	float varianceBeat = 50; //alta all'inizio
	int numBeat=0;
	int numBeatSize = 12;
    float BPM=0;
	float prevTime = 0;
    
	std::queue<float> deltaTQueue;

    float minAbs=0;
    float maxAbs=0;

	int kickmin = 0;
	int kickmax = 0;
	int snaremin = 0;
	int snaremax = 0;
   
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginDajeAudioProcessorEditor)
};
