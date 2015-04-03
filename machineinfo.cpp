#include "machineinfo.h"
#include "ui_machineinfo.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDomElement>
#include <QDir>
#include <QMessageBox>
#include <QSettings>


using namespace std;
#include <iostream>
#include <cmath>


MachineInfo::MachineInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MachineInfo)
{
    ui->setupUi(this);
}

MachineInfo::~MachineInfo()
{
    delete ui;
}


#ifndef XML
#define XML on
#endif

#ifdef XML
class mInfoStatus {
  public:
     QString value, date;
};

QDomElement mInfoStatusToNode (QDomDocument &d, const mInfoStatus &s, QString e){
   QDomElement elm = d.createElement(e);
   elm.setAttribute("value", s.value);
   elm.setAttribute("date", s.date);
   return elm;
};
#endif

mInfoUser *mInfoUsr = new mInfoUser;

void MachineInfo::init() {
 QRegExp regExp2d("[1-9]\\d{0,1}");
 QRegExp regExp3d("[1-9]\\d{0,2}");
 QRegExp regExpReal("\\d*\\.\\d+");

 ui->EnergyLineEdit->setValidator(new QRegExpValidator(regExp2d, this));
 ui->MaxFSWLineEdit->setValidator(new QRegExpValidator(regExp3d, this));
 ui->MaxFSLLineEdit->setValidator(new QRegExpValidator(regExp3d, this));
 ui->SFDLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 //ui->AvalLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->ESCDLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->SUJDLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->SLJDLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->UJTLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->LJTLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->SMDLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->tMLCLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->rMLCLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->cMLCLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->nMLCLineEdit->setValidator(new QRegExpValidator(regExp3d, this));
 ui->tkMLCLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 ui->iMLCLineEdit->setValidator(new QRegExpValidator(regExp3d, this));
 ui->thMLCLineEdit->setValidator(new QRegExpValidator(regExpReal, this));
 mInfoUsr->Aval = "9.0";
 mInfoUsr->Zval = "0.5";
}

void MachineInfo::clearModifiedAll()
{
   // FileNameLineEdit->clear();
   // ManufacturerLineEdit->clear();
   // ModelLineEdit->clear();
   ui->EnergyLineEdit->clear();
   ui->MaxFSWLineEdit->clear();
   ui->rMLCLineEdit->clear();
   ui->tkMLCLineEdit->clear();
   ui->ESCDLineEdit->clear();
   ui->SLJDLineEdit->clear();
   ui->SUJDLineEdit->clear();
   ui->SMDLineEdit->clear();
   ui->cMLCLineEdit->clear();
   ui->nMLCLineEdit->clear();
   ui->LJTLineEdit->clear();
   ui->tMLCLineEdit->clear();
   ui->UJTLineEdit->clear();
   ui->SFDLineEdit->clear();
   ui->AvalLineEdit->clear();
   ui->ZvalLineEdit->clear();
   ui->MaxFSLLineEdit->clear();
   ui->iMLCLineEdit->clear();
   ui->thMLCLineEdit->clear();

}

void MachineInfo::updateAll()
{
   // FileNameLineEdit->setText(mInfoUsr->CurrentModelFile.simplified());
   // ManufacturerLineEdit->setText(mInfoUsr->VENDOR.simplified());
   if (mInfoUsr->MLCtype.contains("None"))ui->comboBoxMLCtype->setCurrentIndex(0);
   if (mInfoUsr->MLCtype.contains("SIMPLE-MLC"))ui->comboBoxMLCtype->setCurrentIndex(1);
   if (mInfoUsr->MLCtype.contains("DBLFOCUS-MLC"))ui->comboBoxMLCtype->setCurrentIndex(2);
   if (mInfoUsr->MLCtype.contains("RNDFOCUS-MLC"))ui->comboBoxMLCtype->setCurrentIndex(3);
   if (mInfoUsr->MLCtype.contains("ELEKTA-MLC"))ui->comboBoxMLCtype->setCurrentIndex(4);
   if (mInfoUsr->MLCtype.contains("VARIAN-MLC"))ui->comboBoxMLCtype->setCurrentIndex(5);
   // ModelLineEdit->setText(mInfoUsr->MODEL.simplified());
   ui->LineEditMLCtype->setText(mInfoUsr->XIOMLC.simplified());
   ui->EnergyLineEdit->setText(mInfoUsr->E0.simplified());
   ui->MaxFSWLineEdit->setText(mInfoUsr->MAXFW.simplified());
   ui->rMLCLineEdit->setText(mInfoUsr->rMLC.simplified());
   ui->tkMLCLineEdit->setText(mInfoUsr->tkMLC.simplified());
   ui->ESCDLineEdit->setText(mInfoUsr->ESCD.simplified());
   ui->SLJDLineEdit->setText(mInfoUsr->SLJD.simplified());
   ui->SUJDLineEdit->setText(mInfoUsr->SUJD.simplified());
   ui->SMDLineEdit->setText(mInfoUsr->SMD.simplified());
   ui->cMLCLineEdit->setText(mInfoUsr->cMLC.simplified());
   ui->nMLCLineEdit->setText(mInfoUsr->nMLC.simplified());
   ui->LJTLineEdit->setText(mInfoUsr->LJT.simplified());
   ui->tMLCLineEdit->setText(mInfoUsr->tMLC.simplified());
   ui->UJTLineEdit->setText(mInfoUsr->UJT.simplified());
   ui->SFDLineEdit->setText(mInfoUsr->SFD.simplified());
   ui->AvalLineEdit->setText(mInfoUsr->Aval.simplified());
   ui->ZvalLineEdit->setText(mInfoUsr->Zval.simplified());
   ui->MaxFSLLineEdit->setText(mInfoUsr->MAXFL.simplified());
   ui->iMLCLineEdit->setText(mInfoUsr->iMLC.simplified());
   ui->thMLCLineEdit->setText(mInfoUsr->thMLC.simplified());

   if (mInfoUsr->MX.contains("1")) {
      ui->xMLCcheckBox->setChecked(true);
   } else {
      ui->xMLCcheckBox->setChecked(false);
   }
   if (mInfoUsr->MY.contains("1")) {
      ui->yMLCcheckBox->setChecked(true);
   }  else {
      ui->yMLCcheckBox->setChecked(false);
   }

   if (mInfoUsr->oMLC.contains("None"))ui->oMLCcomboBox->setCurrentIndex(0);
   if (mInfoUsr->oMLC.contains("X"))ui->oMLCcomboBox->setCurrentIndex(1);
   if (mInfoUsr->oMLC.contains("Y"))ui->oMLCcomboBox->setCurrentIndex(2);

   mInfoUsr->isMachineModified=false;

}

void MachineInfo::setMDIR(QString usrText) {
 mInfoUsr->MDIR = usrText;
 // QTextOStream(&mInfoUsr->MDIR) << usrText;
}

void MachineInfo::setLHOME(QString usrText) {
 mInfoUsr->LHOME = usrText;
}

void MachineInfo::setMODEL(QString usrText) {
 mInfoUsr->MODEL = usrText;
 mInfoUsr->CurrentMachine = usrText;
}

void MachineInfo::setModelFile(QString usrText) {
 mInfoUsr->CurrentModelFile = usrText;
}

void MachineInfo::setVendor(QString usrText) {
 mInfoUsr->VENDOR = usrText;
}

void MachineInfo::setTMPDIR(QString usrText) {
 mInfoUsr->TMPDIR = usrText;
}

QString MachineInfo::mm2cm(QString mm) {
  bool ok;
  float value = mm.toFloat(&ok)/10.0;
  mm.sprintf("%7.2f",value);
  mm.simplified();
  return(mm);
}

void MachineInfo::clearMachineInfo() {
  mInfoUsr->VERSION="";
  mInfoUsr->CHARGE="";
  mInfoUsr->MODEL="";
  mInfoUsr->VENDOR="";
  mInfoUsr->E0="";
  mInfoUsr->REFDIST="";
  mInfoUsr->REFDEPTH="";
  mInfoUsr->MLCtype="";
  mInfoUsr->MAXFW="";
  mInfoUsr->MAXFL="";
  mInfoUsr->ESCD="";
  mInfoUsr->SMD="";
  mInfoUsr->SUJD="";
  mInfoUsr->SLJD="";
  mInfoUsr->SFD="";
  mInfoUsr->Aval="";
  mInfoUsr->Zval="";
  mInfoUsr->tMLC="";
  mInfoUsr->UJT="";
  mInfoUsr->LJT="";
  mInfoUsr->rMLC="";
  mInfoUsr->cMLC="";
  mInfoUsr->MX="";
  mInfoUsr->MY="";
  mInfoUsr->oMLC="";
  mInfoUsr->isMLC="";
  mInfoUsr->nMLC="";
  mInfoUsr->iMLC="";
  mInfoUsr->tkMLC="";
  mInfoUsr->thMLC="";
  mInfoUsr->firstJaw="";
  mInfoUsr->secondJaw="";
  mInfoUsr->thirdJaw="";
  mInfoUsr->XIOMLC="";
  mInfoUsr->MLCDIRECT="";
  mInfoUsr->XIOMLCID="";
  mInfoUsr->MLCREFDIST="";
  for (int i=0; i<100; i++)  mInfoUsr->xMLC[i]=0.0; // MLC position
}

void MachineInfo::readParm(QString mDir) {
  // cout << mDir << endl;
  QFile pFile( mDir );  // parm file in machine directory
  if (pFile.exists()) {
    QTextStream stream( &pFile );
    QString sLine;
    pFile.open( QIODevice::ReadOnly );

  // File Version Number
    sLine = stream.readLine().simplified();
  QTextStream(&sLine) >> mInfoUsr->VERSION;

  // Description
    mInfoUsr->MODEL = stream.readLine().simplified();

  // particle Charge
    mInfoUsr->CHARGE = stream.readLine().simplified();

  // Machine Year
    sLine = stream.readLine().simplified();

  // Machine Manufacturer
    sLine = stream.readLine().simplified();

  // Machine Model
    sLine = stream.readLine().simplified();

  // Target to Flattening Filter Distance
    mInfoUsr->SFD = stream.readLine().simplified();
    // QTextIStream(&sLine) >> mInfoUsr->SFD;
    mInfoUsr->SFD = mm2cm(mInfoUsr->SFD);

  // Nominal Energy
    mInfoUsr->E0 = stream.readLine().simplified();
  //QTextIStream(&sLine) >> mInfoUsr->E0;

  // Reference Depth
    mInfoUsr->REFDEPTH = stream.readLine().simplified();
  //QTextIStream(&sLine) >> mInfoUsr->REFDEPTH;

  // Reference Distance (SCD/SAD)
    sLine = stream.readLine().simplified();
  QTextStream(&sLine) >> mInfoUsr->REFDIST;

  // Horizontal Scan Reference Position
    mInfoUsr->REFDIST = stream.readLine().simplified();
  //QTextIStream(&sLine) >> mInfoUsr->HSCANREF;

  // Vertical Scan Reference Position
    sLine = stream.readLine().simplified();
  QTextStream(&sLine) >> mInfoUsr->VSCANREF;

  // Maximum Field Width
    mInfoUsr->MAXFW = stream.readLine().simplified();
  // QTextIStream(&sLine) >> mInfoUsr->MAXFW;

  // Maximum Field Length
    mInfoUsr->MAXFL = stream.readLine().simplified();
  //QTextIStream(&sLine) >> mInfoUsr->MAXFL;

  // Mimimum Field Width
    sLine = stream.readLine().simplified();

  // Mimimum Field Length
    sLine = stream.readLine().simplified();

    pFile.close();
  }
}
/*
void MachineInfo::readParm(QString mDir) {
  // cout << mDir << endl;
  QFile pFile( mDir );  // parm file in machine directory
  if (pFile.exists()) {
    QTextStream stream( &pFile );
    QString sLine;
    pFile.open( QIODevice::ReadOnly  );
    for (int i=0;i<15;i++) {
      sLine = stream.readLine();  // skip 8 lines and set 9th line to sLine
      // cout << i << "  |  " << sLine << endl;
      if (i == 0) QTextIStream(&sLine) >> mInfoUsr->VERSION;
      if (i == 1) QTextIStream(&sLine) >> mInfoUsr->MODEL;
      if (i == 2) QTextIStream(&sLine) >> mInfoUsr->CHARGE;
      if (i == 6) {
         QTextIStream(&sLine) >> mInfoUsr->SFD;
         mInfoUsr->SFD = mm2cm(mInfoUsr->SFD);
      }
      if (i == 7) QTextIStream(&sLine) >> mInfoUsr->E0;
      if (i == 8) QTextIStream(&sLine) >> mInfoUsr->REFDEPTH;
      if (i == 9) QTextIStream(&sLine) >> mInfoUsr->REFDIST;
      if (i == 9) QTextIStream(&sLine) >> mInfoUsr->HSCANREF;
      if (i == 10) QTextIStream(&sLine) >> mInfoUsr->VSCANREF;
      if (i == 12) QTextIStream(&sLine) >> mInfoUsr->MAXFW;
      if (i == 13) QTextIStream(&sLine) >> mInfoUsr->MAXFL;
    }
    pFile.close();
  }
}
*/
void MachineInfo::readCollim(QString mDir) {
  QFile cFile( mDir );  // collim file in machine directory
  if (cFile.exists()) {
    QTextStream stream( &cFile );
    QString sLine;
    cFile.open( QIODevice::ReadOnly  );

  // Version Number
    mInfoUsr->VERSION = stream.readLine().simplified();
  // QTextIStream(&sLine) >> mInfoUsr->VERSION;

  // Effective Source to Collimator Distance
    mInfoUsr->ESCD = stream.readLine().simplified();
  // QTextIStream(&sLine) >> mInfoUsr->ESCD;
    mInfoUsr->ESCD = mm2cm(mInfoUsr->ESCD);

  // Collimator Angle
  QString colAngle = stream.readLine().simplified();
  // QTextIStream(&sLine) >> colAngle;

  // Collimator Angle Setting
  QString cAngle = stream.readLine().simplified();
  // QTextIStream(&sLine) >> cAngle;

  // Source to Tray Distance, minSTD, and max STD
  QString STD, minSTD, maxSTD;
    STD = stream.readLine().simplified();
  // QTextIStream(&sLine) >> STD;
  minSTD = STD.section(',',1,1);
  maxSTD = STD.section(',',2,2);
  STD = STD.section(',',0,0);

  // HLV of Tray
  QString hvlTray = stream.readLine().simplified();
  // QTextIStream(&sLine) >> hvlTray;

  // Thickness of Tray
  QString tTray  = stream.readLine().simplified();
  // QTextIStream(&sLine) >> tTray;

  // Material of Tray
  QString mTray = stream.readLine().simplified();
  // QTextIStream(&sLine) >> mTray;

  bool ok;
  // cout << hvlTray.toFloat(&ok) << "  " << tTray.toFloat(&ok) << endl;
  // if (hvlTray.toFloat(&ok) != 0.0 && tTray.toFloat(&ok) != 0.0) {

   // Default Values of number of HVL, thikness of HVL, and material of HVL
   QString nHVL, tHVL, mHVL;
     nHVL = stream.readLine().simplified();
   // QTextIStream(&sLine) >> nHVL;
   tHVL = nHVL.section(',',1,1);
   mHVL = mHVL.section(',',2,2);
   nHVL = nHVL.section(',',0,0);

   // Number of Trays
   QString nTrays = stream.readLine().simplified();
   // QTextIStream(&sLine) >> nTrays;
   int nTray = nTrays.toInt(&ok, 10);

   for (int iTray=0; iTray<nTray; iTray++){
      // Description of Tray
    QString dTray = stream.readLine().simplified();
     // QTextIStream(&sLine) >> dTray;

      // Tray Factor
    QString fTray  = stream.readLine().simplified();
     // QTextIStream(&sLine) >> fTray;

      // Tray Material
    QString matTray  = stream.readLine().simplified();
     // QTextIStream(&sLine) >> matTray;

      // Thickness of Tray
    QString thickTray  = stream.readLine().simplified();
     // QTextIStream(&sLine) >> thickTray;


      // Default Tray Material and Thickness but not separated
    QString mtTray  = stream.readLine().simplified();
     // QTextIStream(&sLine) >> mtTray;
   }

   // Number of Trays
   QString defTray  = stream.readLine().simplified();
   // QTextIStream(&sLine) >> defTray;
   // cout << "Number of Trays = " << defTray << endl;

   // MLC Type
     mInfoUsr->XIOMLC = stream.readLine().simplified();
   // QTextIStream(&sLine) >> mInfoUsr->XIOMLC;
   // cout << "mInfoUsr->XIOMLC = " << mInfoUsr->XIOMLC << endl;

   // Source to MLC Distance
     mInfoUsr->SMD = stream.readLine().simplified();
   // QTextIStream(&sLine) >> mInfoUsr->SMD;
   mInfoUsr->SMD = mm2cm(mInfoUsr->SMD);

   // MLC Edge AL
   QString edgeMLC = stream.readLine().simplified();
   // QTextIStream(&sLine) >> edgeMLC;

   // MLC Offset
   QString offsetMLC = stream.readLine().simplified();
   // QTextIStream(&sLine) >> offsetMLC;

   // MLC Type Number
   QString nTypeMLC = stream.readLine().simplified();
   // QTextIStream(&sLine) >> nTypeMLC;

   // MLC Leakage, Thickness, and Material
   mInfoUsr->tMLC = stream.readLine().simplified();
       //QTextIStream(&sLine) >> mInfoUsr->tMLC;
       mInfoUsr->tMLC = mm2cm(mInfoUsr->tMLC.section(',',1,1));

   // MLC Order (Length and Width)
       QString jawPosition = stream.readLine().simplified();
       // QTextIStream(&sLine) >> jawPosition;
       mInfoUsr->firstJaw = jawPosition.section(',',0,0);
       mInfoUsr->secondJaw = jawPosition.section(',',1,1);

       // Source to Upper Jaw Distance
     mInfoUsr->SUJD = stream.readLine().simplified();
       // QTextIStream(&sLine) >> mInfoUsr->SUJD;
       mInfoUsr->SUJD = mm2cm(mInfoUsr->SUJD);

   // Upper Jaw Thickness and Material
     mInfoUsr->UJT = stream.readLine().simplified();
   // QTextIStream(&sLine) >> mInfoUsr->UJT;
   mInfoUsr->UJT = mm2cm(mInfoUsr->UJT.section(',',0,0));

   // Upper Jaw Width and Length
   QString UJWL = stream.readLine().simplified();
   // QTextIStream(&sLine) >> UJWL;

       // Source to Lower Jaw Distance
     mInfoUsr->SLJD = stream.readLine().simplified();
       // QTextIStream(&sLine) >> mInfoUsr->SLJD;
       mInfoUsr->SLJD = mm2cm(mInfoUsr->SLJD);

   // Lower Jaw Thickness and Material
     mInfoUsr->LJT = stream.readLine().simplified();
   // QTextIStream(&sLine) >> mInfoUsr->LJT;
   mInfoUsr->LJT = mm2cm(mInfoUsr->LJT.section(',',0,0));

   // Lower Jaw Width and Length
   QString LJWL = stream.readLine().simplified();
   // QTextIStream(&sLine) >> LJWL;
  // }
    cFile.close();
  }
}

void MachineInfo::readMlc(QString mDir) {
  QFile mlcFile( mDir );  // mlc file in machine directory
  if (mlcFile.exists()) {
    QTextStream stream( &mlcFile );
    QString sLine;
    mlcFile.open( QIODevice::ReadOnly  );

    sLine = stream.readLine();
    QTextStream(&sLine) >> mInfoUsr->VERSION;

    QString MLCinfo="";
    sLine = stream.readLine();
    mInfoUsr->XIOMLCID = sLine.section(',',0,0).simplified();
    mInfoUsr->MLCDIRECT = sLine.section(',',1,1).simplified();
    if (mInfoUsr->MLCDIRECT.contains('0')) mInfoUsr->oMLC = "X";
    if (mInfoUsr->MLCDIRECT.contains('1')) mInfoUsr->oMLC = "Y";
    mInfoUsr->nMLC = sLine.section(',',2,2).simplified();

    sLine = stream.readLine();
    mInfoUsr->MLCREFDIST = mm2cm(sLine);

    bool ok;
    int nMLC = mInfoUsr->nMLC.toInt(&ok,10);
    // cout << "readMlc::nMLC = " << nMLC << endl;
    int iStart = 0; // MLC width change starts
    int iEnd = 0;   // MLC width change ends
    float tkMLC = 0.0;
    float thMLC = 0.0;

    for (int i=0;i<nMLC;i++) {
      sLine = stream.readLine();
      QString xMLC = mm2cm(sLine.section(',',0,0));
      bool ok;
      mInfoUsr->xMLC[i] = xMLC.toFloat(&ok);
      // cout << i << "  " << mInfoUsr->xMLC[i] << endl;
      if (i == 1) tkMLC = fabs(mInfoUsr->xMLC[i] - mInfoUsr->xMLC[i-1]);
      if (i > 0) {
      float mWidth = fabs(mInfoUsr->xMLC[i] - mInfoUsr->xMLC[i-1]);
         if (fabs(mWidth)-tkMLC > 0.01) {
            if (iStart == 0) {
              iStart = i;
              thMLC = mWidth;
            }
            else {
              iEnd = i;
            }
         }
      }
    }
    // cout << "iStart = " << iStart << "  " << "iEnd = " << iEnd << endl;
    int iMLC = (iEnd - iStart) + 1;
    if (iMLC == 1) iMLC = 0;
    mInfoUsr->iMLC.sprintf("%d",iMLC);
    mInfoUsr->tkMLC.sprintf("%d",(int)(tkMLC*10));
    mInfoUsr->thMLC.sprintf("%d",(int)(thMLC*10));

    if (mInfoUsr->XIOMLC.contains("MILLENNIUM120") ||
        mInfoUsr->XIOMLC.contains("VARIAN40") ||
        mInfoUsr->XIOMLC.contains("VARIAN26") ) {
       mInfoUsr->VENDOR = "Varian";
       mInfoUsr->MLCtype = "VARIAN-MLC";
       mInfoUsr->rMLC = "8.00";
       mInfoUsr->cMLC = mInfoUsr->SMD;
       mInfoUsr->MX = "0";
       mInfoUsr->MY = "0";
    }
    if (mInfoUsr->XIOMLC.contains("SIEMENSV50") ||
        mInfoUsr->XIOMLC.contains("SIEMENSIEC") ) {
       mInfoUsr->VENDOR = "Siemens";
       mInfoUsr->MLCtype = "DBLFOCUS-MLC";
       mInfoUsr->rMLC = "8.00";
       mInfoUsr->cMLC = mInfoUsr->SMD;
       mInfoUsr->MX = "0";
       mInfoUsr->MY = "0";
    }
    if (mInfoUsr->XIOMLC.contains("ElektaBM80Leaf") ||
        mInfoUsr->XIOMLC.contains("PHILIPSMLC") ) {
       mInfoUsr->VENDOR = "Elekta";
       mInfoUsr->MLCtype = "ELEKTA-MLC";
       mInfoUsr->rMLC = "15.0";
       mInfoUsr->cMLC = mInfoUsr->SMD;
       mInfoUsr->isMLC = "1";
       if (mInfoUsr->MLCDIRECT.contains('0')) mInfoUsr->MX = "1";
       if (mInfoUsr->MLCDIRECT.contains('1')) mInfoUsr->MY = "1";
       mInfoUsr->iMLC = "0";
       mInfoUsr->thMLC = "0";
    }

    mlcFile.close();
  }
}

void MachineInfo::getMachineInfo() {
  QString localInfo = mInfoUsr->LHOME + mInfoUsr->CurrentMachine
                    + "/" + mInfoUsr->CurrentMachine + ".info";
  QFile infoFile(localInfo);  // mlc file in machine directory
  if (infoFile.exists()) {
     getLocalInfo(localInfo);
     mInfoUsr->CurrentModelFile = mInfoUsr->CurrentMachine + ".info";
  } else {
     getXiOInfo();
     mInfoUsr->CurrentModelFile = "XiO Machine Files";
  }
}

void MachineInfo::resetMachine() {
   getXiOInfo();
   mInfoUsr->CurrentModelFile = "XiO Machine Files";
   updateAll();
}

void MachineInfo::getXiOInfo() {
  clearMachineInfo();
  QString mDir;

  mDir = mInfoUsr->MDIR + "/" + mInfoUsr->CurrentMachine + "/parm" ;
  readParm(mDir);

  mDir = mInfoUsr->MDIR + "/" + mInfoUsr->CurrentMachine + "/collim" ;
  readCollim(mDir);

  mDir = mInfoUsr->MDIR + "/" + mInfoUsr->CurrentMachine + "/mlc" ;
  readMlc(mDir);
}

void MachineInfo::getLocalInfo(QString modelFile) {
 QFile mFile( modelFile );
 if (mFile.exists()) {
  QTextStream stream( &mFile );
  QString sLine;
  mFile.open( QIODevice::ReadOnly  );
  // cout << modelFile << " is open" << endl;
  while ( !stream.atEnd() ) {
   sLine = stream.readLine();
   QString strLine = sLine.toLatin1();
   QString keyWord = strLine.section('=',0,0);
   QString keyValueTmp = strLine.section('#',0,0).section('=',1,1);
   QString keyValue = keyValueTmp.simplified();
   // cout << "keyWord = " << keyWord << "  keyValue = " << keyValue << endl;
   if (keyWord.count("MODEL",Qt::CaseSensitive) != -1)  mInfoUsr->MODEL = keyValue;
   if (keyWord.contains("MODEL"))  mInfoUsr->MODEL = keyValue;
   if (keyWord.contains("VENDOR")) mInfoUsr->VENDOR = keyValue;
   if (keyWord.contains("CHARGE")) mInfoUsr->CHARGE = keyValue;
   if (keyWord.contains("E0"))     mInfoUsr->E0 = keyValue;
   if (keyWord.contains("MLCtype")) mInfoUsr->MLCtype = keyValue;
   if (keyWord.contains("MAXFW"))  mInfoUsr->MAXFW = keyValue;
   if (keyWord.contains("MAXFL"))  mInfoUsr->MAXFL = keyValue;
   if (keyWord.contains("ESCD"))   mInfoUsr->ESCD = keyValue;
   if (keyWord.contains("SMD"))    mInfoUsr->SMD = keyValue;
   if (keyWord.contains("SUJD"))   mInfoUsr->SUJD = keyValue;
   if (keyWord.contains("SLJD"))   mInfoUsr->SLJD = keyValue;
   if (keyWord.contains("SFD"))    mInfoUsr->SFD = keyValue;
   if (keyWord.contains("AVAL"))   mInfoUsr->Aval = keyValue;
   if (keyWord.contains("ZVAL"))   mInfoUsr->Zval = keyValue;
   if (keyWord.contains("tMLC"))   mInfoUsr->tMLC = keyValue;
   if (keyWord.contains("UJT"))    mInfoUsr->UJT = keyValue;
   if (keyWord.contains("LJT"))    mInfoUsr->LJT = keyValue;
   if (keyWord.contains("rMLC"))   mInfoUsr->rMLC = keyValue;
   if (keyWord.contains("cMLC"))   mInfoUsr->cMLC = keyValue;
   if (keyWord.contains("MX"))     mInfoUsr->MX = keyValue;
   if (keyWord.contains("MY"))     mInfoUsr->MY = keyValue;
   if (keyWord.contains("oMLC"))   mInfoUsr->oMLC = keyValue;
   if (keyWord.contains("isMLC"))  mInfoUsr->isMLC = keyValue;
   if (keyWord.contains("nMLC"))   mInfoUsr->nMLC = keyValue;
   if (keyWord.contains("iMLC"))   mInfoUsr->iMLC = keyValue;
   if (keyWord.contains("tkMLC"))  mInfoUsr->tkMLC = keyValue;
   if (keyWord.contains("thMLC"))  mInfoUsr->thMLC = keyValue;
   if (keyWord.contains("XIOMLC"))  mInfoUsr->XIOMLC = keyValue;
  }
 }
 mFile.close();

}

void MachineInfo::writeModified (QString group){
   QDateTime currentDateTime = QDateTime::currentDateTime();
   QString DT = currentDateTime.toString();
   // cout << "Modified = " << group << "  " << mInfoUsr->LHOME +mInfoUsr->CurrentMachine << endl;
   QString mDir = mInfoUsr->LHOME + mInfoUsr->CurrentMachine;

#ifdef XML
   QString LHOME = mDir;
   QString mName = group;
   QFile xmlFile(LHOME+"/status.xml");
   // cout << LHOME+"/status.xml" << endl;
   if (!xmlFile.open( QIODevice::ReadOnly )){
      cout << "No " << LHOME.toStdString()+"/status.xml is found" << endl;
      exit(-1);
   }

   QDomDocument doc(mName);
   doc.setContent(&xmlFile);
   xmlFile.close();

   QDomElement root = doc.documentElement();
   if (root.tagName() != mName) {
      // cout << "Tag Name ("<<eRoot.tagName()<<") is different from " << mName << endl;
      exit(-1);
   }

   QDomNode n = root.firstChild();

   QString keyWord = "MachineModified";
   QString keyValue = "Modified";
   // Set attribute for existing tag
   bool isDone = false;
   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         if(e.tagName() == keyWord) {
        e.setAttribute("value", keyValue);
        isDone = true;
         }
      }
      n = n.nextSibling();
   }
   if(!isDone) {
      mInfoStatus s;  // Status class

      QString eName = keyWord;
      s.value = keyValue;  s.date = DT;
      root.appendChild(mInfoStatusToNode(doc, s, eName));
   }

   n = root.firstChild();
   keyWord = "Energy";
   keyValue = mInfoUsr->E0;
   // Set attribute for existing tag
   isDone = false;
   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         if(e.tagName() == keyWord) {
        e.setAttribute("value", keyValue);
        isDone = true;
         }
      }
      n = n.nextSibling();
   }
   if(!isDone) {
      mInfoStatus s;  // Status class

      QString eName = keyWord;
      s.value = keyValue;  s.date = DT;
      root.appendChild(mInfoStatusToNode(doc, s, eName));
   }

   n = root.firstChild();
   keyWord = "ModelFile";
   keyValue = mInfoUsr->CurrentModelFile;
   // Set attribute for existing tag
   isDone = false;
   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         if(e.tagName() == keyWord) {
        e.setAttribute("value", keyValue);
        isDone = true;
         }
      }
      n = n.nextSibling();
   }
   if(!isDone) {
      mInfoStatus s;  // Status class

      QString eName = keyWord;
      s.value = keyValue;  s.date = DT;
      root.appendChild(mInfoStatusToNode(doc, s, eName));
   }

   n = root.firstChild();
   keyWord = "MLCtype";
   keyValue = mInfoUsr->MLCtype;
   // Set attribute for existing tag
   isDone = false;
   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         if(e.tagName() == keyWord) {
        e.setAttribute("value", keyValue);
        isDone = true;
         }
      }
      n = n.nextSibling();
   }
   if(!isDone) {
      mInfoStatus s;  // Status class

      QString eName = keyWord;
      s.value = keyValue;  s.date = DT;
      root.appendChild(mInfoStatusToNode(doc, s, eName));
   }
   // QFile xmlFile(LHOME+"/status.xml");
   // cout << LHOME+"/status.xml" << endl;
   xmlFile.open(QIODevice::WriteOnly );
   QTextStream txtStrm(&xmlFile);
   txtStrm << doc.toString();
   xmlFile.close();

#else
 QSettings settings;
 settings.insertSearchPath(QSettings::Unix, mDir);
 settings.beginGroup("/"+group);
 settings.writeEntry("/"+group+"/MachineModified", "Modified");
 settings.writeEntry("/"+group+"/MachineModified_DT", DT);
 settings.writeEntry("/"+group+"/Energy", mInfoUsr->E0);
 settings.writeEntry("/"+group+"/Energy_DT", DT);
 settings.writeEntry("/"+group+"/ModelFile", mInfoUsr->CurrentModelFile);
 settings.writeEntry("/"+group+"/ModelFile_DT", DT);
 settings.writeEntry("/"+group+"/MLCtype", mInfoUsr->MLCtype);
 settings.writeEntry("/"+group+"/MLCtype_DT", DT);
 settings.endGroup();
 // cout << "End of writeSetting" << endl;
#endif
}

void MachineInfo::writeNotModified (QString group){
 QDateTime currentDateTime = QDateTime::currentDateTime();
 QString DT = currentDateTime.toString();
 // cout << "Not Modified = " << group << "  " << mInfoUsr->LHOME+mInfoUsr->CurrentMachine << endl;
  QString mDir = mInfoUsr->LHOME + mInfoUsr->CurrentMachine;

#ifdef XML
   QString LHOME = mDir;
   QString mName = group;
   QFile xmlFile(LHOME+"/status.xml");
   // cout << LHOME+"/status.xml" << endl;
   if (!xmlFile.open(QIODevice::ReadOnly )){
      cout << "No " << LHOME.toStdString()+"/status.xml is found" << endl;
      exit(-1);
   }

   QDomDocument doc(mName);
   doc.setContent(&xmlFile);
   xmlFile.close();

   QDomElement root = doc.documentElement();
   if (root.tagName() != mName) {
      // cout << "Tag Name ("<<eRoot.tagName()<<") is different from " << mName << endl;
      exit(-1);
   }

   QDomNode n = root.firstChild();

   QString keyWord = "MachineModified";
   QString keyValue = "NotModified";
   // Set attribute for existing tag
   bool isDone = false;
   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         if(e.tagName() == keyWord) {
        e.setAttribute("value", keyValue);
        isDone = true;
         }
      }
      n = n.nextSibling();
   }
   if(!isDone) {
      mInfoStatus s;  // Status class

      QString eName = keyWord;
      s.value = keyValue;  s.date = DT;
      root.appendChild(mInfoStatusToNode(doc, s, eName));
   }

   // QFile xmlFile(LHOME+"/status.xml");
   // cout << LHOME+"/status.xml" << endl;
   xmlFile.open(QIODevice::WriteOnly );
   QTextStream txtStrm(&xmlFile);
   txtStrm << doc.toString();
   xmlFile.close();

#else
 QSettings settings;
 settings.insertSearchPath(QSettings::Unix, mDir);
 settings.beginGroup("/"+group);
 settings.writeEntry("/"+group+"/MachineModified", "NotModified");
 settings.writeEntry("/"+group+"/MachineModified_DT", DT);
 //settings.writeEntry("/"+group+"/Energy", mInfoUsr->E0);
 //settings.writeEntry("/"+group+"/ModelFile", mInfoUsr->CurrentModelFile);
 //settings.writeEntry("/"+group+"/MLCtype", mInfoUsr->MLCtype);
 settings.endGroup();
#endif
 // cout << "End of writeSetting" << endl;
}

void MachineInfo::writeMachineInfo() {
  mInfoUsr->CurrentModelFile = mInfoUsr->CurrentMachine + ".info";
  QString oFileName = mInfoUsr->LHOME
                    + mInfoUsr->CurrentMachine + "/"
                    + mInfoUsr->CurrentModelFile;
  // cout << oFileName << endl;
  QFile oFile( oFileName );
  oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
  QTextStream oStream( &oFile );

  // oStream << "MODEL = " << ModelLineEdit->text()
  //         << "   # Model " << endl ;
  oStream << "VENDOR = " << mInfoUsr->VENDOR
          << "   # Vendor " << endl ;
  oStream << "CHARGE = " << mInfoUsr->CHARGE
          << "   # Particle Charge " << endl ;
  oStream << "MLCtype = " << ui->comboBoxMLCtype->currentText()
          << "   # MLC Type" << endl;
  oStream << "E0 = " << ui->EnergyLineEdit->text()
          << "   # Nominal Photon Energy" << endl ;
  oStream << "MAXFW = " << ui->MaxFSWLineEdit->text()
          << "   # Max Field Width (mm)" << endl ;
  oStream << "MAXFL = " << ui->MaxFSLLineEdit->text()
          << "   # Max Field Length (mm)" << endl ;
  oStream << "ESCD = " << ui->ESCDLineEdit->text()
          << "   # Effective Source to Collimator Distance (cm)" << endl ;
  oStream << "SMD = " << ui->SMDLineEdit->text()
          << "   # Source to MLC (Middle) Distance (cm)" << endl ;
  oStream << "SUJD = " << ui->SUJDLineEdit->text()
          << "   # Source to Upper Jaw Distance (cm)" << endl ;
  oStream << "SLJD = " << ui->SLJDLineEdit->text()
          << "   # Source to Lower Jaw Distance (cm)" << endl ;
  oStream << "SFD = " << ui->SFDLineEdit->text()
          << "   # Source to Flattening Filter Bottom Distance (cm)" << endl ;
  oStream << "AVAL = " << ui->AvalLineEdit->text()
          << "   # Flattening Filter Attenuation Factor (A-VALUE)" << endl ;
  oStream << "ZVAL = " << ui->ZvalLineEdit->text()
          << "   # Min Thickness of Flattening Filter (Z-VALUE)" << endl ;
  oStream << "tMLC = " << ui->tMLCLineEdit->text()
          << "   # MLC Thickness (cm)" << endl ;
  oStream << "UJT = " << ui->UJTLineEdit->text()
          << "   # Upper Jaw Thickness (cm)" << endl ;
  oStream << "LJT = " << ui->LJTLineEdit->text()
          << "   # Lower Jaw Thickness (cm)" << endl ;
  oStream << "rMLC = " << ui->rMLCLineEdit->text()
          << "   # Radius of Round Leaf (cm)" << endl ;
  oStream << "cMLC = " << ui->cMLCLineEdit->text()
          << "   # Center of Round Leaf (cm)" << endl ;
  if(ui->xMLCcheckBox->isChecked()) {
    oStream << "MX = 1   # MLC Including for X Penumbra Calculation (1/0)\n";
  } else {
    oStream << "MX = 0   # MLC Including for X Penumbra Calculation (1/0)\n";
  }
  if(ui->yMLCcheckBox->isChecked()) {
    oStream << "MY = 1   # MLC Including for Y Penumbra Calculation (1/0)\n";
  } else {
    oStream << "MY = 0   # MLC Including for Y Penumbra Calculation (1/0)\n";
  }
  oStream << "oMLC = " << ui->oMLCcomboBox->currentText()
          << "   # Orientation of MLC Travel (X/Y)" << endl;
   if (mInfoUsr->MLCtype.contains("ELEKTA-MLC"))
      oStream << "isMLC = 1    # MLC Including in Model (1/0)\n";
   else
      oStream << "isMLC = 0    # MLC Including in Model (1/0)\n";

   oStream << "nMLC = " << ui->nMLCLineEdit->text()
           << "   # Number of Leaf Pairs" << endl ;
   oStream << "iMLC = " << ui->iMLCLineEdit->text()
           << "   # Number of Inner (thin) MLCs" << endl ;
   oStream << "tkMLC = " << ui->tkMLCLineEdit->text()
           << "   # Thick MLC width (mm)" << endl ;
   oStream << "thMLC = " << ui->thMLCLineEdit->text()
           << "   # Thin MLC width (mm)" << endl ;
   oStream << "XIOMLC = " << ui->LineEditMLCtype->text()
           << "   # XiO MLC Type" << endl ;

  oFile.close();
}

void MachineInfo::checkModified(){

 // if (ModelLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (mInfoUsr->MLCtype.contains(ui->comboBoxMLCtype->currentText()))
                                 mInfoUsr->isMachineModified = true;
 if (ui->EnergyLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->MaxFSWLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->MaxFSLLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->ESCDLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->SMDLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->SUJDLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->SLJDLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->SFDLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->AvalLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->ZvalLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->tMLCLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->UJTLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->LJTLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->rMLCLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->cMLCLineEdit->isModified()) mInfoUsr->isMachineModified = true;

 if(ui->xMLCcheckBox->isChecked()) {
   if (mInfoUsr->MX.contains("0")) mInfoUsr->isMachineModified = true;
 } else {
   if (mInfoUsr->MX.contains("1")) mInfoUsr->isMachineModified = true;
 }
 if(ui->yMLCcheckBox->isChecked()) {
   if (mInfoUsr->MY.contains("0")) mInfoUsr->isMachineModified = true;
 } else {
   if (mInfoUsr->MY.contains("1")) mInfoUsr->isMachineModified = true;
 }
 if (mInfoUsr->oMLC.contains(ui->oMLCcomboBox->currentText()))
     mInfoUsr->isMachineModified = true;

 if (ui->nMLCLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->iMLCLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->tkMLCLineEdit->isModified()) mInfoUsr->isMachineModified = true;
 if (ui->thMLCLineEdit->isModified()) mInfoUsr->isMachineModified = true;

 writeMachineInfo();
  if (mInfoUsr->isMachineModified == true) {
   // cout << "Save the new machine specification into " << mInfoUsr->LHOME << "/"
   //         << mInfoUsr->CurrentMachine << "/" << mInfoUsr->CurrentMachine << ".info" << endl;
   writeModified(mInfoUsr->CurrentMachine);

  QString mDir = mInfoUsr->LHOME + mInfoUsr->CurrentMachine;
  QDir *afitDir = new QDir;
  afitDir->setPath(mDir+"/afit");
  if(afitDir->exists()) {
     QString AFIT_INP_TMP = mDir + "/afit/afit.inp.tmp";
     QFile mFile(AFIT_INP_TMP);
     if (mFile.exists()) {
        QString CMD = "rm " + AFIT_INP_TMP;
        if (system(CMD.toStdString().c_str()) != 0)
            cout << "ERROR: Somethings are wrong: " << CMD.toStdString() << endl ;
     }
  }
  } else {
   writeNotModified(mInfoUsr->CurrentMachine);
  }


  // close();
}

void MachineInfo::Cancel(){
  getMachineInfo();
  updateAll();
  writeNotModified(mInfoUsr->CurrentMachine);
  // close();
}
