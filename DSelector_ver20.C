#include "DSelector_ver20.h"

void DSelector_ver20::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "ver20.root"; //"" for none
	dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = ""; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = ""; //if blank, default name will be chosen

	//Because this function gets called for each TTree in the TChain, we must be careful:
		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms
	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously
	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree
	//gDirectory now points to the output file with name dOutputFileName (if any)
	if(locInitializedPriorFlag)
		return; //have already created histograms, etc. below: exit

	Get_ComboWrappers();
	dPreviousRunNumber = 0;

	/*********************************** EXAMPLE USER INITIALIZATION: ANALYSIS ACTIONS **********************************/

	// EXAMPLE: Create deque for histogramming particle masses:
	// // For histogramming the phi mass in phi -> K+ K-
	// // Be sure to change this and dAnalyzeCutActions to match reaction
	std::deque<Particle_t> MyPhi;
	MyPhi.push_back(KPlus); MyPhi.push_back(KMinus);

	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data

	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));
	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems
	//dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KPlus, SYS_BCAL));

	//MASSES
	//dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, Lambda, 1000, 1.0, 1.2, "Lambda"));
	//dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1));

	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));

	//CUT MISSING MASS
	//dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.03, 0.02));

	//BEAM ENERGY
	dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));
	//dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 8.4, 9.05));

	//KINEMATICS
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false));

	// ANALYZE CUT ACTIONS
	// // Change MyPhi to match reaction
	dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "CutActionEffect" );

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();
	dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1I("MissingMassSquared", ";Missing Mass Squared (GeV/c^{2})^{2}", 600, -0.06, 0.06);
	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", 600, 0.0, 12.0);
	for (int i=0; i<10; ++i){
		dHist_pi0Mass[i] = new TH1F(("pi0Mass_chiSqBin"+to_string(i)).c_str(), ";Mpi0", 100, 0.0, 0.5);
		dHist_etaMass[i] = new TH1F(("etaMass_chiSqBin"+to_string(i)).c_str(), ";Meta", 100, 0.0, 1);
	}

	dHist_pi0Mass_pi00 = new TH1F("matched to thrown pion", ";Mpi0", 100, 0.0, 0.5);

	dHist_pi0etaMass = new TH1F("pi0etaMass", ";Mpi0eta", 150, 0.0, 3.5);
	dHist_countTopology = new TH1F("countTopology", "", 16, 0, 16);
	dHist_countTopologyPi0 = new TH1F("countTopologyPi0", "", 7, 0, 7);
	dHist_countTopologyEta = new TH1F("countTopologyEta", "", 7, 0, 7);

	dHist_numUniquePairsPerPh1234Set = new TH1F("numUniquePairsPerPh1234Set", "",5,0,5);
	dHist_numUniquePh1234Sets = new TH1F("numUniquePh1234Sets","",10,0,10);
	dHist_eventVsChiSq1234 = new TH2F("eventVsChiSq1234",";ChiSqs;Event Number",40,0,40,100,0,100);
	dHist_ChiSq1234VsChiSqCombo = new TH2F("chiSq1234VsChiSqCombo",";ChiSqs1234;ChiSqCombo",40,0,40,100,0,100);


	string xLabels[17] = {"pi0 00", "pi0 01", "pi0 02", "pi0 12", "pi0 11", "pi0 22", "pi0 Else", "", "", "", "eta 00", "eta 01", "eta 02", "eta 12", "eta 11", "eta 22", "eta Else"};


	for (int i=0; i<17; ++i){
		dHist_countTopology->GetXaxis()->SetBinLabel(dHist_countTopology->GetXaxis()->FindBin(i), xLabels[i].c_str());
	}
	string xLabelsPi0[7] = {"00", "01", "02", "12", "11", "22", "pi0 Else"};
	string xLabelsEta[7] = {"00", "01", "02", "12", "11", "22", "eta Else"};
	for (int i=0; i<7; ++i){
		dHist_countTopologyPi0->GetXaxis()->SetBinLabel(dHist_countTopologyPi0->GetXaxis()->FindBin(i), xLabelsPi0[i].c_str());
		dHist_countTopologyEta->GetXaxis()->SetBinLabel(dHist_countTopologyEta->GetXaxis()->FindBin(i), xLabelsEta[i].c_str());
	}

	for (int i=0; i<200; ++i){ // there is a maximum of 200 in this case. No specific reason. we defnitiely want it larger than nthreads though
		string filename="/d/grid15/ln16/pi0eta/testing/pi0eta/logFile/composition_"+to_string(i)+".txt";
		ifstream ifile(filename.c_str());
		if (!ifile) { //if the file does not exist then we will open it and make our output file link to it and break the for loop to continue with the program
			compositionFile.open(filename.c_str(),std::ios_base::trunc);
			break;
		}
	}



	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - MAIN TREE *************************/

	//EXAMPLE MAIN TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):
	//The type for the branch must be included in the brackets
	//1st function argument is the name of the branch
	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)
	/*
	dTreeInterface->Create_Branch_Fundamental<Int_t>("my_int"); //fundamental = char, int, float, double, etc.
	dTreeInterface->Create_Branch_FundamentalArray<Int_t>("my_int_array", "my_int");
	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("my_combo_array", "NumCombos");
	dTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("my_p4");
	dTreeInterface->Create_Branch_ClonesArray<TLorentzVector>("my_p4_array");
	*/

	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - FLAT TREE *************************/

	//EXAMPLE FLAT TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):
	//The type for the branch must be included in the brackets
	//1st function argument is the name of the branch
	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)
	/*
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("flat_my_int"); //fundamental = char, int, float, double, etc.
	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>("flat_my_int_array", "flat_my_int");
	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("flat_my_p4");
	dFlatTreeInterface->Create_Branch_ClonesArray<TLorentzVector>("flat_my_p4_array");
	*/

	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/

	//TO SAVE PROCESSING TIME
		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up
		//By default, for each event, the data is retrieved for all branches
		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need
		//Do this by doing something similar to the commented code below

	//dTreeInterface->Clear_GetEntryBranches(); //now get none
	//dTreeInterface->Register_GetEntryBranch("Proton__P4"); //manually set the branches you want
}

Bool_t DSelector_ver20::Process(Long64_t locEntry)
{
	++eventNum;
	if (eventNum>100){
		Abort("Lawrence... your max number of events is reached...");
	}
	//++iterToRun;
	//if ( iterToRun > 1000 ) { return kTRUE; }
	// The Process() function is called for each entry in the tree. The entry argument
	// specifies which entry in the currently loaded tree is to be processed.
	//
	// This function should contain the "body" of the analysis. It can contain
	// simple or elaborate selection criteria, run algorithms on the data
	// of the event and typically fill histograms.
	//
	// The processing can be stopped by calling Abort().
	// Use fStatus to set the return value of TTree::Process().
	// The return value is currently not used.

	//CALL THIS FIRST
	DSelector::Process(locEntry); //Gets the data from the tree for the entry
	//cout << "RUN " << Get_RunNumber() << ", EVENT " << Get_EventNumber() << endl;
	//TLorentzVector locProductionX4 = Get_X4_Production();

	/******************************************** GET POLARIZATION ORIENTATION ******************************************/

	//Only if the run number changes
	//RCDB environment must be setup in order for this to work! (Will return false otherwise)
	UInt_t locRunNumber = Get_RunNumber();
	if(locRunNumber != dPreviousRunNumber)
	{
		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);
		dPreviousRunNumber = locRunNumber;
	}

	/********************************************* SETUP UNIQUENESS TRACKING ********************************************/

	//ANALYSIS ACTIONS: Reset uniqueness tracking for each action
	//For any actions that you are executing manually, be sure to call Reset_NewEvent() on them here
	Reset_Actions_NewEvent();
	dAnalyzeCutActions->Reset_NewEvent(); // manual action, must call Reset_NewEvent()

	//PREVENT-DOUBLE COUNTING WHEN HISTOGRAMMING
		//Sometimes, some content is the exact same between one combo and the next
			//e.g. maybe two combos have different beam particles, but the same data for the final-state
		//When histogramming, you don't want to double-count when this happens: artificially inflates your signal (or background)
		//So, for each quantity you histogram, keep track of what particles you used (for a given combo)
		//Then for each combo, just compare to what you used before, and make sure it's unique

	//EXAMPLE 1: Particle-specific info:
	set<Int_t> locUsedSoFar_BeamEnergy; //Int_t: Unique ID for beam particles. set: easy to use, fast to search

	//EXAMPLE 2: Combo-specific info:
		//In general: Could have multiple particles with the same PID: Use a set of Int_t's
		//In general: Multiple PIDs, so multiple sets: Contain within a map
		//Multiple combos: Contain maps within a set (easier, faster to search)
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_MissingMass;
	set<pair< map<Particle_t, set<Int_t> >, map<Particle_t, set<Int_t> > > > locUsedSoFar_pi0_eta;
	set< map<Particle_t, set<Int_t> > > locUsedSoFar_ph1234;

	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE

	/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

	/*
	Int_t locMyInt = 7;
	dTreeInterface->Fill_Fundamental<Int_t>("my_int", locMyInt);

	TLorentzVector locMyP4(4.0, 3.0, 2.0, 1.0);
	dTreeInterface->Fill_TObject<TLorentzVector>("my_p4", locMyP4);

	for(int loc_i = 0; loc_i < locMyInt; ++loc_i)
		dTreeInterface->Fill_Fundamental<Int_t>("my_int_array", 3*loc_i, loc_i); //2nd argument = value, 3rd = array index
	*/

	/************************************************* LOOP OVER COMBOS *************************************************/

	//Loop over combos
	
	int countPairs = 0;
	int countPh1234Sets = 0;
	map<TString, vector<topology>> topologyMap;

	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)
	{
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);

		// Is used to indicate when combos have been cut
		if(dComboWrapper->Get_IsComboCut()) // Is false when tree originally created
			continue; // Combo has been cut previously

		/********************************************** GET PARTICLE INDICES *********************************************/

		//Used for tracking uniqueness when filling histograms, and for determining unused particles

		//Step 0
		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		//Step 1
		Int_t locPhoton1NeutralID = dPhoton1Wrapper->Get_NeutralID();
		Int_t locPhoton2NeutralID = dPhoton2Wrapper->Get_NeutralID();

		//Step 2
		Int_t locPhoton3NeutralID = dPhoton3Wrapper->Get_NeutralID();
		Int_t locPhoton4NeutralID = dPhoton4Wrapper->Get_NeutralID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();
		//Step 1
		TLorentzVector locPhoton1P4 = dPhoton1Wrapper->Get_P4();
		TLorentzVector locPhoton2P4 = dPhoton2Wrapper->Get_P4();
		//Step 2
		TLorentzVector locPhoton3P4 = dPhoton3Wrapper->Get_P4();
		TLorentzVector locPhoton4P4 = dPhoton4Wrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPhoton1P4_Measured = dPhoton1Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton2P4_Measured = dPhoton2Wrapper->Get_P4_Measured();
		//Step 2
		TLorentzVector locPhoton3P4_Measured = dPhoton3Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton4P4_Measured = dPhoton4Wrapper->Get_P4_Measured();

		double pi0Mass = (locPhoton1P4+locPhoton2P4).M();
		double etaMass = (locPhoton3P4+locPhoton4P4).M();
		double pi0etaMass = (locPhoton1P4+locPhoton2P4+locPhoton3P4+locPhoton4P4).M();
        	double locChiSqKinFit = dComboWrapper->Get_ChiSq_KinFit("");

		int locNumThrown = Get_NumThrown();
		vector<Int_t> thrownPIDs;
		vector<TLorentzVector> thrown_p4s;
		vector<Int_t> parentIDs;
		if (Get_NumThrown() != 0 ) {
			for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)
			{	
				dThrownWrapper->Set_ArrayIndex( loc_i  );
				//cout << "thrown PID: " << dThrownWrapper->Get_PID() << " with parent thrown index: " <<  dThrownWrapper->Get_ParentIndex() << endl;
				thrownPIDs.push_back(dThrownWrapper->Get_PID());
				thrown_p4s.push_back(dThrownWrapper->Get_P4());
				parentIDs.push_back(dThrownWrapper->Get_ParentIndex());
			}
		}
		TString locThrownTopology = Get_ThrownTopologyString();
		cout << "topology: " << locThrownTopology << endl;

		Int_t locParentPID;
		Int_t parentID;


		vector<Int_t> thrownID_phs = {dPhoton1Wrapper->Get_ThrownIndex(), dPhoton2Wrapper->Get_ThrownIndex(), dPhoton3Wrapper->Get_ThrownIndex(), dPhoton4Wrapper->Get_ThrownIndex()};
		vector<TString> parents_phs = {"","","",""};;
		TString composition = "";

		for (size_t iph=0; iph < thrownID_phs.size(); ++iph){
			if (thrownID_phs[iph] != -1){ // if -1 then  matched to a thrown particle
				dThrownWrapper->Set_ArrayIndex( thrownID_phs[iph]  );
				Particle_t locPID = dThrownWrapper->Get_PID();
				while ( getParents(&thrownID_phs[iph],parentIDs,thrownPIDs,&parents_phs[iph]) != -1 );
			}
			else{ 
				parents_phs[iph] += "(0)"; 
			}
			//cout << parents_phs[iph] << endl;
			if ( iph == 0 ){
				composition += parents_phs[iph];
			}
			else { composition += "_"+parents_phs[iph]; }
		}

		//compositionFile << "none " << locThrownTopology << " " << composition << endl;
		topology someTopology;
		someTopology.locThrownTopology = locThrownTopology;
		someTopology.composition = composition;
		topologyMap["none"].push_back(someTopology);
		if ( locChiSqKinFit < 13.277 ){
			//compositionFile << "locChiSqKinFit " << locThrownTopology << " " << composition << endl;
			someTopology = topology();
			someTopology.locThrownTopology = locThrownTopology;
			someTopology.composition = composition;
			topologyMap["locChiSqKinFit"].push_back(someTopology);
		}
        	double locUnusedEnergy = dComboWrapper->Get_Energy_UnusedShowers();
		if ( locUnusedEnergy < 0.010 ) { 
			//compositionFile << "locUnusedEnergy " << locThrownTopology << " " << composition << endl;
			someTopology = topology();
			someTopology.locThrownTopology = locThrownTopology;
			someTopology.composition = composition;
			topologyMap["locUnusedEnergy"].push_back(someTopology);
		}
                double showerQuality1 = dPhoton1Wrapper->Get_Shower_Quality();
                double showerQuality2 = dPhoton2Wrapper->Get_Shower_Quality();
                double showerQuality3 = dPhoton3Wrapper->Get_Shower_Quality();
                double showerQuality4 = dPhoton4Wrapper->Get_Shower_Quality();
		if ( (showerQuality1 > 0.5)*(showerQuality2 > 0.5)*(showerQuality3 > 0.5)*(showerQuality4 > 0.5) ){
			//compositionFile << "showerQuality " << locThrownTopology << " " << composition << endl;
			someTopology = topology();
			someTopology.locThrownTopology = locThrownTopology;
			someTopology.composition = composition;
			topologyMap["showerQuality"].push_back(someTopology);
		}





// ==========================================================================================================
// This section is for outputting the consituents of the eta and pi
// ==========================================================================================================
 		//cout << "--------------- Next combo ----------" << endl;
		//Int_t thrownID_ph1;
		//Particle_t locPID;
		//Int_t locParentIndex;
		//thrownID_ph1 = dPhoton1Wrapper->Get_ThrownIndex();
		//Int_t locParentPID;

		//// The order of the bool array is:
		//// { from omega, is a primary photon, not thrown / fake }
		//string isPhoton1 = "";
		//string isPhoton2 = "";
		//string isPhoton3 = "";
		//string isPhoton4 = "";


		//cout << "PHOTON1" << endl;
		//cout << "--- Reconstructed P4 ---" << endl;
		//locPhoton1P4.Print();
		//if ( thrownID_ph1 != -1 ) { // if its matched to a thrown particle
		//	dThrownWrapper->Set_ArrayIndex( thrownID_ph1  );
		//	Particle_t locPID = dThrownWrapper->Get_PID();
		//	locParentIndex = dThrownWrapper->Get_ParentIndex();
		//	if (thrownPIDs[locParentIndex] == 7 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon1 matched with thrown particle  PID " << locPID << " with a pi0 parent" << endl;
		//		isPhoton1 = "0";
		//	}
		//	else if (thrownPIDs[locParentIndex] == 17 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon1 matched with thrown particle  PID " << locPID << " with a eta parent" << endl;
		//		isPhoton1 = "1";
		//	}
		//	else {
		//		cout << "Photon1 has no parent, parentID: " << thrownPIDs[locParentIndex] << endl;
		//	}
		//	cout << "--- Matched Thrown P4 ---" << endl;
		//	thrown_p4s[thrownID_ph1].Print();
		//}
		//else {
		//	cout << "Photon1 matched with no thrown particle..." << endl;
		//	isPhoton1 = "2";
		//}
	
		//Int_t thrownID_ph2 = dPhoton2Wrapper->Get_ThrownIndex();
		//cout << "PHOTON2" << endl;
		//cout << "--- Reconstructed P4 ---" << endl;
		//locPhoton2P4.Print();
		//if ( thrownID_ph2 != -1 ) {
		//	dThrownWrapper->Set_ArrayIndex( thrownID_ph2  );
		//	locPID = dThrownWrapper->Get_PID();
		//	locParentIndex = dThrownWrapper->Get_ParentIndex();
		//	if (thrownPIDs[locParentIndex] == 7 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon2 matched with thrown particle  PID " << locPID << " with a pi0 parent" << endl;
		//		isPhoton2 = "0";
		//	}
		//	else if (thrownPIDs[locParentIndex] == 17 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon2 matched with thrown particle  PID " << locPID << " with a eta parent" << endl;
		//		isPhoton2 = "1";
		//	}
		//	else {
		//		cout << "Photon2 has no parent, parentID: " << thrownPIDs[locParentIndex] << endl;
		//	}
		//	dThrownWrapper->Set_ArrayIndex( thrownID_ph2 );
		//	cout << "--- Matched Thrown P4 ---" << endl;
		//	thrown_p4s[thrownID_ph2].Print();
		//}
		//else {
		//	cout << "Photon2 matched with no thrown particle..." << endl;
		//	isPhoton2 = "2";
		//}

		//Int_t thrownID_ph3 = dPhoton3Wrapper->Get_ThrownIndex();
		//cout << "PHOTON3" << endl;
		//cout << "--- Reconstructed P4 ---" << endl;
		//locPhoton3P4.Print();
		//if ( thrownID_ph3 != -1 ) {
		//	dThrownWrapper->Set_ArrayIndex( thrownID_ph3  );
		//	locPID = dThrownWrapper->Get_PID();
		//	locParentIndex = dThrownWrapper->Get_ParentIndex();
		//	if (thrownPIDs[locParentIndex] == 7 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon3 matched with thrown particle  PID " << locPID << " with a pi0 parent" << endl;
		//		isPhoton3 = "0";
		//	}
		//	else if (thrownPIDs[locParentIndex] == 17 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon3 matched with thrown particle  PID " << locPID << " with a eta parent" << endl;
		//		isPhoton3 = "1";
		//	}
		//	else {
		//		cout << "Photon3 has no parent, parentID: " << thrownPIDs[locParentIndex] << endl;
		//	}
		//	cout << "--- Matched Thrown P4 ---" << endl;
		//	thrown_p4s[thrownID_ph3].Print();
		//}
		//else {
		//	cout << "Photon3 matched with no thrown particle..." << endl;
		//	isPhoton3 = "2";
		//}

		//Int_t thrownID_ph4 = dPhoton4Wrapper->Get_ThrownIndex();
		//cout << "PHOTON4" << endl;
		//cout << "--- Reconstructed P4 ---" << endl;
		//locPhoton4P4.Print();
		//if ( thrownID_ph4 != -1 ) {
		//	dThrownWrapper->Set_ArrayIndex( thrownID_ph4  );
		//	locPID = dThrownWrapper->Get_PID();
		//	locParentIndex = dThrownWrapper->Get_ParentIndex();
		//	if (thrownPIDs[locParentIndex] == 7 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon4 matched with thrown particle  PID " << locPID << " with a pi0 parent" << endl;
		//		isPhoton4 = "0";
		//	}
		//	else if (thrownPIDs[locParentIndex] == 17 ){ // if the thrown particle's parent is the pi0
		//		cout << "Photon4 matched with thrown particle  PID " << locPID << " with a eta parent" << endl;
		//		isPhoton4 = "1";
		//	}
		//	else {
		//		cout << "Photon4 has no parent, parentID: " << thrownPIDs[locParentIndex] << endl;
		//	}
		//	cout << "--- Matched Thrown P4 ---" << endl;
		//	thrown_p4s[thrownID_ph4].Print();
		//}
		//else {
		//	cout << "Photon4 matched with no thrown particle..." << endl;
		//	isPhoton4 = "2";
		//}


		//string isPhotons = isPhoton1+isPhoton2+isPhoton3+isPhoton4;
		//string isPi0 = isPhoton1+isPhoton2;
		//string isEta = isPhoton3+isPhoton4;
		//cout << "Topology: " << isPhotons << endl;
		//// photon 1 and 2 make up the pi0

		//bool applyCuts = locChiSqKinFit < 13.277;
		//if (applyCuts){ 
		//	if ( isPi0 == "00" ) {
		//		dHist_countTopologyPi0->Fill(0);
		//		dHist_countTopology->Fill(0);
		//		dHist_pi0Mass_pi00->Fill(pi0Mass);
		//	}
		//	// reconstructed pion made with real gamma and one gamma from the real pion
		//	else if ( isPi0 == "01" || isPi0 == "10" ) {
		//		dHist_countTopologyPi0->Fill(1);
		//		dHist_countTopology->Fill(1);
		//	}
		//	// reconstructed pion made with fake gamma and one gamma from the real pion
		//	else if ( isPi0 == "02" || isPi0 == "20" ) {
		//		dHist_countTopologyPi0->Fill(2);
		//		dHist_countTopology->Fill(2);
		//	}
		//	// reconstructed pion made with fake gamma and a real gamma
		//	else if ( isPi0 == "12" || isPi0 == "21" ) {
		//		dHist_countTopologyPi0->Fill(3);
		//		dHist_countTopology->Fill(3);
		//	}
		//	else if ( isPi0 == "11" ){
		//		dHist_countTopologyPi0->Fill(4);
		//		dHist_countTopology->Fill(4);
		//	}
		//	// reconstructed pion made with two fake gammas
		//	else if ( isPi0 == "22" ) {
		//		dHist_countTopologyPi0->Fill(5);
		//		dHist_countTopology->Fill(5);
		//	}
		//	else{
		//		dHist_countTopologyPi0->Fill(6);
		//		dHist_countTopology->Fill(6);
		//		cout << "unaccounted for pi0 topology: " << isPi0 << endl;
		//	}
		//	// photon 1 and 2 make up the pi0
		//	if ( isEta == "00" ) {
		//		dHist_countTopologyEta->Fill(0);
		//		cout << "Eta is made with the two photons from the pion..." << endl;
		//		dHist_countTopology->Fill(10);
		//		cout << "Mass of thrown ph3+ph4 : " << (thrown_p4s[thrownID_ph3]+thrown_p4s[thrownID_ph4]).M() << endl;
		//	}
		//	// reconstructed pion made with real gamma and one gamma from the real pion
		//	else if ( isEta == "01" || isEta == "10" ) {
		//		dHist_countTopologyEta->Fill(1);
		//		dHist_countTopology->Fill(11);
		//	}
		//	// reconstructed pion made with fake gamma and one gamma from the real pion
		//	else if ( isEta == "02" || isEta == "20" ) {
		//		dHist_countTopologyEta->Fill(2);
		//		dHist_countTopology->Fill(12);
		//	}
		//	// reconstructed pion made with fake gamma and a real gamma
		//	else if ( isEta == "12" || isEta == "21" ) {
		//		dHist_countTopologyEta->Fill(3);
		//		dHist_countTopology->Fill(13);
		//	}
		//	else if ( isEta == "11" ){
		//		dHist_countTopologyEta->Fill(4);
		//		dHist_countTopology->Fill(14);
		//	}
		//	// reconstructed pion made with two fake gammas
		//	else if ( isEta == "22" ) {
		//		dHist_countTopologyEta->Fill(5);
		//		dHist_countTopology->Fill(15);
		//	}
		//	else{
		//		dHist_countTopologyEta->Fill(6);
		//		dHist_countTopology->Fill(16);
		//		cout << "unaccounted for eta topology: " << isEta << endl;
		//	}
		//}




		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locProtonP4_Measured + locPhoton1P4_Measured + locPhoton2P4_Measured + locPhoton3P4_Measured + locPhoton4P4_Measured;

		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

		//if you manually execute any actions, and it fails a cut, be sure to call:
			//dComboWrapper->Set_IsComboCut(true);

		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

		/*
		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);
		//for arrays below: 2nd argument is value, 3rd is array index
		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever they were from the last event)
			//So, when you draw the branch, be sure to cut on "IsComboCut" to avoid these.
		dTreeInterface->Fill_Fundamental<Float_t>("my_combo_array", -2*loc_i, loc_i);
		dTreeInterface->Fill_TObject<TLorentzVector>("my_p4_array", locMyComboP4, loc_i);
		*/

		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/

		//Histogram beam energy (if haven't already)
		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
		{
			dHist_BeamEnergy->Fill(locBeamP4.E());
			locUsedSoFar_BeamEnergy.insert(locBeamID);
		}

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

		//Missing Mass Squared
		double locMissingMassSquared = locMissingP4_Measured.M2();

		//Uniqueness tracking: Build the map of particles used for the missing mass
			//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;
		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);
		locUsedThisCombo_MissingMass[Gamma].insert(locPhoton1NeutralID);
		locUsedThisCombo_MissingMass[Gamma].insert(locPhoton2NeutralID);
		locUsedThisCombo_MissingMass[Gamma].insert(locPhoton3NeutralID);
		locUsedThisCombo_MissingMass[Gamma].insert(locPhoton4NeutralID);

		for (int i=0; i<10; ++i){
			if ( locChiSqKinFit < (i+1)*10 ) {
				dHist_pi0Mass[i]->Fill(pi0Mass);
				dHist_etaMass[i]->Fill(etaMass);
			}
		}
		dHist_pi0etaMass->Fill(pi0etaMass);

		//compare to what's been used so far
		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
			dHist_MissingMassSquared->Fill(locMissingMassSquared);
			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
		}

        	map<Particle_t, set<Int_t> > using1234;
        	using1234[Gamma].insert(locPhoton1NeutralID);
        	using1234[Gamma].insert(locPhoton2NeutralID);
        	using1234[Gamma].insert(locPhoton3NeutralID);
        	using1234[Gamma].insert(locPhoton4NeutralID);

        	map<Particle_t, set<Int_t> > using12;
        	using12[Gamma].insert(locPhoton1NeutralID);
        	using12[Gamma].insert(locPhoton2NeutralID);
        	map<Particle_t, set<Int_t> > using34;
        	using34[Gamma].insert(locPhoton3NeutralID);
        	using34[Gamma].insert(locPhoton4NeutralID);

		double pi0Term = (pi0Mass-pi0Mass_true)/pi0MassStd_true;
		double etaTerm = (etaMass-etaMass_true)/etaMassStd_true;	
		double chiSqPair = sqrt(pi0Term*pi0Term+etaTerm*etaTerm);

        	pair< map<Particle_t, set<Int_t> >, map<Particle_t, set<Int_t> > > using12_34 = make_pair(using12,using34);
		if(locUsedSoFar_pi0_eta.find(using12_34) == locUsedSoFar_pi0_eta.end()) {
			locUsedSoFar_pi0_eta.insert(using12_34);
			if (eventNum < 100) dHist_eventVsChiSq1234->Fill(chiSqPair,eventNum);
			dHist_ChiSq1234VsChiSqCombo->Fill(chiSqPair,locChiSqKinFit);
			++countPairs;
			if(locUsedSoFar_ph1234.find(using1234) == locUsedSoFar_ph1234.end()) 
			{
				locUsedSoFar_ph1234.insert(using1234);
				++countPh1234Sets;
			}
		}



		//E.g. Cut
		//if((locMissingMassSquared < -0.04) || (locMissingMassSquared > 0.04))
		//{
		//	dComboWrapper->Set_IsComboCut(true);
		//	continue;
		//}

		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/

		/*
		//FILL ANY CUSTOM BRANCHES FIRST!!
		Int_t locMyInt_Flat = 7;
		dFlatTreeInterface->Fill_Fundamental<Int_t>("flat_my_int", locMyInt_Flat);

		TLorentzVector locMyP4_Flat(4.0, 3.0, 2.0, 1.0);
		dFlatTreeInterface->Fill_TObject<TLorentzVector>("flat_my_p4", locMyP4_Flat);

		for(int loc_j = 0; loc_j < locMyInt_Flat; ++loc_j)
		{
			dFlatTreeInterface->Fill_Fundamental<Int_t>("flat_my_int_array", 3*loc_j, loc_j); //2nd argument = value, 3rd = array index
			TLorentzVector locMyComboP4_Flat(8.0, 7.0, 6.0, 5.0);
			dFlatTreeInterface->Fill_TObject<TLorentzVector>("flat_my_p4_array", locMyComboP4_Flat, loc_j);
		}
		*/

		//FILL FLAT TREE
		//Fill_FlatTree(); //for the active combo
	} // end of combo loop

	Int_t nCombosPassed;
	double comboWeight;
	for ( const auto &currentPair : topologyMaps ) {
		compositionFile << "Filling topologies with cut: " << currentPair.first << endl;
		nCombosPassed = currentPair.second.size();
		comboWeight = 1/nCombosPassed;
		compositionFile << "-Num combos passed cuts: " <<
		for ( auto currentTopology : currentPair.second ) {
			compositionFile << "--Current topology: " << currentTopology.locThrownTopology << endl;
			compositionFile << "--Current composition: " << currentTopology.composition << endl;
		}
	}	

	dHist_numUniquePairsPerPh1234Set->Fill(countPh1234Sets);
	dHist_numUniquePh1234Sets->Fill(countPairs);

	//FILL HISTOGRAMS: Num combos / events surviving actions
	Fill_NumCombosSurvivedHists();

	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/
/*
	//Thrown beam: just use directly
	if(dThrownBeam != NULL)
		double locEnergy = dThrownBeam->Get_P4().E();

	//Loop over throwns
	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dThrownWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}
*/
	/****************************************** LOOP OVER OTHER ARRAYS (OPTIONAL) ***************************************/
/*
	//Loop over beam particles (note, only those appearing in combos are present)
	for(UInt_t loc_i = 0; loc_i < Get_NumBeam(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dBeamWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}

	//Loop over charged track hypotheses
	for(UInt_t loc_i = 0; loc_i < Get_NumChargedHypos(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dChargedHypoWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}

	//Loop over neutral particle hypotheses
	for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dNeutralHypoWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}
*/

	/************************************ EXAMPLE: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/
/*
	Bool_t locIsEventCut = true;
	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i) {
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);
		// Is used to indicate when combos have been cut
		if(dComboWrapper->Get_IsComboCut())
			continue;
		locIsEventCut = false; // At least one combo succeeded
		break;
	}
	if(!locIsEventCut && dOutputTreeFileName != "")
		Fill_OutputTree();
*/

	return kTRUE;
}

void DSelector_ver20::Finalize(void)
{
	//Save anything to output here that you do not want to be in the default DSelector output ROOT file.

	//Otherwise, don't do anything else (especially if you are using PROOF).
		//If you are using PROOF, this function is called on each thread,
		//so anything you do will not have the combined information from the various threads.
		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem.

	//DO YOUR STUFF HERE

	//CALL THIS LAST
	DSelector::Finalize(); //Saves results to the output file
}
