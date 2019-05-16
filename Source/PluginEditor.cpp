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

    StringArray midiOutputList = MidiOutput::getDevices();
    int portIndex = midiOutputList.indexOf("loopMIDI Port");
    if (portIndex != -1)
        midiOutput = MidiOutput::openDevice(portIndex); //WINDOWS
    else
        midiOutput = MidiOutput::openDevice(0); //MAC
    

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
void PluginDajeAudioProcessorEditor::paint(Graphics& g)
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
    
    float timeNow = Time::getMillisecondCounterHiRes() * 0.001;
    
    if(timeNow - firstTime - startTime >0.3)
    {
        BPMDetection(timeNow);
        
        buttonMidi.setButtonText((String)BPM);
        
        message.setTimeStamp(timeNow - startTime);
        
        midiOutput->sendMessageNow(message);
        
        addMessageToList(message);
    }
    
   
}

void PluginDajeAudioProcessorEditor::BPMDetection(float timeNow)
{
    numBeat++;
    timeAverage = (timeAverage * (numBeat - 1) + (timeNow - firstTime - startTime)) / numBeat;
    firstTime = timeNow - startTime;
    
    //fillBPMQueue(firstTime);
    BPM = 60 / timeAverage;
}

//void PluginDajeAudioProcessorEditor::fillBPMQueue(float firstTime)
//{
//
//    bpmQueue.push(firstTime);
//    if(numBeat%7==0)
//    {
//        medianFilterFunction();
//    }
//
//}

//void PluginDajeAudioProcessorEditor::medianFilterFunction()
//{
//    for(int i=0;i<3;i++)
//    {
//        bpmQueue.pop();
//    }
//
//    bpmQueue.front()
//}

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
	setNoteNumber(80);
}


void PluginDajeAudioProcessorEditor::drawNextLineOfSpectrogram()
{
	auto rightHandEdge = spectrogramImage.getWidth() - 1;
	auto imageHeight = spectrogramImage.getHeight();
	spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);                       // [1]
    
    findRangeValueFunction(processor.getFFTData());
    
    //for(int i=0; i<processor.fftSize; i++){
    //    printf("%.2f ", processor.getFFTData()[i]);
    //}
    //printf("\n");
    
    
	forwardFFT.performFrequencyOnlyForwardTransform(processor.getFFTData());                         // [2]
    //printf("\nwe");
    
    
    //findRangeValueFunction(processor.getFFTData());     //da qui parte il processo di normalizzazione, per ora commentato perchè ho                                   visto che funziona meglio senza (la normalizzazione funziona)
    for(int i=0; i<processor.fftSize; i++){
        printf("%.2f ", processor.getFFTData()[i]);
    }
    printf("\n");
    
	beatDetection();
    
	auto maxLevel = FloatVectorOperations::findMinAndMax(processor.getFFTData(), processor.fftSize / 2);                      // [3]
	for (auto y = 1; y < imageHeight; ++y)                                                           // [4]
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

void PluginDajeAudioProcessorEditor::findRangeValueFunction(float* data)
{
    //float* data = processor.getFFTData();
    float min=data[0];
    float max=data[0];
    
    for(int i=1; i<processor.fftSize; i++){
        if(data[i]>max)
            max = data[i];
        if(data[i]<min)
            min = data[i];
    }
    
    
    
    scaleFunction(min, max, data);
    //scaleFunction(min, max, data);
}

void PluginDajeAudioProcessorEditor::scaleFunction(float min,float max,float* data)
{
    
    for(int i=0; i<processor.fftSize; i++){
       
        //processor.getFFTData()[i] =  ((data[i]-min) * (1-(-1))) / ((max-min)+(-1))  ;
        processor.getFFTData()[i] = 2 *((data[i] - min)/(max - min)) -1;
    }
    
    
}




//assunzioni: ogni secondo devo aggiornare la threshold.
//Per ogni fftData, mi calcolo l' energia del lowMidRange e lo confronto con la threshold
//Se è maggiore, è un beat
void PluginDajeAudioProcessorEditor::beatDetection() {

    float energyLowMid = performEnergyLowMidFFT();          //calcolo energia del range low-mid del singolo buffer
    
    
    //fillEnergyHistory(energyLowMid);
    //printf("Energy: %.3f", energyLowMid);
    energyHistory.push(energyLowMid);
    
    

    
    //if(energyHistory.size()>0)
    //{
    //    printf("testa della coda %d\n", energyHistory.size());
    //    printf("FRONT %.3f       ENERGY: %.3f\n", energyHistory.front(), energyLowMid);
    //}
    
    
   //for(int i=0;i<=1023; i++){
   //    printf("%.2f ", processor.getFFTData()[i]);
   //}
   //printf("\n________________________________\n");
    //printf("Thresh: %.4f   +   energy: %.4f    +   oldCount: %d\n", BPMthreshold, energyHistory.front() , oldCount);
    
    
    
    
    if (energyIndex > dim-1) {
        BPMthreshold = thresholdCalculus();
        
        
        //for(int i=0; i<dim; i++){
        //    energyHistoryOld[i] = energyHistory[i];
        //}
        
        //printf("Thresh: %.4f   +   energy: %.4f   \n", BPMthreshold, energyLowMid);
        
        
        if (energyLowMid > BPMthreshold) {        //confronto con 1 secondo di delay
            setNoteNumber(80);
            
            //printf("\n");
            //printf("BEAT\n");
            //for(int i=7; i<=17; i++){
            //    printf("sample: %f",processor.getFFTData()[i]);
            //    printf("  ");
            //}
            //printf("\n");
        }
        
        energyHistory.pop();
        
    }

     energyIndex++;
     
}

//fa la media dell energy history buffer
float PluginDajeAudioProcessorEditor::fullGas(std::queue<float> temporalQueue)
{
    float val, sum=0;
    
    while(temporalQueue.size()>0)
    {
        
        val=temporalQueue.front();
        sum = sum+val;
        temporalQueue.pop();
        
    }
    
    
    return sum/dim;
    

}


//considero i sample del low-mid range, che vanno dall' 8° al 18° sample del fftData (sono 11 sample)
float PluginDajeAudioProcessorEditor::performEnergyLowMidFFT(){
    
    float sum = 0;
    
    
    for (int i = 7; i <= 17; i++) {
        sum = sum + processor.getFFTData()[i];
        //printf("%.4f\n", processor.getFFTData()[i]);
    }
    
    
    return sum / 11;
}




//void PluginDajeAudioProcessorEditor::fillEnergyHistory(float energy) {
//    energyHistory[energyIndex] = energy;
//    energyIndex++;
//}


float PluginDajeAudioProcessorEditor::thresholdCalculus() {
    
    std::queue<float> temporalQueue = energyHistory;
    float average=fullGas(temporalQueue);
    
    std::queue<float> tempQueue = energyHistory;
    //float average = averageEnergyHistory();
    float variance = varianceEnergyHistory(average, tempQueue);
    return (-0.0025714 * variance)*average + 1.5142857;
    //return (-15 * variance + 1.55)*average;
}


//float PluginDajeAudioProcessorEditor::averageEnergyHistory() {
//    float sum = 0;
//    for (int i = 0; i < dim; i++) {
//        sum = sum + energyHistory.
//    }
//    return sum / dim;
//}

float PluginDajeAudioProcessorEditor::varianceEnergyHistory(float average, std::queue<float> tempQueue) {
    float sum = 0, val;
    //for (int i = 0; i < dim; i++) {
    //    sum = sum + (energyHistory[i] - average)*(energyHistory[i] - average);
    //}
    
    while(tempQueue.size()>0)
    {
        val=tempQueue.front();
        
        
        sum = sum + (val - average)*(val - average);
        tempQueue.pop();
        
    }
    return sum / dim;
}





