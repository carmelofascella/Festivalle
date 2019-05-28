/*
==============================================================================

This file was auto-generated!

It contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginDajeAudioProcessor::PluginDajeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
}

PluginDajeAudioProcessor::~PluginDajeAudioProcessor()
{
}

//==============================================================================
const String PluginDajeAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool PluginDajeAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool PluginDajeAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool PluginDajeAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double PluginDajeAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int PluginDajeAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
				// so this should be at least 1, even if you're not really implementing programs.
}

int PluginDajeAudioProcessor::getCurrentProgram()
{
	return 0;
}

void PluginDajeAudioProcessor::setCurrentProgram(int index)
{
}

const String PluginDajeAudioProcessor::getProgramName(int index)
{
	return {};
}

void PluginDajeAudioProcessor::changeProgramName(int index, const String& newName)
{
}

//==============================================================================
void PluginDajeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..
}

void PluginDajeAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginDajeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void PluginDajeAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	numSample = buffer.getNumSamples();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
        
		auto* channelData = buffer.getReadPointer(channel);
        
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			
            //if(channel==0){
            //    leftChannel[sample] = buffer.getSample(0, sample);
            //}
            
            //if(channel==1){
                //rightChannel[sample] = buffer.getSample(1, sample);
                //
                //totChannel[sample] = (leftChannel[sample])*(leftChannel[sample]) + (rightChannel[sample]*rightChannel[sample]);


                    pushNextSampleIntoFifo(channelData[sample], channel);
            //}
			//channelData[sample] = buffer.getSample(channel, sample) * rawVolume;
           
            
            
            
			
			//prova = round((channelData[sample]*127));
			////
			//    if (channelData[sample] > threshold) {
			//    sendChangeMessage();
			//}
			// count++;
			//if (count > getSampleRate()) {
            //    //printf("ZZ: %d\n", count);
            //    printf("%d\n", buffer.getNumSamples());
			////sendChangeMessage();
			//count = 0;
			// }



		}




	}


}

//==============================================================================
bool PluginDajeAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PluginDajeAudioProcessor::createEditor()
{
	return new PluginDajeAudioProcessorEditor(*this);
}

//==============================================================================
void PluginDajeAudioProcessor::getStateInformation(MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void PluginDajeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new PluginDajeAudioProcessor();
}

double PluginDajeAudioProcessor::setThreshold(double value) {
	return threshold = value;
}

void PluginDajeAudioProcessor::pushNextSampleIntoFifo(float sample, int channel) noexcept
{
	if (channel == 0)
	{
		if (fifoIndexL == fftSize)    // [8]
		{
			if (!nextFFTBlockReady) // [9]
			{
				zeromem(fftDataL, sizeof(fftDataL));
				memcpy(fftDataL, fifoL, sizeof(fifoL));
				//nextFFTBlockReady = true;
			}
			fifoIndexL = 0;
		}
		fifoL[fifoIndexL++] = sample;  // [9]
	}
	else if(channel == 1)
	{
		if (fifoIndexR == fftSize)    // [8]
		{
			if (!nextFFTBlockReady) // [9]
			{
				zeromem(fftDataR, sizeof(fftDataR));
				memcpy(fftDataR, fifoR, sizeof(fifoR));
				nextFFTBlockReady = true;
			}
			fifoIndexR = 0;
		}
		fifoR[fifoIndexR++] = sample;  // [9]
	}
}


/*float* PluginDajeAudioProcessor::getFFTData() {
	return fftData;
}

float PluginDajeAudioProcessor::getFFTDataIndex(int index) {
	return fftData[index];
}*/

bool PluginDajeAudioProcessor::getNextFFTBlockReady() {
	return nextFFTBlockReady;
}

void PluginDajeAudioProcessor::setNextFFTBlockReady(bool setup) {
	nextFFTBlockReady = setup;
}

void PluginDajeAudioProcessor::setThreadBoolean(bool setup){
    threadFlag = setup;
}
