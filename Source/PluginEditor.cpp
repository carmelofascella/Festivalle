/*
    ==============================================================================
    
    This file was auto-generated!
    
    It contains the basic framework code for a JUCE plugin editor.
    
    ==============================================================================
    */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginDajeAudioProcessorEditor::PluginDajeAudioProcessorEditor(PluginDajeAudioProcessor& p)
: AudioProcessorEditor(&p), processor(p), startTime(Time::getMillisecondCounterHiRes() * 0.001),
forwardFFT(PluginDajeAudioProcessor::fftOrder), spectrogramImage(Image::RGB, 512, 512, true)
{
    processor.addChangeListener(this);
    
    midiOutput = MidiOutput::openDevice(0);
    
    addAndMakeVisible(buttonMidi);
    buttonMidi.setButtonText("Button Midi (81)");
    buttonMidi.onClick = [this] { setNoteNumber(81); };
    
    addAndMakeVisible(thresholdSlider);
    thresholdSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    thresholdSlider.setRange(0.1, 10);
    thresholdSlider.setValue(5);
    processor.setThreshold(thresholdSlider.getValue());
    thresholdSlider.addListener(this);
    
    addAndMakeVisible(midiMessagesBox);
    midiMessagesBox.setMultiLine(true);
    midiMessagesBox.setReturnKeyStartsNewLine(true);
    midiMessagesBox.setReadOnly(true);
    midiMessagesBox.setScrollbarsShown(true);
    midiMessagesBox.setCaretVisible(false);
    midiMessagesBox.setPopupMenuEnabled(true);
    midiMessagesBox.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
    midiMessagesBox.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
    midiMessagesBox.setColour(TextEditor::shadowColourId, Colour(0x16000000));
    
    startTimerHz(60);
    setSize(700, 500);
}

PluginDajeAudioProcessorEditor::~PluginDajeAudioProcessorEditor()
{
}

//==============================================================================
void PluginDajeAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black);
    g.setOpacity(1.0f);
    g.drawImage(spectrogramImage, getLocalBounds().toFloat());
    
}

void PluginDajeAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto halfWidth = getWidth() / 2;
    
    auto buttonsBounds = getLocalBounds().withWidth(halfWidth).reduced(10);
    
    buttonMidi.setBounds(buttonsBounds.getX(), 10, buttonsBounds.getWidth(), 20);
    
    thresholdSlider.setBounds(buttonsBounds.getX(), 50, buttonsBounds.getWidth(), 20);
    
    midiMessagesBox.setBounds(getLocalBounds().withWidth(halfWidth).withX(halfWidth).reduced(10));
}

//MIDI==========================================================================
static String getMidiMessageDescription(const MidiMessage& m)
{
    if (m.isNoteOn())           return "Note on " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
    if (m.isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
    if (m.isProgramChange())    return "Program change " + String(m.getProgramChangeNumber());
    if (m.isPitchWheel())       return "Pitch wheel " + String(m.getPitchWheelValue());
    if (m.isAftertouch())       return "After touch " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + String(m.getAfterTouchValue());
    if (m.isChannelPressure())  return "Channel pressure " + String(m.getChannelPressureValue());
    if (m.isAllNotesOff())      return "All notes off";
    if (m.isAllSoundOff())      return "All sound off";
    if (m.isMetaEvent())        return "Meta event";
    
    if (m.isController())
    {
        String name(MidiMessage::getControllerName(m.getControllerNumber()));
        
        if (name.isEmpty())
            name = "[" + String(m.getControllerNumber()) + "]";
        
        return "Controller " + name + ": " + String(m.getControllerValue());
    }
    
    return String::toHexString(m.getRawData(), m.getRawDataSize());
}

void PluginDajeAudioProcessorEditor::setNoteNumber(int noteNumber)
{
    //auto message = MidiMessage::noteOn(midiChannel, noteNumber, (uint8)100);
    auto message = MidiMessage::controllerEvent(midiChannel, 0, noteNumber);
    message.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001 - startTime);
    
    midiOutput->sendMessageNow(message);
    
    addMessageToList(message);
}

void PluginDajeAudioProcessorEditor::logMessage(const String& m)
{
    midiMessagesBox.moveCaretToEnd();
    midiMessagesBox.insertTextAtCaret(m + newLine);
}

void PluginDajeAudioProcessorEditor::addMessageToList(const MidiMessage& message)
{
    auto time = message.getTimeStamp();
    
    auto hours = ((int)(time / 3600.0)) % 24;
    auto minutes = ((int)(time / 60.0)) % 60;
    auto seconds = ((int)time) % 60;
    auto millis = ((int)(time * 1000.0)) % 1000;
    
    auto timecode = String::formatted("%02d:%02d:%02d.%03d",
                                      hours,
                                      minutes,
                                      seconds,
                                      millis);
    
    logMessage(timecode + "  -  " + getMidiMessageDescription(message));
}

void PluginDajeAudioProcessorEditor::sliderValueChanged(Slider * slider)
{
    if (slider == &thresholdSlider) {
        processor.setThreshold(slider->getValue());
    }
    
}

void PluginDajeAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    setNoteNumber(processor.prova);
}


void PluginDajeAudioProcessorEditor::drawNextLineOfSpectrogram()
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);                       // [1]
    forwardFFT.performFrequencyOnlyForwardTransform(processor.getFFTData());                                        // [2]
    auto maxLevel = FloatVectorOperations::findMinAndMax(processor.getFFTData(), processor.fftSize / 2);                      // [3]
    for (auto y = 1; y < imageHeight; ++y)                                                            // [4]
    {
        auto skewedProportionY = 1.0f - std::exp(std::log(y / (float)imageHeight) * 0.2f);
        auto fftDataIndex = jlimit(0, processor.fftSize / 2, (int)(skewedProportionY * processor.fftSize / 2));
        auto level = jmap(processor.getFFTDataIndex(fftDataIndex), 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
        spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level, 1.0f));   // [5]
    }
}

void PluginDajeAudioProcessorEditor::timerCallback()
{
    if (processor.getNextFFTBlockReady())
    {
        drawNextLineOfSpectrogram();
        processor.setNextFFTBlockReady(false);
        repaint();
    }
}
