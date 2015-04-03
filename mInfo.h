#ifndef MINFO_H
#define MINFO_H


#define MAXDATA 5000

class mInfoUser {
    public:

        QString PWD;      	// Preesent Working Directory
        QString FOCUS;    	// FOCUS Directory
        QString LHOME;     	// Local Home Directory
        QString LBIN;     	// Local BIN Directory
        QString LDIR;     	// Local Machine Directory
        QString TMPDIR;     	// Local TEMP Directory
        QString MDIR;     	// XiO MC Machine Directory
        QString XVMC_HOME; 	// Local XVMC_HOME Directory
        QString XVMC_WORK; 	// Local XVMC_WORK Directory
        QString AFITDIR;   	// Local AFIT Directory
        QString WFITDIR;   	// Local WFIT Directory
        QString MONOMCDIR;   	// Local WFIT Directory

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
        QString MLCtype;
        QString MAXFW;
        QString MAXFL;
        QString ESCD;
        QString SMD;
        QString SUJD;
        QString SLJD;
        QString SFD;
        QString Aval;
        QString Zval;
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
};
#endif // MINFO_H
