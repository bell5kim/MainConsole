#include <QString>

#define MAXDATA 5000

class User {
	public:

		QString PWD;      // Preesent Working Directory
        QString HOME;
		QString FOCUS;    // FOCUS Directory
		QString LHOME;     // Local Home Directory
		QString LBIN;     // Local BIN Directory
		QString LDIR;     // Local Machine Directory
		QString TMPDIR;     // Local TEMP Directory
		QString TELE_DIR;     // XiO TELE Directory
		QString XVMC_HOME; // Local XVMC_HOME Directory 
		QString XVMC_WORK; // Local XVMC_WORK Directory
		QString AFIT_DIR;   // Local AFIT Directory
		QString WFIT_DIR;   // Local WFIT Directory
		QString MONO_MC_DIR;   // Local WFIT Directory

		QString RecentMachine;   // Machine Name
		QString RecentVendor;    // Machine Vendor Name
		QString RecentModel;     // Model Name
		QString RecentModelFile; // Model File in lib
		
		QString CurrentMachine;   // Machine Name
		QString CurrentVendor;    // Machine Vendor Name
		QString CurrentModel;     // Model Name
		QString CurrentModelFile; // Model File in lib
		
		QString comboBoxModelFile; // Model File in lib
		
		bool isModelFileModified;
		bool isMachineModified;
		bool isModelModified;
		bool afitDirExists;
		bool wfitDirExists;
		bool PBCommDirExists;
		bool isAfitDone;
		bool isWfitDone;
		bool isMonoMCDone;
		
		QString VERSION;
		QString CHARGE;
		QString MODEL;
		QString VENDOR;
		QString E0;
		QString REFDIST;
		QString REFDEPTH;
		QString SAD;
		QString SSD;
		QString MLCtype;		
		QString MAXFW;
		QString MAXFL;
		QString ESCD;
		QString SMD;
		QString SUJD;
		QString SLJD;
		QString SFD;
		QString tMLC;
		QString UJT;
		QString LJT;
		QString rMLC;
		QString cMLC;
		QString MX;
		QString MY;
		QString oMLC;
		QString isMLC;
		QString nMLC;
		QString iMLC;
		QString tkMLC;
		QString thMLC;
		QString AVAL;
		QString ZVAL;
		
		QString HSCANREF;   // horz_scan_ref Proton
		QString VSCANREF;   // vert_scan_ref Proton
		
        QString firstJaw;
		QString secondJaw;
		QString thirdJaw;
		QString XIOMLC;
		QString MLCDIRECT;
		QString XIOMLCID;
		QString MLCREFDIST;
		
		float xMLC[100]; // MLC position
		int nLines;
		
	   // parameters for BDT file
        QString p0;  // probabilty for primary photons
        QString s0;  // primary width
        QString h0;  // horn correction parameter 0
        QString h1;  // horn correction parameter 1
        QString h2;  // horn correction parameter 2
        QString h3;  // horn correction parameter 3
        QString h4;  // horn correction parameter 4
        QString ss;  // scatter width

        QString norm;  // normalization factor
        QString lval;  // parameter l
        QString bval;  // parameter b
        QString aval;  // parameter a
        QString zval;  // parameter z
        QString Emin;  // minimum energy
        QString Emax;  // maximum energy
        QString pcon;  // probability for electron contam.

		QString E;      // photon energy
		QString ZI;     // iso-center position
		QString Z0;     // target (primary source) position
		QString ZS;     // filter (scatter source) position
		QString ZC;     // Collimator Position
		QString ZM;     // modifier (MLC) position
		QString ZE;     // Electron Contamination Source Distance
		QString ZX;     // x collimator position
		QString ZY;     // y collimator position
		QString XN;     // Norminal Origin of X
		QString YN;     // Norminal Origin of Y
		QString ZN;     // Norminal Origin of Z
		QString WXN;    // Maximum X Field Size
        QString WYN;    // Maximum Y Field Size
		QString FFRad;  //  Flattening Filter Radius
		QString eEnergy;// Electron Mean Energy
		QString nu;     // nu Value
  		
        QString gy_mu_dmax;
        QString Emean;
        QString Eprob;
        QString chi;
        QString avgdev;
        QString normValue;
        QString version;
        QString particle;
		QString ID;

   			
};

