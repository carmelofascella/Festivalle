/*
  ==============================================================================

    PanFeature.h
    Created: 28 May 2019 7:49:32pm
    Author:  Carmelo Fascella

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class PanFeature    : public Component,
                      public Thread,
                      public ChangeBroadcaster
{
public:
    PanFeature(PluginProcessor&);
    ~PanFeature();

    
    void paint (Graphics&) override;
    void resized() override;
    void run();
    void panFeatureFunction();
    
    
    float panValue = 0;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanFeature);
    
    PluginProcessor& processor;
    

};
