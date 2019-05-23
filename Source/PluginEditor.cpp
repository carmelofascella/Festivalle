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
    
    fftSize = processor.fftSize;
    
	addAndMakeVisible(actualBPM);
	actualBPM.setText("BPM: calculate...");
	actualBPM.setReadOnly(true);
    
    addAndMakeVisible(tapTempo);
    tapTempo.setButtonText("Tap Tempo");
    tapTempo.onClick = [this] { if(onOff) manualBPM(); };
    
    addAndMakeVisible(manualMode);
    manualMode.setButtonText("Manual Mode: off");
    manualMode.onClick = [this] {
        onOff = !onOff;
        if(onOff) {
            numBeat = 0;
            timeAverage = 0;
            manualMode.setButtonText("Manual Mode: on");
			beforeTransient = false;
			transient = false;
			transientAttack.setText("transientAttack: off");
        }
        else {
            BPMsum = 0;
            BPMsumq = 0;
            varianceBeat = 50; //alta all'inizio
            numBeat = 0;
            prevTime = 0;
            while(!deltaTQueue.empty()) {
                deltaTQueue.pop();
            }
            manualMode.setButtonText("Manual Mode: off");
        }
    };

    addAndMakeVisible(panCount);
    
	addAndMakeVisible(actualVar);
	addAndMakeVisible(minimumVar);
	addAndMakeVisible(transientAttack);
	actualVar.setText("actualVar: calculate...");
	minimumVar.setText("minimumVar: calculate...");
	transientAttack.setText("transientAttack: off");
	actualVar.setReadOnly(true);
	minimumVar.setReadOnly(true);
	transientAttack.setReadOnly(true);

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

	actualBPM.setBounds(buttonsBounds.getX(), 10, buttonsBounds.getWidth(), 20);

	midiMessagesBox.setBounds(getLocalBounds().withWidth(halfWidth).withX(halfWidth).reduced(10));
    
    tapTempo.setBounds(buttonsBounds.getX(), 140, buttonsBounds.getWidth(), 20);
    
    manualMode.setBounds(buttonsBounds.getX(), 170, buttonsBounds.getWidth(), 20);
	
	actualVar.setBounds(buttonsBounds.getX(), 40, buttonsBounds.getWidth(), 20);

	minimumVar.setBounds(buttonsBounds.getX(), 70, buttonsBounds.getWidth(), 20);

	transientAttack.setBounds(buttonsBounds.getX(), 100, buttonsBounds.getWidth(), 20);
    
    panCount.setBounds(buttonsBounds.getX(), 130, buttonsBounds.getWidth(), 20);
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
    
    double timeNow = Time::getMillisecondCounterHiRes() * 0.001;
    
    if(timeNow - prevTime - startTime > 0.3)   //200bpm massimi
    {
		BPMDetection(timeNow);
		prevTime = timeNow - startTime; //occhio
        message.setTimeStamp(timeNow - startTime);
        midiOutput->sendMessageNow(message);
        addMessageToList(message);
    }
    
   
}

void PluginDajeAudioProcessorEditor::BPMDetection(double timeNow) 
{
	if (transient)
	{
		if (timeNow - transientStartTime > 3)
		{
			transient = false;
			transientAttack.setText("transientAttack: off");
		}
	}

	else 
	{
		double deltaT = timeNow - prevTime - startTime;
		deltaTQueue.push(deltaT);

		numBeat++;

		//prevTime = timeNow - startTime;  //messo fuori, vediamo se è meglio

		BPMsum = BPMsum + deltaT;
		BPMsumq = BPMsumq + (deltaT * deltaT);

		if (numBeat >= numBeatSize)
		{
			double av = BPMsum / (numBeatSize);
			double var = BPMsumq / (numBeatSize)-(av * av);
			//double var = BPMsumq / (numBeatSize-1) + (av * av * numBeatSize) / (numBeatSize - 1) - (2 * av * BPMsum) / (numBeatSize - 1);

			if (var < varianceBeat)
			{
				BPM = round(60 / av);
				varianceBeat = var;
				actualBPM.setText("BPM: " + (String)BPM);
			}

			actualVar.setText("actualVar: " + (String)var);
			minimumVar.setText("minimumVar: " + (String)varianceBeat);

			BPMsum = BPMsum - deltaTQueue.front();
			BPMsumq = BPMsumq - (deltaTQueue.front() * deltaTQueue.front());
			deltaTQueue.pop();
			numBeat--;
		}
	}
}

void PluginDajeAudioProcessorEditor::manualBPM()
{
    double timeNow = Time::getMillisecondCounterHiRes() * 0.001;
    auto message = MidiMessage::controllerEvent(midiChannel, 0, 65);
    
    numBeat++;
    
    if(numBeat > 1)
        timeAverage = (timeAverage * (numBeat - 2) + (timeNow - prevTime - startTime)) / (numBeat - 1);
    
    prevTime = timeNow - startTime;
    
    if(numBeat > 1) {
        BPM = round(60 / timeAverage);
		actualBPM.setText("BPM: " + (String)BPM);
    }
    
    message.setTimeStamp((timeNow - startTime));
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
	/*if (slider == &thresholdSlider) {
		processor.setThreshold(slider->getValue());
	}*/  //Potrebbe servire

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
    
    /*
    if(!onOff)
        beatDetection();
	*/
    
    panningFeature();
    
    /*
	auto maxLevel = FloatVectorOperations::findMinAndMax(processor.getFFTData(), processor.fftSize / 2);                      // [3]
	for (auto y = 1; y < imageHeight; ++y)                                                           // [4]
	{
		auto skewedProportionY = 1.0f - std::exp(std::log(y / (float)imageHeight) * 0.2f);
		auto fftDataIndex = jlimit(0, processor.fftSize / 2, (int)(skewedProportionY * processor.fftSize / 2));
		auto level = jmap(processor.getFFTDataIndex(fftDataIndex), 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
		spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level, 1.0f));   // [5]
	}
     */
     
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
    
    energyRange[0] = performEnergyFFT(0);   //calcolo energia del range low-mid del singolo buffer
    energyRange[1] = performEnergyFFT(1);      

	if (energyRange[0] == 0 && energyRange[1] == 0 || isnan(energyRange[0]) && isnan(energyRange[1])) {
		beforeTransient = true;   //vuoto prima dell'inizio dell'attacco
	}
	else if (beforeTransient) {
		beforeTransient = false;
		transientAttack.setText("transientAttack: on");
		transientStartTime = Time::getMillisecondCounterHiRes() * 0.001;
		transient = true;  //inizio dell'attacco
	}

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
        sum = sum / ((kickmax - kickmin) * 2); //numero canali;
    }
    else if(index == 1)
    {
        for (int i = snaremin; i <= snaremax; i++) {  //SNARE   //OCCHIOOOOOO
        sum = sum + processor.fftDataL[i] + processor.fftDataR[i];
        //printf("%.4f\n", processor.getFFTData()[i]);
        }
        sum = sum / ((snaremax-snaremin) * 2); //numero canali;
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


void PluginDajeAudioProcessorEditor::panningFeature()
{
    
    float leftSPS[fftSize];
    float rightSPS[fftSize];
    float diffSPS[fftSize];
    float totSPS[fftSize];
    
    float leftVal = 0;
    float rightVal = 0;
    int countPos=0, countNeg =0, countZeros = 0;
    
    for(int i=0; i<=fftSize ;i++)
    {
        leftVal = processor.fftDataL[i];
        rightVal = processor.fftDataR[i];
        leftSPS[i] = abs(leftVal * rightVal) / abs(leftVal*leftVal);
        
        rightSPS[i] = abs(leftVal * rightVal) / abs(rightVal*rightVal);
        
        diffSPS[i] = leftSPS[i] - rightSPS[i];
        
        //totSPS[i] = 2 * ( (abs(leftVal * rightVal)) / ( abs(leftVal*leftVal) + abs(rightVal*rightVal)) );
 
        //panSpectrum[i] = (1 - totSPS[i])*diffSPS[i];
        
        if(diffSPS[i]>0)
        {
            diffSPS[i] = 1;
            countPos++;
        }
        
        else if(diffSPS[i]<0)
        {
            diffSPS[i] = -1;
            countNeg++;
        }
        
        else if(diffSPS[i]==0)
        {
            diffSPS[i] = 0;
            countZeros++;
        }
    }
    
    if(countPos>850)
    {
        panCount.setText("POSITIVE COUNT: " + (String)countPos + "NEGATIVE COUNT: " + (String)countNeg + "ZERO COUNT: " + (String)countZeros );
    }
    
    
}




