/*
  ==============================================================================

    SpectralCentroid.h
    Created: 29 May 2019 12:00:51am
    Author:  Carmelo Fascella

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
//==============================================================================
/*
*/
class SpectralCentroid    : public Component,
                            public Thread,
                            public ChangeBroadcaster
{
public:
    SpectralCentroid(PluginDajeAudioProcessor&);
    ~SpectralCentroid();

    void paint (Graphics&) override;
    void resized() override;
    void run();
    void powerSpectrumFunction();
    
    void spectralCentroidFunction();

    void centersGrativtyFunction();
    
    float centroidL=0;
    float centroidR=0;
    float centroidMid=0;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralCentroid)
    
    PluginDajeAudioProcessor& processor;
    float powerSpectrumL[PluginDajeAudioProcessor::fftSize];
    float powerSpectrumR[PluginDajeAudioProcessor::fftSize];
    
    float centersFreq[PluginDajeAudioProcessor::fftSize];

    
    int rangeFreq = round(processor.getSampleRate() / processor.fftSize);
    
};
