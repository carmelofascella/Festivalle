<p align="center"> <img width="877" height="494" src="images/Festivalle_polimi.JPG" > </p>

# 1. The concept

The goal of this project is the automated control of the dynamic lighting of an architectural structure composed by LED stripes: this is done by a plug-in which analyzes the mixer-output audio signal in real-time.

# 2. Plugin Interface
![gif](images/interface.gif)
The Plug-in is all made by using the C++ JUCE application framework. 
# 3. The Algorithm

The Algorithm is based on few simples points:

- Beat detection frame by frame from the signal and the BPM calculus from it;
- Features extraction from the actual audio frame: audio panning, Audio Spectral Centroid (for audio "birghtness" calculation) and velocity (as an indicator of the audio intensity);
- Once features are extracted, a MIDI message created by a set of rules related to those characteristics is sent, in order to activate 3N possible light patterns.

##  3.1 Beat Tracking algorithm
The beat tracking algorithm is mainly based on a statistical model which use the energy content of the audio signal.

The basic idea of the algorithm is to use a simple statistical model based on sound energy. We can calculate the average energy of a couple of seconds of the sound before the current playback and compare this with the current energy of the sound, if the difference of energy pass a certain threshold we can say there is a beat.

The first choise to make for the analysis of the audio frame is the window: the one chosen here is a rectangular one 1024 samples large with 0% overlap, to avoid latency problems and because we do not care a specific frequency tracking. The sampling frequency that we are considering is 44100 Hz.

<p align="center"> <img width="877" height="494" src="images/beattrack_0(1).png" > </p>

Since this Plug-in is mainly thought for electro-music situations, we are mostly interested in a few range of frequencies: the choice has been made considering the "snare" [range]  and "kick" [range]  frequency range where we are sure to find the beat of interest.

<p align="center"> <img width="475" height="159" src="images/beattrack_0(2).png" > </p>

For every 1024-samples frame we calculate the energy associated. Since we are analyzing 1024 samples of audio, to take 1 second history we need to store 43 blocks in an array and then calculate the average energy for that second. Then, every 1024 samples a threshold is calculated based on the variance of the "history energy array" calculated before and that is updated every 1024 samples to improve the real-time performances. If the difference of energy pass that certain threshold we can say there is a beat.

<p align="center"> <img width="1076" height="460" src="images/beattrack(3).png" > </p>





# 4. Feature extraction phase
##  4.1 Panning
##  4.2 Audio Spectral Centroid
##  4.3 Velocity

# 5. Lights' animation 
##  5.1 Rules for animations' choice
##  5.2 Animations management

# 6.The rendering phase: 3D Mapper + Plugin 




# Live Performance on stage
