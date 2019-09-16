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
	forwardFFT(PluginDajeAudioProcessor::fftOrder), spectrogramImage(Image::RGB, 512, 512, true), panFeature(processor), beatDetector(processor), spectralCentroid(processor)
{
    panFeature.addChangeListener(this);
    beatDetector.addChangeListener(this);
    spectralCentroid.addChangeListener(this);
    
	processor.addChangeListener(this);

    StringArray midiOutputList = MidiOutput::getDevices();
    int portIndex = midiOutputList.indexOf("loopMIDI Port");
    if (portIndex != -1)
        midiOutput = MidiOutput::openDevice(portIndex); //WINDOWS
    else
        midiOutput = MidiOutput::openDevice(0); //MAC


	addAndMakeVisible(panCount);
	panCount.setText("panCount: calculate...");
	panCount.setReadOnly(true);
	panCount.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	panCount.setColour(TextEditor::shadowColourId, Colour(0x16000000));


	addAndMakeVisible(spectralCount);
	spectralCount.setText("spectralCount: calculate...");
	spectralCount.setReadOnly(true);
	spectralCount.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	spectralCount.setColour(TextEditor::shadowColourId, Colour(0x16000000));

	addAndMakeVisible(velocitySubBar);
	velocitySubBar.setReadOnly(true);
	velocitySubBar.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	velocitySubBar.setColour(TextEditor::shadowColourId, Colour(0x16000000));
	velocitySubBar.setText("Velocity: ");
	velocitySubBar.setJustification(4);

	addAndMakeVisible(velocityMessage);
	//velocityMessage.setText("velocityMessage: calculate...");
	velocityMessage.setReadOnly(true);
	velocityMessage.setColour(TextEditor::backgroundColourId, Colours::greenyellow);

	addAndMakeVisible(actualBPM);
	actualBPM.setText("BPM: calculate...");
	actualBPM.setReadOnly(true);
	actualBPM.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	actualBPM.setColour(TextEditor::shadowColourId, Colour(0x16000000));

    addAndMakeVisible(tapTempo);
    tapTempo.setButtonText("Tap Tempo"); 
	tapTempo.setColour(TextButton::buttonColourId, Colour(0x32ffffff));
    tapTempo.onClick = [this] { if(onOff) manualBPM(); };

	addAndMakeVisible(resetVarianceBeat);
	resetVarianceBeat.setButtonText("BPM Recalculate");
	resetVarianceBeat.setColour(TextButton::buttonColourId, Colour(0x32ffffff));
	resetVarianceBeat.onClick = [this] { varianceBeat = 50; };

    addAndMakeVisible(manualMode);
    manualMode.setButtonText("Manual Mode");
	manualMode.setColour(TextButton::buttonColourId, Colour(0x32ffffff));
	manualMode.setColour(TextButton::buttonOnColourId, Colour(0x32ffffff));
	manualMode.setColour(TextButton::textColourOnId, Colours::greenyellow);
	manualMode.setClickingTogglesState(true);
    manualMode.onClick = [this] {
        onOff = !onOff;
        if(onOff) {
            numBeat = 0;
            timeAverage = 0;
            //manualMode.setButtonText("Manual Mode: on");
			if (beatDetector.transient) {
				minVelocity = std::numeric_limits<float>::max();
				maxVelocity = std::numeric_limits<float>::min();
			}
			beatDetector.beforeTransient = false;
			beatDetector.transient = false;
			beatDetector.reducedTransient = false;
			transientAttack.applyColourToAllText(Colours::white);
        }
        else {
            BPMsum = 0;
            BPMsumq = 0;
            varianceBeat = 50; //alta all'inizio
            numBeat = 0;
            prevTime = 0;
			prevTimeTemp = 0;
            while(!deltaTQueue.empty()) {
                deltaTQueue.pop();
            }
            //manualMode.setButtonText("Manual Mode: off");
        }
    };

	addAndMakeVisible(actualVar);
	addAndMakeVisible(minimumVar);
	addAndMakeVisible(transientAttack);
	actualVar.setText("actualVar: calculate...");
	minimumVar.setText("minimumVar: calculate...");
	transientAttack.setText("transientAttack");
	actualVar.setReadOnly(true);
	minimumVar.setReadOnly(true);
	transientAttack.setReadOnly(true);
	actualVar.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	actualVar.setColour(TextEditor::shadowColourId, Colour(0x16000000));
	minimumVar.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	minimumVar.setColour(TextEditor::shadowColourId, Colour(0x16000000));
	transientAttack.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	transientAttack.setColour(TextEditor::shadowColourId, Colour(0x16000000));

	addAndMakeVisible(midiMessagesBox);
	midiMessagesBox.setMultiLine(true);
	midiMessagesBox.setReturnKeyStartsNewLine(true);
	midiMessagesBox.setReadOnly(true);
	midiMessagesBox.setScrollbarsShown(true);
	midiMessagesBox.setCaretVisible(false);
	midiMessagesBox.setPopupMenuEnabled(true);
	midiMessagesBox.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
	//midiMessagesBox.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
	midiMessagesBox.setColour(TextEditor::shadowColourId, Colour(0x16000000));
	getLookAndFeel().setColour(ScrollBar::thumbColourId, Colours::greenyellow);

	startTimerHz(60);
	setSize(860, 420);

	addAndMakeVisible(midiChannelSelector);
	midiChannelSelector.setRange(1, 16, 1);
	midiChannelSelector.setTextValueSuffix(" Ch");
	midiChannelSelector.setValue(midiChannel);
	midiChannelSelector.addListener(this);

	addAndMakeVisible(bpmMaxSelector);
	bpmMaxSelector.setRange(1, 200, 1);
	bpmMaxSelector.setTextValueSuffix(" BPM");
	bpmMaxSelector.setValue(bpmMax);
	bpmMaxSelector.addListener(this);

	addAndMakeVisible(numAnimSelector);
	numAnimSelector.setRange(3, 42, 1);
	numAnimSelector.setTextValueSuffix(" N");
	numAnimSelector.setValue(numAnimazioni);
	numAnimSelector.addListener(this);

	getLookAndFeel().setColour(Slider::thumbColourId, Colours::greenyellow);

    /*addAndMakeVisible(button0);
    addAndMakeVisible(button1);
    addAndMakeVisible(button2);
    addAndMakeVisible(button3);
    addAndMakeVisible(button4);
    addAndMakeVisible(button5);
    addAndMakeVisible(button6);
    addAndMakeVisible(button7);
    addAndMakeVisible(button8);
    addAndMakeVisible(button9);
    addAndMakeVisible(button10);
    addAndMakeVisible(button11);
    addAndMakeVisible(button12);
    addAndMakeVisible(button13);
    addAndMakeVisible(button14);
    addAndMakeVisible(button15);
    addAndMakeVisible(button16);
    addAndMakeVisible(button17);
    addAndMakeVisible(button18);
    addAndMakeVisible(button19);
    addAndMakeVisible(button20);*/
 
    button0.setButtonText((String)("0"));
    button1.setButtonText((String)("1"));
    button2.setButtonText((String)("2"));
    button3.setButtonText((String)("3"));
    button4.setButtonText((String)("4"));
    button5.setButtonText((String)("5"));
    button6.setButtonText((String)("6"));
    button7.setButtonText((String)("7"));
    button8.setButtonText((String)("8"));
    button9.setButtonText((String)("9"));
    button10.setButtonText((String)("10"));
    button11.setButtonText((String)("11"));
    button12.setButtonText((String)("12"));
    button13.setButtonText((String)("13"));
    button14.setButtonText((String)("14"));
    button15.setButtonText((String)("15"));
    button16.setButtonText((String)("16"));
    button17.setButtonText((String)("17"));
    button18.setButtonText((String)("18"));
    button19.setButtonText((String)("19"));
    button20.setButtonText((String)("20"));

    
    button0.onClick = [this] { setNoteNumber(0,120); };
    button1.onClick = [this] { setNoteNumber(1,12); };
    button2.onClick = [this] { setNoteNumber(2,121); };
    button3.onClick = [this] { setNoteNumber(3,120); };
    button4.onClick = [this] { setNoteNumber(4,12); };
    button5.onClick = [this] { setNoteNumber(5,25); };
    button6.onClick = [this] { setNoteNumber(6,12); };
    button7.onClick = [this] { setNoteNumber(7,123); };
    button8.onClick = [this] { setNoteNumber(8,121); };
    button9.onClick = [this] { setNoteNumber(9,120); };
    button10.onClick = [this] { setNoteNumber(10,12); };
    button11.onClick = [this] { setNoteNumber(11,121); };
    button12.onClick = [this] { setNoteNumber(12,120); };
    button13.onClick = [this] { setNoteNumber(13,12); };
    button14.onClick = [this] { setNoteNumber(14,121); };
    button15.onClick = [this] { setNoteNumber(15,120); };
    button16.onClick = [this] { setNoteNumber(16,12); };
    button17.onClick = [this] { setNoteNumber(17,121); };
    button18.onClick = [this] { setNoteNumber(18,120); };
    button19.onClick = [this] { setNoteNumber(19,12); };
    button20.onClick = [this] { setNoteNumber(20,121); };

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
    
    tapTempo.setBounds(buttonsBounds.getX(), 230, buttonsBounds.getWidth(), 20);
    
    manualMode.setBounds(buttonsBounds.getX(), 260, buttonsBounds.getWidth(), 20);

	resetVarianceBeat.setBounds(buttonsBounds.getX(), 290, buttonsBounds.getWidth(), 20);

	midiChannelSelector.setBounds(buttonsBounds.getX(), 330, buttonsBounds.getWidth(), 20);

	bpmMaxSelector.setBounds(buttonsBounds.getX(), 360, buttonsBounds.getWidth(), 20);

	numAnimSelector.setBounds(buttonsBounds.getX(), 390, buttonsBounds.getWidth(), 20);
	
	actualVar.setBounds(buttonsBounds.getX(), 40, buttonsBounds.getWidth(), 20);

	minimumVar.setBounds(buttonsBounds.getX(), 70, buttonsBounds.getWidth(), 20);

	transientAttack.setBounds(buttonsBounds.getX(), 100, buttonsBounds.getWidth(), 20);

	panCount.setBounds(buttonsBounds.getX(), 130, buttonsBounds.getWidth(), 20);

	spectralCount.setBounds(buttonsBounds.getX(), 160, buttonsBounds.getWidth(), 20);

	velocitySubBar.setBounds(buttonsBounds.getX(), 190, buttonsBounds.getWidth(), 20);
    
	velocityMessage.setBounds(buttonsBounds.getX(), 190, buttonsBounds.getWidth(), 20);

    button0.setBounds(buttonsBounds.getX(), 230, 20, 20);
    button1.setBounds(buttonsBounds.getX()+20, 230, 20, 20);
    button2.setBounds(buttonsBounds.getX()+40, 230, 20, 20);
    button3.setBounds(buttonsBounds.getX()+60, 230, 20, 20);
    button4.setBounds(buttonsBounds.getX()+80, 230, 20, 20);
    button5.setBounds(buttonsBounds.getX()+100, 230, 20, 20);
    
    button6.setBounds(buttonsBounds.getX(), 250, 20, 20);
    button7.setBounds(buttonsBounds.getX()+20, 250, 20, 20);
    button8.setBounds(buttonsBounds.getX()+40, 250, 20, 20);
    button9.setBounds(buttonsBounds.getX()+60, 250, 20, 20);
    button10.setBounds(buttonsBounds.getX()+80, 250, 20, 20);
    button11.setBounds(buttonsBounds.getX()+100, 250, 20, 20);
    
    button12.setBounds(buttonsBounds.getX(), 270, 20, 20);
    button13.setBounds(buttonsBounds.getX()+20, 270, 20, 20);
    button14.setBounds(buttonsBounds.getX()+40, 270, 20, 20);
    button15.setBounds(buttonsBounds.getX()+60, 270, 20, 20);
    button16.setBounds(buttonsBounds.getX()+80, 270, 20, 20);
    button17.setBounds(buttonsBounds.getX()+100, 270, 20, 20);
    
    button18.setBounds(buttonsBounds.getX()+60, 290, 20, 20);
    button19.setBounds(buttonsBounds.getX()+80, 290, 20, 20);
    button20.setBounds(buttonsBounds.getX()+100, 290, 20, 20);
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

		return "Controller " + name/* + ": " + String(m.getControllerValue())*/;
	}

	return String::toHexString(m.getRawData(), m.getRawDataSize());
}

void PluginDajeAudioProcessorEditor::setNoteNumber(int faderNumber, int velocity)
{
    //auto message = MidiMessage::noteOn(midiChannel, noteNumber, (uint8)100);
    auto message = MidiMessage::controllerEvent(midiChannel, faderNumber, velocity);
    
    double timeNow = Time::getMillisecondCounterHiRes() * 0.001;
    
	if (faderNumber == 0) {
		message.setTimeStamp(timeNow - startTime);
		midiOutput->sendMessageNow(message);
		//printf("\n%f", spectralCentroid.centroidL);

		//velocityMessage.setText((String)maxVelocity);
		velocityMessage.setBounds(getLocalBounds().withWidth(getWidth() / 2).reduced(10).getX(), 190, ((velocity * getLocalBounds().withWidth(getWidth() / 2).reduced(10).getWidth()) / 127), 20);
		velocitySubBar.setText("Velocity: " + (String)velocity);
		//countVelMess++;
	}

	else if (timeNow - prevTime - startTime > 0.3 && !onOff)   //200bpm massimi
	{
		BPMDetection(timeNow);
		prevTime = timeNow - startTime; //occhio

		if (timeNow - prevTimeTemp - startTime > (60 / bpmMax))
		{
			prevTimeTemp = timeNow - startTime;
			lightNumber = setLightNumber(faderNumber, velocity);
			
			message = MidiMessage::controllerEvent(midiChannel, lightNumber, velocity);
			message.setTimeStamp(timeNow - startTime);
			midiOutput->sendMessageNow(message);
			addMessageToList(message);
			//printf("\n%f", spectralCentroid.centroidL);
		}
	}

	else if (onOff)
	{
		lightNumber = setLightNumber(faderNumber, velocity);

		message = MidiMessage::controllerEvent(midiChannel, lightNumber, velocity);
		message.setTimeStamp(timeNow - startTime);
		midiOutput->sendMessageNow(message);
		addMessageToList(message);
	}
   
}

void PluginDajeAudioProcessorEditor::BPMDetection(double timeNow) 
{
	if (beatDetector.transient)
	{
		transientAttack.applyColourToAllText(Colours::greenyellow);
		if (timeNow - beatDetector.transientStartTime > 10)
		{
			beatDetector.transient = false;
			minVelocity = std::numeric_limits<float>::max();
			maxVelocity = std::numeric_limits<float>::min();
			transientAttack.applyColourToAllText(Colours::white);
		}
		if (beatDetector.reducedTransient)
		{
			beatDetector.reducedTransient = false;
			minVelocity = std::numeric_limits<float>::max();
			maxVelocity = std::numeric_limits<float>::min();
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
			double var = BPMsumq / (numBeatSize) - (av * av);
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
    //auto message = MidiMessage::controllerEvent(midiChannel, 7, ((int)(Time::getMillisecondCounterHiRes()) % 128));
	designLightPattern();

    numBeat++;
    
    if(numBeat > 1)
        timeAverage = (timeAverage * (numBeat - 2) + (timeNow - prevTime - startTime)) / (numBeat - 1);
    
    prevTime = timeNow - startTime;
    
    if(numBeat > 1) {
        BPM = round(60 / timeAverage);
		actualBPM.setText("BPM: " + (String)BPM);
    }
    
   /* message.setTimeStamp((timeNow - startTime));
    midiOutput->sendMessageNow(message);
    addMessageToList(message);*/
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

	if (lightNumber < 10)
		logMessage(timecode + " - |0" + (String)lightNumber + "|" + " " + getMidiMessageDescription(message));
	else
		logMessage(timecode + " - |" + (String)lightNumber + "|" + " " + getMidiMessageDescription(message));
}

void PluginDajeAudioProcessorEditor::sliderValueChanged(Slider * slider)
{
	/*if (slider == &thresholdSlider) {
		processor.setThreshold(slider->getValue());
	}*/  //Potrebbe servire

	if (slider == &midiChannelSelector) {
		midiChannel = slider->getValue();
		//tapTempo.setButtonText((String)midiChannel);
	}

	if (slider == &bpmMaxSelector) {
		bpmMax = slider->getValue();
		//tapTempo.setButtonText((String)bpmMax);
	}

	if (slider == &numAnimSelector) {
		numAnimazioni = slider->getValue();
		//tapTempo.setButtonText((String)numAnimazioni);
	}
}

void PluginDajeAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{

    /*if(source == &panFeature )
    {
        //END OF THE PAN FEATURE THREAD (to be handle)
        //printf("\n%d", panFeature.panValue);
        
    }
    
    else if(source == &beatDetector)
    {
        //END OF THE BEAT DETECTION THREAD
        //setNoteNumber(beatDetector.beatTime);
        
    }
    
    else if(source == &spectralCentroid)
    {
        //END OF THE SPECTRAL CENTROID THREAD (to be handle)
        //printf("\n%f", spectralCentroid.centroidL);

    }*/
	designLightPattern();
    
}


void PluginDajeAudioProcessorEditor::drawNextLineOfSpectrogram()
{
	auto rightHandEdge = spectrogramImage.getWidth() - 1;
	auto imageHeight = spectrogramImage.getHeight();
	spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);                       // [1]
    
	forwardFFT.performFrequencyOnlyForwardTransform(processor.fftDataL);                         // [2]
	forwardFFT.performFrequencyOnlyForwardTransform(processor.fftDataR);
    
	float fftData[PluginDajeAudioProcessor::fftSize / 2];

	for (int i = 0; i < processor.fftSize / 2; i++) {
		fftData[i] = processor.fftDataL[i] + processor.fftDataR[i];
	}

	auto maxLevel = FloatVectorOperations::findMinAndMax(fftData, processor.fftSize / 2); // [3]
	
	for (auto y = 1; y < imageHeight; ++y)                                                            // [4]
	{
		auto skewedProportionY = 1.0f - std::exp(std::log(y / (float)imageHeight) * 0.2f);
		auto fftDataIndex = jlimit(0, processor.fftSize / 2, (int)(skewedProportionY * processor.fftSize / 2));
		auto level = jmap(processor.fftDataL[fftDataIndex] + processor.fftDataR[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
		spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level, 1.0f));   // [5]
	}

    findRangeValueFunction(processor.fftDataL, 0);
	findRangeValueFunction(processor.fftDataR, 1);

    
	
    
    if(!onOff)
        beatDetector.run();         //START BEAT DETECTION THREAD
    
    panFeature.run();               //START PAN FEATURE THREAD
    
    spectralCentroid.run();         //START SPECTRAL CENTROID THREAD
    

	//-----FORSE
	float energySum = beatDetector.performEnergyFFT(2);
	setNoteNumber(0, velocityRange(10*log10(energySum + std::numeric_limits<float>::epsilon())));

}

int PluginDajeAudioProcessorEditor::velocityRange(float energyAmount) {
	if (energyAmount > maxVelocity/* || countVelMess > 18 * 20*/) //18: numero di messaggi al secondo circa
		maxVelocity = energyAmount;
	if (energyAmount < minVelocity/* || countVelMess > 18 * 20*/)
		minVelocity = energyAmount;
	//if (countVelMess > 18 * 20) //ogni 20 secondi di musica circa ricalcolo max e min
		//countVelMess = 0;

	float ris;

	if (maxVelocity - minVelocity != 0) {
		ris = (int)(127 * ((energyAmount - minVelocity) / (maxVelocity - minVelocity)) - 0);
		if (ris > 127)
			ris = 127;
		return ris;
	}
	else
		return 0;

}

void PluginDajeAudioProcessorEditor::timerCallback()
{
	if (beatDetector.beforeTransient) {
		transientAttack.applyColourToAllText(Colours::white);
	}

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

    for(int i = 0; i < PluginDajeAudioProcessor::fftSize / 2; i++) {
        if(data[i] > maxAbs)
            maxAbs = data[i];
        if(data[i] < minAbs)
            minAbs = data[i];
    }
    
    scaleFunction(data, index);
   
}

void PluginDajeAudioProcessorEditor::scaleFunction(float* data, int index)
{
    if(index == 0 && maxAbs - minAbs != 0)
		for(int i=0; i < PluginDajeAudioProcessor::fftSize; i++){
			//processor.getFFTData()[i] =  ((data[i]-min) * (1-(-1))) / ((max-min)+(-1))  ;
			processor.fftDataL[i] = 1 *((data[i] - minAbs) / (maxAbs - minAbs)) - 0;
		}
	else if(index == 1 && maxAbs - minAbs != 0)
		for (int i = 0; i < PluginDajeAudioProcessor::fftSize; i++) {
			//processor.getFFTData()[i] =  ((data[i]-min) * (1-(-1))) / ((max-min)+(-1))  ;
			processor.fftDataR[i] = 1 * ((data[i] - minAbs) / (maxAbs - minAbs)) - 0;
		}
    
}

int PluginDajeAudioProcessorEditor::setLightNumber(int index, int limit) 
{
	if (repetition == index)
	{
		if (repetition >= limit) {
			repetition = index - 1;
		}
		else
		{
			repetition = index + 1;
		}
	}
	else
	{
		repetition = index;
	}

	//tapTempo.setButtonText((String)repetition);
	return repetition;

}

void PluginDajeAudioProcessorEditor::designLightPattern()
{
    
	for (int i = 0; i < numAnimazioni; i++) {
		if (panFeature.panValue < -0.5) {   //+-0.5 va bene per il panning? io metterei di più, tipo +-0.8
			if (spectralRangeMin <= -3.5) {
				if (spectralCentroid.centroidL <= -3.5)
				{
					setNoteNumber(i + 1, numAnimazioni); //velocity usata come limite nel setLightNumber
					break;
				}
			}
			else if(spectralRangeMin > 1.5) {
				if (spectralCentroid.centroidL > 1.5)
				{
					setNoteNumber(i + 1, numAnimazioni);
					break;
				}
			}
			else {
				if (spectralCentroid.centroidL > spectralRangeMin - (spectralRange / (numAnimazioni - 2)) && spectralCentroid.centroidL <= spectralRangeMin)
				{
					setNoteNumber(i + 1, numAnimazioni);
					break;
				}
			}
		}

		else if (panFeature.panValue > 0.5) {
			if (spectralRangeMin <= -3.5) {
				if (spectralCentroid.centroidR <= -3.5)
				{
					setNoteNumber(i + 1 + numAnimazioni * 2, numAnimazioni * 3);
					break;
				}
			}
			else if (spectralRangeMin > 1.5) {
				if (spectralCentroid.centroidR > 1.5)
				{
					setNoteNumber(i + 1 + numAnimazioni * 2, numAnimazioni * 3);
					break;
				}
			}
			else {
				if (spectralCentroid.centroidR > spectralRangeMin - (spectralRange / (numAnimazioni - 2)) && spectralCentroid.centroidR <= spectralRangeMin)
				{
					setNoteNumber(i + 1 + numAnimazioni * 2, numAnimazioni * 3);
					break;
				}
			}
		}

		else {
			if (spectralRangeMin <= -3.5) {
				if (spectralCentroid.centroidMid <= -3.5)
				{
					setNoteNumber(i + 1 + numAnimazioni, numAnimazioni * 2);
					break;
				}
			}
			else if (spectralRangeMin > 1.5) {
				if (spectralCentroid.centroidMid > 1.5)
				{
					setNoteNumber(i + 1 + numAnimazioni, numAnimazioni * 2);
					break;
				}
			}
			else {
				if (spectralCentroid.centroidMid > spectralRangeMin - (spectralRange / (numAnimazioni - 2)) && spectralCentroid.centroidMid <= spectralRangeMin)
				{
					setNoteNumber(i + 1 + numAnimazioni, numAnimazioni * 2);
					break;
				}
			}
		}

		spectralRangeMin += (spectralRange / (numAnimazioni - 2));
	}

	spectralRangeMin = -3.5;

    /*if(panFeature.panValue<-0.5) //caso left
    {
        if(spectralCentroid.centroidL < -3.5)
        {
            setNoteNumber(1, rand()%100);
        }
        
        else if(spectralCentroid.centroidL>=-3.5 && spectralCentroid.centroidL<-2.5)
        {
            setNoteNumber(2, rand()%100);
        }
        
        else if(spectralCentroid.centroidL>=-2.5 && spectralCentroid.centroidL<-1.5)
        {
            setNoteNumber(3, rand()%100);
        }
        
        else if(spectralCentroid.centroidL>=-1.5 && spectralCentroid.centroidL<-0.5)
        {
            setNoteNumber(4, rand()%100);
        }
        else if(spectralCentroid.centroidL>=-0.5 && spectralCentroid.centroidL<0.5)
        {
            setNoteNumber(5, rand()%100);
        }
        
        else if(spectralCentroid.centroidL>=0.5 && spectralCentroid.centroidL<1.5)
        {
            setNoteNumber(6, rand()%100);
        }
        else if(spectralCentroid.centroidL>1.5)
        {
            setNoteNumber(7, rand()%100);
        }
    }
    
    else if(panFeature.panValue>=-0.5 && panFeature.panValue<0.5)
    {
        if(spectralCentroid.centroidMid<-3.5)
        {
            setNoteNumber(8, rand()%100);
        }
        
        else if(spectralCentroid.centroidMid>=-3.5 && spectralCentroid.centroidMid<-2.5)
        {
            setNoteNumber(9, rand()%100);
        }
        
        else if(spectralCentroid.centroidMid>=-2.5 && spectralCentroid.centroidMid<-1.5)
        {
            setNoteNumber(10, rand()%100);
        }
        
        else if(spectralCentroid.centroidMid>=-1.5 && spectralCentroid.centroidMid<-0.5)
        {
            setNoteNumber(11, rand()%100);
        }
        else if(spectralCentroid.centroidMid>=-0.5 && spectralCentroid.centroidMid<0.5)
        {
            setNoteNumber(12, rand()%100);
        }
        
        else if(spectralCentroid.centroidMid>=0.5 && spectralCentroid.centroidMid<1.5)
        {
            setNoteNumber(13, rand()%100);
        }
        else if(spectralCentroid.centroidMid>=1.5)
        {
            setNoteNumber(14, rand()%100);
        }
        
    }
    
    else if(panFeature.panValue>=0.5)
    {
        if(spectralCentroid.centroidR<-3.5)
        {
            setNoteNumber(15, rand()%100);
        }
        
        else if(spectralCentroid.centroidR>=-3.5 && spectralCentroid.centroidR<-2.5)
        {
            setNoteNumber(16, rand()%100);
        }
        
        else if(spectralCentroid.centroidR>=-2.5 && spectralCentroid.centroidR<-1.5)
        {
            setNoteNumber(17, rand()%100);
        }
        
        else if(spectralCentroid.centroidR>=-1.5 && spectralCentroid.centroidR<-0.5)
        {
            setNoteNumber(18, rand()%100);
        }
        else if(spectralCentroid.centroidR>=-0.5 && spectralCentroid.centroidR<0.5)
        {
            setNoteNumber(19, rand()%100);
        }
        
        else if(spectralCentroid.centroidR>=0.5 && spectralCentroid.centroidR<1.5)
        {
            setNoteNumber(20, rand()%100);
        }
        else if(spectralCentroid.centroidR>=1.5)
        {
            setNoteNumber(21, rand()%100);
        }
        
    }*/
    
	panCount.setText("Panning: " + (String)panFeature.panValue);
	spectralCount.setText("SpectralCentroidMid: " + (String)spectralCentroid.centroidMid);
}
