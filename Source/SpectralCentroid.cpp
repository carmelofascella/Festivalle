/*
  ==============================================================================

    SpectralCentroid.cpp
    Created: 29 May 2019 12:00:51am
    Author:  Carmelo Fascella

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SpectralCentroid.h"
#include "PluginProcessor.h"

//==============================================================================
SpectralCentroid::SpectralCentroid(PluginDajeAudioProcessor& p)
:Thread("SpectralCentroidThread"), processor(p)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

SpectralCentroid::~SpectralCentroid()
{
}

void SpectralCentroid::paint (Graphics& g)
{
   
}

void SpectralCentroid::resized()
{

}

void SpectralCentroid::run()
{
    spectralCentroidFunction();
    
}

void SpectralCentroid::spectralCentroidFunction()
{
    float num=0, den=0, leftVal=0;
    
    for(int i=0;i<PluginDajeAudioProcessor::fftSize;i++)   //ho fatto solo canale sinistro
    {
        leftVal = processor.fftDataL[i];
        
        num += leftVal * (i+1);
        den += leftVal;
        
        
    }
    
    centroid = num/den;
    
    //printf("CENTROID: %f\n", centroid);
    sendChangeMessage();
}
