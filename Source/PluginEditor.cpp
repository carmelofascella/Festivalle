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

	kickmin = round(60 / dim);
	kickmax = round(130 / dim);
	snaremin = round(301 / dim);
	snaremax = round(750 / dim);
    

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
    
    if(timeNow - prevTime - startTime > 0.3)
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
	float deltaT = timeNow - prevTime - startTime;
	deltaTQueue.push(deltaT);
	
	numBeat++;

	buttonMidi.setButtonText((String)numBeat);

	BPMsum = BPMsum + deltaT;
	BPMsumq = BPMsumq + deltaT * deltaT;

	prevTime = timeNow - startTime;
	
	if (numBeat >= numBeatSize)
	{
		float av = BPMsum / numBeatSize;
		float var = BPMsumq / numBeatSize - (av * av);
		if (var < varianceBeat)
		{
			BPM = 60 / av;
			varianceBeat = var;
		}
		BPMsum = BPMsum - deltaTQueue.front();
		BPMsumq = BPMsumq - deltaTQueue.front() * deltaTQueue.front();
		deltaTQueue.pop();
		numBeat--;
	}
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
	setNoteNumber(80);
}


void PluginDajeAudioProcessorEditor::drawNextLineOfSpectrogram()
{
	auto rightHandEdge = spectrogramImage.getWidth() - 1;
	auto imageHeight = spectrogramImage.getHeight();
	spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);                       // [1]
    
    
    
    //for(int i=0; i<processor.fftSize; i++){
    //    printf("%.2f ", processor.getFFTData()[i]);
    //}
    //printf("\n");
    
	forwardFFT.performFrequencyOnlyForwardTransform(processor.fftDataL);                         // [2]
	forwardFFT.performFrequencyOnlyForwardTransform(processor.fftDataR);
    //printf("\nwe");
    
    findRangeValueFunction(processor.fftDataL, 0);
	findRangeValueFunction(processor.fftDataR, 1);

    //findRangeValueFunction(processor.getFFTData());     //da qui parte il processo di normalizzazione, per ora commentato perchè ho                                   visto che funziona meglio senza (la normalizzazione funziona)
    //for(int i=0; i<processor.fftSize; i++){
    //    printf("%.2f ", processor.getFFTData()[i]);
    //}
    //printf("\n");
    
	beatDetection();
	
	/*auto maxLevel = FloatVectorOperations::findMinAndMax(processor.getFFTData(), processor.fftSize / 2);                      // [3]
	for (auto y = 1; y < imageHeight; ++y)                                                           // [4]
	{
		auto skewedProportionY = 1.0f - std::exp(std::log(y / (float)imageHeight) * 0.2f);
		auto fftDataIndex = jlimit(0, processor.fftSize / 2, (int)(skewedProportionY * processor.fftSize / 2));
		auto level = jmap(processor.getFFTDataIndex(fftDataIndex), 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
		spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level, 1.0f));   // [5]
	}*/
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

void PluginDajeAudioProcessorEditor::findRangeValueFunction(float* data, int index)
{
    //float* data = processor.getFFTData();
    
    for(int i=1; i<processor.fftSize; i++){
        if(data[i]>maxAbs)
            maxAbs = data[i];
        if(data[i]<minAbs)
            minAbs = data[i];
    }
    
    
    
    scaleFunction(data, index);
   
}

void PluginDajeAudioProcessorEditor::scaleFunction(float* data, int index)
{
    if(index == 0)
		for(int i=0; i<processor.fftSize; i++){
			//processor.getFFTData()[i] =  ((data[i]-min) * (1-(-1))) / ((max-min)+(-1))  ;
			processor.fftDataL[i] = 1 *((data[i] - minAbs)/(maxAbs - minAbs)) -0;
		}
	else
		for (int i = 0; i<processor.fftSize; i++) {
			//processor.getFFTData()[i] =  ((data[i]-min) * (1-(-1))) / ((max-min)+(-1))  ;
			processor.fftDataR[i] = 1 * ((data[i] - minAbs) / (maxAbs - minAbs)) - 0;
		}
    
}




//assunzioni: ogni secondo devo aggiornare la threshold.
//Per ogni fftData, mi calcolo l' energia del lowMidRange e lo confronto con la threshold
//Se è maggiore, è un beat
void PluginDajeAudioProcessorEditor::beatDetection() {

    float energyRange[2];
    
    energyRange[0] = performEnergyFFT(0);
    energyRange[1] = performEnergyFFT(1);          //calcolo energia del range low-mid del singolo buffer
    
    
    //fillEnergyHistory(energyLowMid);
    //printf("Energy: %.3f", energyLowMid);
    
    std::vector<float> fftResult;
    fftResult.reserve(2);
    for(int i = 0; i < 2; i++)
    {
        fftResult.push_back(energyRange[i]);
    }
    
    energyHistory.push(fftResult);
    
    
    //printf("energyIndex: %d\n", energyIndex%43);
    
    //if(energyHistory.size()>0)
    //{
    //    printf("testa della coda %d\n", energyHistory.size());
    //printf("FRONT %.3f       ENERGY: %.3f\n", energyHistory.front(), energyLowMid);
    //}
    
    
   //for(int i=0;i<=1023; i++){
   //    printf("%.2f ", processor.getFFTData()[i]);
   //}
   //printf("\n________________________________\n");
    //printf("Thresh: %.4f   +   energy: %.4f    +   oldCount: %d\n", BPMthreshold, energyHistory.front() , oldCount);
    
    
    
    
    if (energyIndex >= dim-1) {
        thresholdCalculus();  //calcolus BPMThreshold
        
        
        //for(int i=0; i<dim; i++){
        //    energyHistoryOld[i] = energyHistory[i];
        //}
        
        //printf("Thresh: %.4f   +   energy: %.4f    +dim: %d   \n", BPMthreshold[0], energyRange[0], energyHistory.size());
        
        
        if (energyRange[0] - 0.05 > BPMthreshold[0] || energyRange[1] - 0.005 > BPMthreshold[1]) {        //confronto con 1 secondo di delay
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
float PluginDajeAudioProcessorEditor::averageQueue(std::queue<std::vector<float>> temporalQueue, int index)
{
    std::vector<float> val;
    float sum = 0;
    
    if(index == 0)
    {
        while(temporalQueue.size() > 0)
        {
        val = temporalQueue.front();
        sum = sum + val[0];
        temporalQueue.pop();
        }
    }
    
    else if(index == 1)
    {
        while(temporalQueue.size() > 0)
        {
            val = temporalQueue.front();
            sum = sum + val[1];
            temporalQueue.pop();
        }
    }
    
    
    return sum/dim;
    
	
}


//considero i sample del low-mid range, che vanno dall' 8° al 18° sample del fftData (sono 11 sample)
float PluginDajeAudioProcessorEditor::performEnergyFFT(int index){
    
    float sum = 0;
    
    if(index == 0)
    {
        for (int i = kickmin; i <= kickmax; i++) {  //KICK
            sum = sum + processor.fftDataL[i] + processor.fftDataR[i];
            //printf("%.4f\n", processor.getFFTData()[i]);
        }
        sum = sum / ((130/dim - 60/dim) * 2); //numero canali;
    }
    else if(index == 1)
    {
        for (int i = snaremin; i <= snaremax; i++) {  //SNARE   //OCCHIOOOOOO
        sum = sum + processor.fftDataL[i] + processor.fftDataR[i];
        //printf("%.4f\n", processor.getFFTData()[i]);
        }
        sum = sum / ((750/dim - 301/dim) * 2); //numero canali;
    }
    return sum;
}




//void PluginDajeAudioProcessorEditor::fillEnergyHistory(float energy) {
//    energyHistory[energyIndex] = energy;
//    energyIndex++;
//}


void PluginDajeAudioProcessorEditor::thresholdCalculus() {
    
    std::queue<std::vector<float>> temporalQueue = energyHistory;
    
    float average[2];
    average[0] = averageQueue(temporalQueue, 0);
    average[1] = averageQueue(temporalQueue, 1);
    
    temporalQueue = energyHistory;
    //float average = averageEnergyHistory();
    float variance[2];
    variance[0] = varianceEnergyHistory(average[0], temporalQueue, 0);
    variance[1] = varianceEnergyHistory(average[1], temporalQueue, 1);
    //return (-0.0025714 * variance)*average + 1.5142857;
    
    BPMthreshold[0] = (-15 * variance[0] + 1.55) * average[0];
    BPMthreshold[1] = (-15 * variance[1] + 1.55) * average[1];
}


//float PluginDajeAudioProcessorEditor::averageEnergyHistory() {
//    float sum = 0;
//    for (int i = 0; i < dim; i++) {
//        sum = sum + energyHistory.
//    }
//    return sum / dim;
//}

float PluginDajeAudioProcessorEditor::varianceEnergyHistory(float average, std::queue<std::vector<float>> tempQueue, int index) {
    
    std::vector<float> val;
    float sum = 0;
    
    if(index == 0)
    {
        while(tempQueue.size() > 0)
        {
            val=tempQueue.front();
            sum = sum + (val[0] - average)*(val[0] - average);
            tempQueue.pop();
        }
    }
    
    else if(index == 1)
    {
        while(tempQueue.size() > 0)
        {
            val=tempQueue.front();
            sum = sum + (val[1] - average)*(val[1] - average);
            tempQueue.pop();
        }
    }
    
    
    return sum/dim;
    //for (int i = 0; i < dim; i++) {
    //    sum = sum + (energyHistory[i] - average)*(energyHistory[i] - average);
    //}
}





