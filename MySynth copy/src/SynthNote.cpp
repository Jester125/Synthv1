//
//  SynthNote.cpp
//  MySynth Plugin Source Code - for individual notes
//
//  Used to define the bodies of functions used by the plugin, as declared in SynthPlugin.h.
//


#include "SynthNote.h"
#include "SynthPlugin.h"

//================================================================================
// MyNote - object representing a single note (within the synthesiser - see above)
//================================================================================

// Triggered when a note is started (use to initialise / prepare note processing)
void MyNote::onStartNote (int pitch, float velocity)
{
    // convert note number to fundamental frequency (Hz)
    fFrequency = 440.f * pow (2.f, (pitch - 69.f) / 12.f);
    fLevel = velocity;                      // store velocity 
    
    fAttack = parameters[15];
    fDecay = parameters[16];
    fSustain = parameters[17];
    fRelease = parameters[18];
    envelopeType = parameters[2];
    
    sawLevel = parameters[7] * 0.2;
    fAttack2 = parameters[13];
    fRelease2 = parameters[14];
    filtFreq = parameters[12];
    oscRep = parameters[4];
    
    tremoloRate = parameters[5] * 10;
    tremoloDepth = parameters[6];
    modTarget = parameters[3];
    
    fSinLevel = parameters[19];
    
    signalGenerator.reset();
    signalGenerator.setFrequency(fFrequency);
    sawGenerator.reset();
    Modulator.reset(); //this is for the LFO
    Modulator.setFrequency(tremoloRate);
    
    // Sin Wav
    
    if (envelopeType == 0) { // "ADSR"
        envelope.set(Envelope::Points(0,0)(fAttack,1)(fDecay + fAttack,fSustain));
        envelope.setLoop(fAttack + fDecay + 1, fAttack + fDecay + 1);
//        envelope.setLoop(2, 2);
    }
    else if (envelopeType == 1) { // "ASR"
        envelope.set(Envelope::Points(0,0)(fAttack,fSustain));
        envelope.setLoop(fAttack + 1, fAttack + 1);
    }
    else if (envelopeType == 2) { // "AR"
        envelope.set(Envelope::Points(0,0)(fAttack,fSustain)(fAttack + fRelease, 0));
    }
    
    // Saw Wav
    
    envelope2.set(Envelope::Points(0,0)(fAttack2,1)(fAttack2 + fRelease2, 0));
    if (oscRep == 1) {
        envelope2.setLoop(0, fAttack2 + fRelease2);
    }
    else {
        envelope2.resetLoop();
    }
    
    
    
    
}
    
// Triggered when a note is stopped (return false to keep the note alive)
bool MyNote::onStopNote (float velocity)
{
    if (envelopeType == 0 || envelopeType == 1){
        envelope.release(fRelease); // tell the envelope to fade out in 0.5s
        return false; // return false to keep the note alive

    }
    else {
        return false;
    }
    
}
    

void MyNote::onPitchWheel (int value){

}
 
void MyNote::onControlChange (int controller, int value){

}

float MyNote::relayLFO() {
    return lfo1;
}
    
// Called to render the note's next buffer of audio (generates the sound)
// (return false to terminate the note)
bool MyNote::process (float** outputBuffer, int numChannels, int numSamples)
{
    float fMix = 0;
    float* pfOutBuffer0 = outputBuffer[0], *pfOutBuffer1 = outputBuffer[1];
    
    lfo1 = Modulator.tick() * tremoloDepth + 0.5;
    
    oscRep = parameters[4];
    modIndex = parameters[9] * 10;
    if (modTarget == 2) { // FM Mod
        modFreq = ((parameters[8] - 0.5) * 100) * lfo1;

    } else { // any of the others
        modFreq = ((parameters[8] - 0.5) * 100);
    }

    
    float ifd = modIndex * modFreq; // this is for FM modulaton
    float carrier = fFrequency;
    
    sawGenerator.setFrequency(modFreq * ifd + carrier);
    
//    float fGain = parameters[0];
    
    while(numSamples--)
    {
        float fSin = signalGenerator.tick();
        fSin *= fSinLevel;
        float fSaw = sawGenerator.tick();
        lfo1 = Modulator.tick() * tremoloDepth + 0.5;
        
        Filter1.setCutoff(filtFreq);
        
        if (modTarget == 0) { // Sin
            fSin *= envelope.tick();
            fSin *= fLevel;
            fSin *= lfo1;
            fSaw *= envelope2.tick();
            fSaw *= sawLevel;
            fMix = fSin + Filter1.tick(fSaw);
        }
        else if (modTarget == 1) { // Saw
            fSin *= envelope.tick();
            fSin *= fLevel;
            fSaw *= envelope2.tick();
            fSaw *= sawLevel;
            fSaw *= lfo1;
            fMix = fSin + Filter1.tick(fSaw);
        }
        else {
            fSin *= envelope.tick();
            fSin *= fLevel;
            fSaw *= envelope2.tick();
            fSaw *= sawLevel;
            fMix = fSin + Filter1.tick(fSaw);
        }
        
        *pfOutBuffer0++ = fMix * 0.4;
        *pfOutBuffer1++ = fMix * 0.4;
    }

    return envelope.getStage() != Envelope::STAGE::ENV_OFF;
}

