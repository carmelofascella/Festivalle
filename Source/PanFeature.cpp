/*
  ==============================================================================

    PanFeature.cpp
    Created: 28 May 2019 7:49:32pm
    Author:  Carmelo Fascella

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "PanFeature.h"
#include "PluginProcessor.h"

//==============================================================================
PanFeature::PanFeature (PluginDajeAudioProcessor& p)
:Thread("PanThread"), processor(p)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

PanFeature::~PanFeature()
{
}

void PanFeature::paint (Graphics& g)
{
    
}

void PanFeature::resized()
{
    

}



void PanFeature::run()
{
    panFeatureFunction();
}

void PanFeature::panFeatureFunction()
{
    
    int panLeft = 0, panRight = 0;
    
    for (int i = 0; i < processor.fftSize; i++)
    {
        if (processor.fftDataL[i] - processor.fftDataR[i] > 0)
        {
            panLeft++;
        }
        else if (processor.fftDataL[i] - processor.fftDataR[i] < 0)
        {
            panRight++;
        }
        
    }
    
    count ++;
    sendChangeMessage();
    //panCount.setText("L: " + (String)panLeft + " - R: " + (String)panRight);
    //printf("\n%d", count);
}
