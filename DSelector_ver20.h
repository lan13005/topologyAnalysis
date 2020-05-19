#ifndef DSelector_ver20_h
#define DSelector_ver20_h

#include <iostream>
#include <fstream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"

// change thrownID and string_ph in place 
Int_t getParents( Int_t* thrownID, vector<Int_t> parentIDs, vector<Int_t> thrownPIDs, TString* string_ph ){
	Int_t initialThrownID = *thrownID;
	*thrownID  = parentIDs[*thrownID];
	Int_t thrownPID = thrownPIDs[*thrownID];
	if (*thrownID != -1){
		Int_t parentPID = thrownPIDs[*thrownID];
		//cout << "ThrownID: " << initialThrownID << " with PID=" << thrownPID << " has *thrownID: " << *thrownID << " with PID=" << parentPID << endl;
		*string_ph += "("+to_string(parentPID)+")";
		return *thrownID;
	}
	else {
		//cout << "Found primary particle" << endl;
		//*string_ph += "(-1)";
		return *thrownID;
	}
}

struct topology {
	TString locThrownTopology;
	TString composition;
};

class DSelector_ver20 : public DSelector
{
	public:

		DSelector_ver20(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_ver20(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Get_ComboWrappers(void);
		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

		// ANALYZE CUT ACTIONS
		// // Automatically makes mass histograms where one cut is missing
		DHistogramAction_AnalyzeCutActions* dAnalyzeCutActions;

		//CREATE REACTION-SPECIFIC PARTICLE ARRAYS

		//Step 0
		DParticleComboStep* dStep0Wrapper;
		DBeamParticle* dComboBeamWrapper;
		DChargedTrackHypothesis* dProtonWrapper;

		//Step 1
		DParticleComboStep* dStep1Wrapper;
		DNeutralParticleHypothesis* dPhoton1Wrapper;
		DNeutralParticleHypothesis* dPhoton2Wrapper;

		//Step 2
		DParticleComboStep* dStep2Wrapper;
		DNeutralParticleHypothesis* dPhoton3Wrapper;
		DNeutralParticleHypothesis* dPhoton4Wrapper;

		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:
		int iterToRun=0;
		TH1I* dHist_MissingMassSquared;
		TH1I* dHist_BeamEnergy;
		TH1F* dHist_pi0Mass[10];
		TH1F* dHist_etaMass[10];
		TH1F* dHist_pi0Mass_pi00;
		TH1F* dHist_pi0etaMass;
		TH1F* dHist_countTopology;
		TH1F* dHist_countTopologyPi0;
		TH1F* dHist_countTopologyEta;

		TH1F* dHist_numUniquePairsPerPh1234Set;
		TH1F* dHist_numUniquePh1234Sets;
		double eventNum=0;
        	double pi0Mass_true = 0.135784; 
		double etaMass_true = 0.548036; 
		double pi0MassStd_true = 0.00753584;
		double etaMassStd_true = 0.0170809;
		TH2F* dHist_eventVsChiSq1234;
		TH2F* dHist_ChiSq1234VsChiSqCombo;

		ofstream compositionFile;

	ClassDef(DSelector_ver20, 0);
};

void DSelector_ver20::Get_ComboWrappers(void)
{
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(2));

	//Step 1
	dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	dPhoton1Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(0));
	dPhoton2Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(1));

	//Step 2
	dStep2Wrapper = dComboWrapper->Get_ParticleComboStep(2);
	dPhoton3Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep2Wrapper->Get_FinalParticle(0));
	dPhoton4Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep2Wrapper->Get_FinalParticle(1));
}

#endif // DSelector_ver20_h
