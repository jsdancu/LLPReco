#ifndef LLPReco_DataFormats_JetFeatures_h
#define LLPReco_DataFormats_JetFeatures_h

namespace llpdnnx {

struct JetFeatures 
{

    int jetIdx;
    float pt;
    float eta;
    float phi;
    float mass;
    float energy;
    
    float area;
    
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
    
    JetFeatures():
        jetIdx(-1),
        pt(-1),
        eta(0),
        phi(0),
        mass(0),
        energy(0),
        
        area(-1),
        
        n60(-1),
        n90(-1),
        
        chargedEmEnergyFraction(-1),
        chargedHadronEnergyFraction(-1),
        chargedMuEnergyFraction(-1),
        electronEnergyFraction(-1),

        tau1(-1),
        tau2(-1),
        tau3(-1),
        
        relMassDropMassAK(0),
        relMassDropMassCA(0),
        relSoftDropMassAK(0),
        relSoftDropMassCA(0),
        
        thrust(0), //cannot be smaller than 0.5 by definition
        sphericity(-1),
        circularity(-1),
        isotropy(-1),
        eventShapeC(-1),
        eventShapeD(-1)  
    {
    }
    
};

}

#endif 
