#include <QString>

#define MAX_DATA 100

struct QStr3D {
	QString x;
	QString y;
	QString z;
};

struct QStr2D {
	QString x;
	QString y;
	QString z;
};
		
struct DENSITY {
	QString x1;
	QString x2;	
	QString y1;
	QString y2;	
	QString z1;
	QString z2;
	QString rho;
};
	
struct int3d {
	int x;
	int y;
	int z;
};


struct  BEAM {
    QString beamWeight;
	QString sourceType;
	QString deviceType;
	QString deviceKey;
	QString applicator;
	QString nominalEnergy;
	QString eventNumber;
	QString waterHistory;
	QString historyRepeat;
	QString futherRepeat;
	QString batch;
	QStr3D  isocenter;
	QString gantryAngle;
	QString gantryMode;
	QString gantryStart;
	QString gantryStop;
	QString tableAngle;
	QString collAngle;
	QString collWidthX;
	QString collWidthY;
	QString collLeftX;
	QString collRightX;
	QString collLeftY;
	QString collRightY;
	QString irregField;
	QString irregX[MAX_DATA];
	QString irregY[MAX_DATA];
	QString mlcName;
	QString mlcModel;
	QString mlcMode;
	QString nLeaves;
	QString mlcOrient;
	QString mlcMaterial;
	QString mlcUpperLimit;
	QString mlcLowerLimit;
	QString mlcCenter;
	QString mlcRadius;
	QString leafWidth[MAX_DATA];
	QString leftStart[MAX_DATA];
	QString rightStart[MAX_DATA];
	QString leftStop[MAX_DATA];
	QString rightStop[MAX_DATA];
	bool    collSymX;
	bool    collSymY;	
};

class VMC {
	public:

	  QString PNAME;   // Patient Name
      QString FNAME;   // Plane Name
	  bool dmxFound;
	  bool phantomFound;
      int nBeams;
	  int currentBeam;
	  
	  QString title;
	  
	  int3d nProfiles;
	  int3d nPlanes;
	  
	  QStr3D  voxelSize;
	  QStr3D  dimension;
	  DENSITY density[MAX_DATA];
	  int     nDensity;
	  
	  QString write3dDose;
	  QString xyPlane[MAX_DATA];
	  QString xzPlane[MAX_DATA];
	  QString yzPlane[MAX_DATA];
	  QStr2D  xProfile[MAX_DATA];
	  QStr2D  yProfile[MAX_DATA];
	  QStr2D  zProfile[MAX_DATA];
	  QString doseType;
	  QString photoFactor;
	  QString numFractions;
	  QStr3D  referencePoint;
	  QString randomSet;

	  QString k0cut;
	  QString k1cut;
	  	  
	  QString eCutOff;
	  QString pCutOff;
	  QString eStepE;
	  
	  BEAM     beam[MAX_DATA];
	  
	  QString RSD;
	  QString bdtPath;
	  QString physPath;
	  QStringList bdtList;
	  QStringList physList;
};

