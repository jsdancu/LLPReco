#ifndef LLPReco_DataFormats_JetFeatures_h
#define LLPReco_DataFormats_JetFeatures_h

namespace llpdnnx {

struct JetFeatures 
{

    int jetIdx;
    float pt;
    float eta;
    float mass;
    float energy;
    
    int n60;
    int n90;
    
    float chargedEmEnergyFraction;
    float chargedHadronEnergyFraction;
    float chargedMuEnergyFraction;
    float electronEnergyFraction;

    float tau1;
    float tau2;
    float tau3;
    
    float relMassDropMassAK;
    float relMassDropMassCA;
    float relSoftDropMassAK;
    float relSoftDropMassCA;
    
    float thrust;
    float sphericity;
    float circularity;
    float isotropy;
    float eventShapeC;
    float eventShapeD;
};

}

#endif 
