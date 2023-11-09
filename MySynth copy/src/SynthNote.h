//
//  SynthNote.h
//  MySynth Plugin Header File - for individual notes
//
//  Used to declare objects and data structures used by the plugin.
//

#include "SynthPlugin.h"

#pragma once

//================================================================================
// MyNote - object representing a single note (within the synthesiser - see above)
//================================================================================

class MyNote : public APDI::Synth::Note
{
public:
    MyNote(MySynth* synthesiser) : Note(synthesiser), fFrequency(440.f), fLevel(1.f) { }
    
    MySynth* getSynthesiser() { return (MySynth*)synthesiser; }
    
    void onStartNote (int pitch, float velocity);
    bool onStopNote (float velocity);
    void onPitchWheel (int value);
    void onControlChange (int controller, int value);
    float relayLFO();
    
    bool process (float** outputBuffer, int numChannels, int numSamples);
    
    int modTarget;
    float lfo1;
    
private:
    // Declare note class variables here
    float fFrequency;
    float fLevel;
    float fSinLevel;
    float fAttack;
    float fDecay;
    float fSustain;
    float fRelease;
    float sawLevel;
    float sqrLevel;
    float fAttack2;
    float fRelease2;
    int envelopeType;
    Envelope envelope;
    Envelope envelope2;
    
    Sine signalGenerator;
    SawWave sawGenerator;
    Sine Modulator;
    
    LPF Filter1;
    float filtFreq;
    
    bool oscRep;
    
    float modFreq;
    int modIndex;
    
    float tremoloRate;
    float tremoloDepth;
    
    
    
};
