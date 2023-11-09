//
//  SynthPlugin.cpp
//  MySynth Plugin Source Code - for the top-level synthesiser object
//
//  Used to define the bodies of functions used by the plugin, as declared in SynthPlugin.h.
//

#include "SynthPlugin.h"
#include "SynthNote.h"

////////////////////////////////////////////////////////////////////////////
// SYNTH - represents the whole synthesis plugin
////////////////////////////////////////////////////////////////////////////

extern "C" {
    // Called to create the synthesiser (used to add your synth to the host plugin)
    CREATE_FUNCTION createSynth(float sampleRate, const char* resources) {
        ::stk::Stk::setSampleRate(sampleRate);
        
        //==========================================================================
        // CONTROLS - Use this array to completely specify your UI
        // - tells the system what parameters you want, and how they are controlled
        // - add or remove parameters by adding or removing entries from the list
        // - each control should have an expressive label / caption
        // - controls can be of different types: ROTARY, BUTTON, TOGGLE, SLIDER, or MENU (see definitions)
        // - for rotary and linear sliders, you can set the range of values (make sure the initial value is inside the range)
        // - for menus, replace the three numeric values with a single array of option strings: e.g. { "one", "two", "three" }
        // - by default, the controls are laid out in a grid, but you can also move and size them manually
        //   i.e. replace AUTO_SIZE with { 50,50,100,100 } to place a 100x100 control at (50,50)
        
        const Parameters CONTROLS = {
//            {   "PITCH\nBEND",Parameter::WHEEL,0.0, 16384, 8192, AUTO_SIZE  },
//            {   "MOD\nWHEEL",Parameter::WHEEL, 0.0, 127.0, 0.0,  AUTO_SIZE  },

            //  name,       type,              min, max, initial, size
            {   "Left",  Parameter::METER, 0.0, 1.0, 0.0, AUTO_SIZE  }, //0
            {   "Right",  Parameter::METER, 0.0, 1.0, 0.0, AUTO_SIZE  }, //1
            {   "Volume ENV type",  Parameter::MENU, {"ADSR", "ASR", "AR"}, AUTO_SIZE  },//2
            {   "LFO Mod Target",  Parameter::MENU, {"Sin", "Saw", "FM Mod", "Delay Signal"}, AUTO_SIZE  },//3
            {   "osc2 Repeat",  Parameter::TOGGLE, 0.0, 1.0, 0.0, AUTO_SIZE  },//4
            {   "LFO Mod Rate",  Parameter::ROTARY, 0.1, 2.0, 0, AUTO_SIZE  },//5
            {   "LFO Mod Depth",  Parameter::ROTARY, 0, 1.0, 0, AUTO_SIZE  },//6
            {   "osc2 Level",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },//7
            {   "FM Mod Freq",  Parameter::SLIDER, 0.01, 1.0, 0.01, AUTO_SIZE  },//8
            {   "FM Mod Index",  Parameter::SLIDER, 0, 4.0, 0, AUTO_SIZE  },//9
            {   "Delay Amount",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },//10
            {   "Delay Time",  Parameter::ROTARY, 0.1, 0.8, 0.0, AUTO_SIZE  },//11
            {   "osc2 Filter Hz",  Parameter::ROTARY, 20, 20000, 0.0, AUTO_SIZE  },//12
            {   "osc2 Attack",  Parameter::ROTARY, 0.0, 5.0, 0.0, AUTO_SIZE  },//13
            {   "osc2 Release",  Parameter::ROTARY, 0.0, 5.0, 0.0, AUTO_SIZE  },//14
            {   "Attack s",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  }, //15
            {   "Decay s",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  }, //16
            {   "Sustain",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },//17
            {   "Release s",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },//18
            {   "osc1 Level",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },//19
            
            
        };
        const Presets PRESETS = {
            { "Preset 1", { 0, 0, 0, 2, 0, 0.100, 0.000, 0.000, 0.010, 1.119, 0.302, 0.800, 7226.376, 0.613, 0.582, 0.364, 0.314, 0.362, 0.340, 0.575 } },
            { "Preset 2", { 0, 0, 0, 0, 0, 0.685, 0.685, 0.840, 0.661, 1.119, 0.302, 0.800, 7226.376, 0.613, 0.582, 0.364, 0.314, 0.362, 0.340, 0.575 } },
            { "Copied Preset", { 0, 0, 0, 0, 0, 0.000, 0.378, 0.520, 0.504, 0.557, 0.302, 0.322, 7226.376, 1.641, 1.361, 0.364, 0.314, 0.362, 0.340, 0.575 } },
        };
        
        
        
        APDI::Synth* pSynth = new MySynth(CONTROLS, PRESETS, resources);
        pSynth->setSampleRate(sampleRate);

        return pSynth;
    }
}

// Constructor: called when the synth plugin is first created / loaded
MySynth::MySynth(const Parameters& parameters, const Presets& presets, const char* resources)
: Synth(parameters, presets, resources)
{
    iBufferSize = 2 * getSampleRate();
        
    pfCircularBuffer = new float[iBufferSize]; //
    for(int x = 0; x < iBufferSize; x++) // this is for the delay
        pfCircularBuffer[x] = 0; //
    
    iBufferWritePos = 0;
    
    for(int n=0; n<32; n++) // create synthesiser's notes
        notes[n] = new MyNote(this);
    
    tremoloRate = parameters[5];  // LFO modulation
    tremoloDepth = parameters[6];  //
    
    Modulator.reset(); //LFO
    Modulator.setFrequency(tremoloRate);
    
    fPhasePos = 0;

    fSR = getSampleRate();

    iMeasuredLength = iMeasuredItems = 0;

    fMax0 = fMax1 = fMax0old = fMax1old = 0;
    
    // Initialise synthesier variables, etc.
}

// Destructor: called when the synthesiser is terminated / unloaded
MySynth::~MySynth()
{
    // Put your own additional clean up code here (e.g. free memory)
    
    for(int n=0; n<32; n++) // delete synthesiser's notes
        delete notes[n];
}

// EVENT HANDLERS: handle different user input (button presses, preset selection, drop menus)

void MySynth::presetLoaded(int iPresetNum, const char *sPresetName)
{
    // A preset has been loaded, so you could perform setup, such as retrieving parameter values
    // using getParameter and use them to set state variables in the plugin
}

void MySynth::optionChanged(int iOptionMenu, int iItem)
{
    // An option menu, with index iOptionMenu, has been changed to the entry, iItem
    if (iOptionMenu == 3) { // mod target set to delay time
        if (iItem == 3) {
            flange  = true;
        }
        else {
            flange = false;
        }
    }
    if (iOptionMenu == 10) { // this is to make sure we have the right value
        fDelayTime = iItem;
    }
    
}

void MySynth::buttonPressed(int iButton)
{
    // A button, with index iButton, has been pressed
}

// Applies audio processing to a buffer of audio
// (inputBuffer contains the input audio, and processed samples should be stored in outputBuffer)
void MySynth::postProcess(const float** inputBuffers, float** outputBuffers, int numSamples)
{
    float fIn0, fIn1, fOut0 = 0, fOut1 = 0;
    const float *pfInBuffer0 = inputBuffers[0], *pfInBuffer1 = inputBuffers[1];
    float *pfOutBuffer0 = outputBuffers[0], *pfOutBuffer1 = outputBuffers[1];
    
    float fMix;
    float fSR = getSampleRate();
    int iBufferReadPos;
    float fDelaySignal;
    float fDelayAmount;
    float fOut;
    
    fDelayTime = (parameters[11] * 0.5);
    
    tremoloRate = parameters[5];
    tremoloDepth = parameters[6];
    
    lfo2 = Modulator.tick() * tremoloDepth + 0.5;
    
    iMeasuredLength = (int)(0.02 * getSampleRate());

    float fAval0, fAval1;
    
    
//    float fGain = parameters[0];
    
    while(numSamples--)
    {
        // Get sample from input
        fIn0 = *pfInBuffer0++;
        fIn1 = *pfInBuffer1++;
        
        // Add your effect processing here
        
        //code for the delay
        fMix = (fIn0 + fIn1) * 0.5;
        
        
        
        iBufferReadPos = iBufferWritePos - (fSR * fDelayTime);
        
        if (iBufferReadPos < 0 ){
                    
            iBufferReadPos  += iBufferSize;
                    
        }
        
        fDelaySignal = pfCircularBuffer[iBufferReadPos];
        
        fDelayAmount = parameters[10];
                
        fDelaySignal *= fDelayAmount;
        
        if (flange) {
            fDelaySignal *= lfo2; // sets the LFO to modulate the Delay signal
        }
        
        
        fOut = fMix + fDelaySignal;
        
        pfCircularBuffer[iBufferWritePos] = fOut;
                
        iBufferWritePos++;
        
        if (iBufferWritePos == iBufferSize - 1){
                    
            iBufferWritePos = 0;
                    
        }
        
        // this is the code for metering
        fAval0 = fabs(fOut);
        fAval1 = fabs(fOut);
                
        if (fAval0 > fMax0) {

            fMax0 = fAval0;

        }

        if (fAval1 > fMax1) {

            fMax1 = fAval1;

        }

                
        iMeasuredItems ++;

        if (iMeasuredItems == iMeasuredLength) {

            fMax0 = log10((fMax0 * 40) + 1);
            fMax1 = log10((fMax1 * 40) + 1);
            fMax0 = fMax0 / log10(40);
            fMax1 = fMax1 / log10(40);

            if (fMax0 < fMax0old) {

                parameters[0] = (fMax0 * 0.1) + (fMax0old * 0.9);

            }

            else {

                parameters[0] = fMax0;

            }

            if (fMax1 < fMax1old) {

                parameters[1] = (fMax1 * 0.1) + (fMax1old * 0.9);

            }

            else {

                parameters[1] = fMax1;

            }


            fMax0old = fMax0;
            fMax1old = fMax1;
            fMax0 = fMax1 = iMeasuredItems = 0;

        }
        
        fOut0 = fOut;
        fOut1 = fOut;
        
        
        // Copy result to output
        *pfOutBuffer0++ = fOut0;
        *pfOutBuffer1++ = fOut1;
    }
}
