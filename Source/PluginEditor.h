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

	void setNoteNumber(int faderNumber, int velocity);
	void logMessage(const String& m);
	void addMessageToList(const MidiMessage& message);

	void drawNextLineOfSpectrogram();
	void timerCallback() override;

    void findRangeValueFunction(float* fftData, int index);
    void scaleFunction(float* data, int index);

	int velocityRange(float energyAmount);
    
    //void findRangeValueFunction(float *data);
    //void scaleFunction(float min,float max,float* data);
    
	//Beat detection graphics methods=========================
    void BPMDetection(double timeNow);
    void manualBPM();

	int setLightNumber(int index, int limit);
    void designLightPattern();

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
	TextEditor spectralCount;
	TextEditor velocitySubBar;
	TextEditor velocityMessage;
	
	//int countVelMess = 0;

    TextButton tapTempo;
    TextButton manualMode;
    bool onOff = false;

	TextButton resetVarianceBeat;

	int midiChannel = 8;
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
	double prevTimeTemp = 0;
	std::queue<double> deltaTQueue;
    
    double timeAverage = 0; //manual mode

    float minAbs = 10000; //per assegnarli con sicurezza al primo ciclo
    float maxAbs = -10000;
    
    PanFeature panFeature;
    BeatDetector beatDetector;
    SpectralCentroid spectralCentroid;
    
	Slider midiChannelSelector;
	
	Slider bpmMaxSelector;
	float bpmMax = 200;

	Slider numAnimSelector;
	float numAnimazioni = 7;
	float spectralRange = 5; //1.5 - (-3.5)
	float spectralRangeMin = -3.5;
	int repetition = 0;
	int lightNumber = 11; //inizializzato solo per ragioni di sicurezza

    TextButton button0;
    TextButton button1;
    TextButton button2;
    TextButton button3;
    TextButton button4;
    TextButton button5;
    TextButton button6;
    TextButton button7;
    TextButton button8;
    TextButton button9;
    TextButton button10;
    TextButton button11;
    TextButton button12;
    TextButton button13;
    TextButton button14;
    TextButton button15;
    TextButton button16;
    TextButton button17;
    TextButton button18;
    TextButton button19;
    TextButton button20;

	float minVelocity = std::numeric_limits<float>::max(); //per assegnarli con sicurezza al primo ciclo
	float maxVelocity = std::numeric_limits<float>::min();
   
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginDajeAudioProcessorEditor)
};
