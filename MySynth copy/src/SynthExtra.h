//
//  SynthExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#define MAX_HARMONICS 16

class SawWave
{
public:
    void reset(){
        for(int h=0; h<MAX_HARMONICS; h++)
            harmonics[h].reset();
    }
    
    void setFrequency(float f){
        for(int h=0; h<MAX_HARMONICS; h++){
            const int n = h + 1;
            harmonics[h].setFrequency(f * n);
        }
    }
    
    float tick(){
        float fMix = 0;
        for(int h=0; h<MAX_HARMONICS; h++){
            const int n = h + 1;
            fMix += harmonics[h].tick() * 1.f / n;
        }
        return fMix;
    }
protected:
    Sine harmonics[MAX_HARMONICS];
};

class SquareWave : public SawWave
{
public:
    
    
    float tick(){
        float fMix = 0;
        for(int h=0; h<MAX_HARMONICS; h++){
            const int n = h + 1;
            if (n % 2 != 0) {
                fMix += harmonics[h].tick() * 1.f / n;
            }
        }
        return fMix;
        
    }

};
