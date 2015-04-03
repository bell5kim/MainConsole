#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDomElement>
#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include <cmath>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>

using namespace std;

#include "mainconsole.h"
#include "ui_mainconsole.h"

// #include "machineinfo.h"
#include "xvmc.h"
#include "user.h"

// Phantom Size Option Default 30x30x30
#define PHANTOM_40x40x30 ON
// #define PHANTOM_40x40x40

#define MAX_STR_LEN     132
#define PASS  0
#define FAIL -1

User    *usr = new User;
VMC     *vmc = new VMC;

#ifndef XML
#define XML ON
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


MainConsole::MainConsole(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainConsole)
{
    ui->setupUi(this);

    init();
}

MainConsole::~MainConsole()
{
    delete ui;
}


#ifdef XML
class Status {
  public:
     QString value, date;
};

// -----------------------------------------------------------------------------

QDomElement StatusToNode (QDomDocument &d, const Status &s, QString e)
{
   QDomElement elm = d.createElement(e);
   elm.setAttribute("value", s.value);
   elm.setAttribute("date", s.date);
   return elm;
};
#endif
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
void MainConsole::init() {
   // get User's Home
   usr->HOME = getenv("HOME");
   // get Present Working Directory. It must be the Monaco Fitting Home
   usr->PWD = getenv("PWD");
   // Read Previous Settings
   QString HOSTNAME = getenv("HOSTNAME");
   QString gName = HOSTNAME.section('.',0,0);
#ifdef XML
   QString  HOME=getenv("HOME");
   std::cout << "rcFile = " << HOME.toStdString()+"/.monacobct" << endl;
   QFile rcFile(HOME+"/.monacobct");
#else
   QFile rcFile(usr->HOME + "/.qt/" + gName.toLower() + "rc");
   std::cout << usr->HOME + "/.qt/" + gName.toLower() + "rc" << endl;
#endif
   if (rcFile.exists()) {
      readSettings(gName);  // It reads $HOME/.qt/HOSTNAMErc
   } else { // No rc file
      // User Setting using Dialogs
      usr->FOCUS = getenv("FOCUS");
      if (usr->FOCUS == "") {
         usr->FOCUS = pickDir("~", "Find and Choose FOCUS Directory");
         QTextStream (stdout) << "FOCUS = " << usr->FOCUS << endl;
         usr->FOCUS = usr->FOCUS.section('/',0,-1)+"/";
      }
      usr->TELE_DIR = pickDir(usr->FOCUS, "Find and Choose a Tele Directory");
      QTextStream (stdout) << "TELE_DIR = " << usr->TELE_DIR << endl;
      usr->TELE_DIR = usr->TELE_DIR.section('/',0,-1)+"/";
      ui->lineEditTeleDir->setText(usr->TELE_DIR);
      usr->LHOME = pickDir(usr->HOME,"Choose Local Home Dir which must include bin directory");
      QTextStream (stdout) << "LHOME = " << usr->LHOME << endl;
      usr->LHOME = usr->LHOME.section('/',0,-1)+"/";
      ui->lineEditLocalDir->setText(usr->LHOME);
      usr->LBIN = usr->LHOME + "bin";

      getMCMachine();  // get MC machine in XiO directory (tele)

#ifdef REMOVED_Jun162006
      usr->RecentMachine = ui->comboBoxMachine->currentText();
      getVendors();
      usr->RecentVendor = ui->comboBoxVendor->currentText();
      getModels();
      usr->RecentModelFile = ui->comboBoxModel->currentText();
#endif
      writeSettings(gName);
   }

   // Update TELE Directory Name on the Dialog
   ui->lineEditTeleDir->setText(usr->TELE_DIR);
   // Update LOCAL Directory Name on the Dialog
   ui->lineEditLocalDir->setText(usr->LHOME);

   // Initially All Group Boxes are Disabled
   ui->groupBoxAFIT->setEnabled(false);
   ui->groupBoxMonoMC->setEnabled(false);
   ui->groupBoxWFIT->setEnabled(false);
   ui->groupBoxVerification->setEnabled(false);
   ui->groupBoxMCPB->setEnabled(false);
   ui->groupBoxPBComm->setEnabled(false);
   ui->pushButtonPBPack->setEnabled(false);

   usr->comboBoxModelFile="";

   // Get all MC machine names available in usr->TELE_DIR (TELE) direcory
   // and insert them into the machine list
   getMCMachine();

   // Current Machine Name
   QString mName = ui->comboBoxMachine->currentText();
   // Current Machine Directory
   QString mDir = usr->LHOME+mName;

#ifdef REMOVED_Jun162006
   // Update Current Machine with the Previous Working Machine
   if (usr->RecentMachine.simplified() == "")
      usr->CurrentMachine = mName;
   else
      usr->CurrentMachine = usr->RecentMachine.simplified();
   // Update Current Model with the Previous Working Model
   if (usr->RecentModel.simplified() == "")
      getModels();
   else
      usr->CurrentModel = usr->RecentModel.simplified();
   // Update Current Vendor with the Previous Working Vendor
   if (usr->RecentVendor.simplified() == "")
      getVendors();
   else
   usr->CurrentVendor = usr->RecentVendor.simplified();
  // Update Current Machine Info File with the Previous Working Machine Info File
   usr->CurrentModelFile = usr->RecentModelFile.simplified();
#endif

   // Change Manufacturer and Model as Machine changes -----------
   machineChange();

   int m=2;  // number of digits before decial point
   int n=1;  // number of digits after decial point
   // Norm Group Setting
   ui->floatSpinBoxRSD->setRange(0,10);
   // ui->floatSpinBoxRSD->setValidator(m,n);
   ui->floatSpinBoxRSD->setMaximum(30.0);
   ui->floatSpinBoxRSD->setMinimum(0.0);
   ui->floatSpinBoxRSD->setValue(2.0);
   // ui->floatSpinBoxRSD->setValidator(0);

   m=2; n=1;
   ui->floatSpinBoxOffset->setRange(0,10);
   // ui->floatSpinBoxOffset->setValidator(m,n);
   ui->floatSpinBoxOffset->setMaximum(10.0);
   ui->floatSpinBoxOffset->setMinimum(-10.0);
   ui->floatSpinBoxOffset->setValue(0.0);
   // ui->floatSpinBoxOffset->setValidator(0);

#ifdef REMOVED_Dec052006
   QString SMTH_FILE = mDir + "/smooth.inp";
   if (isThereFile(mDir, "smooth.inp")) readSMOOTH(SMTH_FILE);

   QString FIT_FILE = mDir + "/fit.inp";
   if (isThereFile(mDir, "fit.inp")) readFIT(FIT_FILE);
#endif
   // mName = ui->comboBoxMachine->currentText(); // REMOVED
    // Environment Variable Setting --------------------
   // Setting XVMC_HOME, XVMC_WORK and LD_LIBRARY_PATH
   setenv("XVMC_HOME", usr->LBIN.toStdString().c_str(),1);
   setenv("XVMC_WORK", (usr->LHOME+mName).toStdString().c_str(),1);
/*
   QString LD_LIB_PATH = getenv("LD_LIBRARY_PATH");
   if (!LD_LIB_PATH.contains(usr->LHOME))
      setenv("LD_LIBRARY_PATH", (usr->LHOME+"lib").toStdString().c_str(),1);
*/
   // MachineInfo -------------------------------------
   updateMachineInfo();
   initMachineInfo();
   // show();
   //     CMD = usr->LHOME + "bin/machineinfo.exe"
   //   + " -fname " + mName + ".info"
   //   + " -model " + mName
   //   + " -vendor " + mName
   //   + " -MDIR " + ui->lineEditTeleDir->text()
   //   + " -LLIB " + usr->LHOME + mName
   //   + " -LHOME " + usr->LHOME;
}
// -----------------------------------------------------------------------------
void MainConsole::initLocalSettings(QString group){
   // QString mName = ui->comboBoxMachine->currentText();
   QString mName = group;
   QString LHOME = usr->LHOME + mName;
   QTextStream (stdout) << "initLocalSettings:LHOME = " << LHOME << endl;
   // QTextStream (stdout) << "initLocalSettings:mName = " << mName << endl;
   // If the machine directory does not exist, make it.
   makeDir(LHOME);

   QDateTime currentDateTime = QDateTime::currentDateTime();
   QString DT = currentDateTime.toString();

#ifdef XML
   QDomDocument docType(mName+"XML");
   QDomElement root = docType.createElement(mName);
   docType.appendChild(root);

   Status s;  // Status class

   QString eName = "Machine";
   s.value = mName;  s.date = DT;
   root.appendChild(StatusToNode(docType, s, eName));

   QString mVendor = ui->comboBoxVendor->currentText();
   if (mVendor.length() == 0) mVendor = "Unknown";
   eName = "Vendor"; s.value = mVendor;
   root.appendChild(StatusToNode(docType, s, eName));

   QString mModel = ui->comboBoxModel->currentText();
   if (mModel.length() == 0) mModel = "Unknown";
   eName = "Model"; s.value = mModel;
   root.appendChild(StatusToNode(docType, s, eName));

   // usr->CurrentModelFile is determined when model is changed.
   usr->CurrentModelFile="Not Available";
   if (isThereFile(LHOME,mName+".info"))
       usr->CurrentModelFile = mName+".info";
   else
       QTextStream (stdout) << "initLocalSettings::No " << LHOME << "/" << mName+".info exists." << endl;

   eName = "AVALUE"; s.value = "9.0";
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "ZVALUE"; s.value = "0.5";
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "NUVALUE"; s.value = "0.45";
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "MachineModified"; s.value = "Modified";
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "Machine"; s.value = "Machine";
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "ModelFileModified"; s.value = "Modified";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "AirData"; s.value = "NoData";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "AFIT"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "AIROPT"; s.value = "0|0|0|1|0|0|3|1|1.0";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "MonoMC"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "WaterData"; s.value = "NoData";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "WFIT"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "Verification"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "PBVerification"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "PBMC"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "PBComm"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "Validate"; s.value = "NotDone";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "Lock"; s.value = "Unlocked";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "Energy"; s.value = "6.0";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "RSD"; s.value = "1";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "smallGridSize"; s.value = "1";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "largeGridSize"; s.value = "4";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "Offset"; s.value = "0";
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "RealFS"; s.value = "Off";
   root.appendChild(StatusToNode(docType, s, eName));
#ifdef XVMC
       eName = "FS1"; s.value = "100";
       if(mName.contains("EkBM")) s.value = "104";
#else
       eName = "FS1"; s.value = "10";
#endif
   root.appendChild(StatusToNode(docType, s, eName));
#ifdef XVMC
       eName = "FS2"; s.value = "None";
#else
       eName = "FS2"; s.value = "3";
       if(mName.contains("EkBM")) s.value = "32";
#endif
   root.appendChild(StatusToNode(docType, s, eName));
   eName = "OFAdjust"; s.value = "OFF|0.0";
   root.appendChild(StatusToNode(docType, s, eName));

   QFile xmlFile(LHOME+"/status.xml");
   xmlFile.open(QIODevice::WriteOnly);
   QTextStream txtStrm(&xmlFile);
   txtStrm << docType.toString();
   xmlFile.close();

#else
   QSettings settings;
   settings.insertSearchPath( QSettings::Unix, LHOME);
   settings.beginGroup("/"+group);
   settings.writeEntry("/"+group+"/Machine", mName);
   QString mVendor = ui->comboBoxVendor->currentText();
   if (mVendor.length() == 0) mVendor = "Unknown";
   settings.writeEntry("/"+group+"/Vendor", mVendor);
   QString mModel = ui->comboBoxModel->currentText();
   if (mModel.length() == 0) mModel = "Unknown";
   settings.writeEntry("/"+group+"/Model", mModel);
   // usr->CurrentModelFile is determined when model is changed.
   usr->CurrentModelFile="Not Available";
   if (isThereFile(LHOME,mName+".info")) usr->CurrentModelFile = mName+".info";
   settings.writeEntry("/"+group+"/ModelFile_DT", DT);
   settings.writeEntry("/"+group+"/AVALUE", "9.0");
   settings.writeEntry("/"+group+"/ZVALUE", "0.5");
   settings.writeEntry("/"+group+"/NUVALUE", "0.45");
#ifdef REMOVED_Jun162006
   settings.writeEntry("/"+group+"/VendorModified", "Modified");
   settings.writeEntry("/"+group+"/ModelModified", "NotModified");
#endif
   settings.writeEntry("/"+group+"/MachineModified", "Modified");
   settings.writeEntry("/"+group+"/ModelFileModified", "Modified");
   settings.writeEntry("/"+group+"/AirData", "NoData");
   settings.writeEntry("/"+group+"/AFIT", "NotDone");
   settings.writeEntry("/"+group+"/AIROPT", "0|0|0|1|0|0|3|1|1.0");
   settings.writeEntry("/"+group+"/WATEROPT", "0");
   settings.writeEntry("/"+group+"/OffAxis", "0");
   settings.writeEntry("/"+group+"/MonoMC", "NotDone");
   settings.writeEntry("/"+group+"/WaterData", "NoData");
   settings.writeEntry("/"+group+"/WFIT", "NotDone");
   settings.writeEntry("/"+group+"/Verification", "NotDone");
   settings.writeEntry("/"+group+"/PBMC", "NotDone");
   settings.writeEntry("/"+group+"/PBComm", "NotDone");
   settings.writeEntry("/"+group+"/Validate", "NotDone");
   settings.writeEntry("/"+group+"/Lock", "Unlocked");
   settings.endGroup();
   // std::cout << "End of writeSetting" << endl;
   // std::cout << "init:usr->CurrentModelFile=" << usr->CurrentModelFile << endl;
#endif

   QTextStream (stdout) << "initLocalSettings:END of " << mName << endl;
}
// -----------------------------------------------------------------------------
void MainConsole::writeLocalSetting (QString group, QString keyWord,
                                     QString keyValue){
   //QTextStream (stdout) << "writeLocalSetting:" << group
   //                     << "   " << keyWord  << " = " << keyValue << endl;

   QString mName = group;
   QString LHOME = usr->LHOME + mName;
   QDateTime currentDateTime = QDateTime::currentDateTime();
   QString DT = currentDateTime.toString();

#ifdef XML
   QFile xmlFile(LHOME+"/status.xml");
   // std::cout << LHOME+"/status.xml" << endl;
   if (!xmlFile.open(QIODevice::ReadOnly)){
      //QTextStream (stdout) <<  "writeLocalSetting: No " << LHOME+"/status.xml" << endl;
      initLocalSettings(mName);
      xmlFile.open(QIODevice::ReadOnly);
   }

   QDomDocument doc(mName);
   doc.setContent(&xmlFile);
   xmlFile.close();

   QDomElement root = doc.documentElement();
   if (root.tagName() != mName) {
      QTextStream (stdout) << "writeLocalSetting:Tag Name("<< root.tagName()
                           <<") is different from " << mName << endl;
      exit(-1);
   }

   QDomNode n = root.firstChild();

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
      Status s;  // Status class

      QString eName = keyWord;
      s.value = keyValue;  s.date = DT;
      root.appendChild(StatusToNode(doc, s, eName));
   }

   // QFile xmlFile(LHOME+"/status.xml");
   // std::cout << LHOME+"/status.xml" << endl;
   xmlFile.open(QIODevice::WriteOnly);
   QTextStream txtStrm(&xmlFile);
   txtStrm << doc.toString();
   xmlFile.close();

#else
   QString Entry = "/"+group+"/" + keyWord;
   QSettings settings;
   // std::cout << "writeLocalSetting: LHOME = " << LHOME << endl;
   // std::cout << "writeLocalSetting: group = " << group << endl;
   // std::cout << "writeLocalSetting: keyWord = " << keyWord << endl;
   // std::cout << "writeLocalSetting: keyValue = " << keyValue << endl;
   settings.insertSearchPath( QSettings::Unix, LHOME );
   settings.beginGroup("/"+group);
   settings.writeEntry(Entry, keyValue);
   settings.writeEntry(Entry+"_DT", DT); // Date and Time of Update
   settings.endGroup();
   // std::cout << "End of writeLocalSetting" << endl;
#endif
   //QTextStream (stdout) << "writeLocalSetting:" << group
   //                     << "   " << keyWord  << " = " << keyValue << " is written." << endl;
}
// -----------------------------------------------------------------------------
void MainConsole::updateLocalSettingRemoved(QString group) {
   // QString mName = ui->comboBoxMachine->currentText(); // REMOVED
   QString mName = group;
   writeLocalSetting(group,"/Machine",mName);
   writeLocalSetting(group,"/Model",mName);
   writeLocalSetting(group,"/Vendor",mName);
   QString LHOME = usr->LHOME+mName;
   if(isThereFile(LHOME,mName+".info"))
      writeLocalSetting(group,"/ModelFile",mName+".info");
   else
      writeLocalSetting(group,"/ModelFile",mName);
}
// -----------------------------------------------------------------------------
QString MainConsole::readLocalSetting(QString group, QString keyWord){
   //QTextStream(stdout) << "readLocalSetting:group = " << group << ", keyWord = " << keyWord << endl;
   // QString mName = ui->comboBoxMachine->currentText(); // REMOVED
   QString mName = group;
   QString LHOME = usr->LHOME + mName;
   QString keyValue = "";

#ifdef XML
   QFile xmlFile(LHOME+"/status.xml");
   //QTextStream(stdout) << "readLocalSetting:" << LHOME+"/status.xml" << endl;
   if (!xmlFile.open(QIODevice::ReadOnly)){
      //QTextStream(stdout)  << "readLocalSetting: No " << LHOME+"/status.xml exists." << endl;
      initLocalSettings(mName);
      xmlFile.open(QIODevice::ReadOnly);
   }

   QDomDocument docType(mName);
   docType.setContent(&xmlFile);
   xmlFile.close();

   QDomElement root = docType.documentElement();
   // QTextStream(stdout)  << "readLocalSetting:root.tagName()=" << root.tagName() << endl;
   if (root.tagName() != mName) {
      QTextStream(stdout)  << "readLocalSetting:Tag Name ("<<root.tagName()<<") is different from " << mName << endl;
      exit(-1);
   }

   QDomNode n = root.firstChild();

   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         //QTextStream(stdout)  << e.tagName() << " -> " << e.attribute("value", "") << endl;
         if(e.tagName() == keyWord) keyValue = e.attribute("value", "");
      }
      n = n.nextSibling();
   }
#else
   QString Entry = group+"/" + keyWord;
   QSettings settings;
   //std::cout << "readLocalSetting: LHOME = " << LHOME << endl;
   //std::cout << "readLocalSetting: group = " << group << endl;
   //std::cout << "readLocalSetting: keyWord = " << keyWord << endl;
   settings.insertSearchPath( QSettings::Unix, LHOME);
   settings.beginGroup("/"+group);
   keyValue = settings.readEntry(Entry, "");
   //std::cout << "readLocalSetting: keyValue = " << keyValue << endl;
   settings.endGroup();
#endif
   //QTextStream(stdout)  << "readLocalSetting:" << keyWord << " = " << keyValue << endl;
   return (keyValue.simplified());
}
// -----------------------------------------------------------------------------
void MainConsole::writeSettings(QString group){
   QDateTime currentDateTime = QDateTime::currentDateTime();
   QString DT = currentDateTime.toString();
   // QDate date = QDate::currentDate();
   // QString DT = date.toString("MM.dd.yyyy");
   QString LPATH = getenv("USERNAME");
   // std::cout << "LPATH = " << LPATH << endl;
#ifdef XML
   QString mName="XVMC";
   QDomDocument docType("MonacoPath");
   QDomElement root = docType.createElement(mName);
   docType.appendChild(root);

   Status s;  // Status class

   QString eName = "FOCUS";
   s.value = usr->FOCUS;  s.date = DT;
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "LHOME"; s.value = usr->LHOME;  s.date = DT;
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "LBIN"; s.value = usr->LBIN;  s.date = DT;
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "TELE"; s.value = usr->TELE_DIR;  s.date = DT;
   root.appendChild(StatusToNode(docType, s, eName));

   eName = "ModelFile"; s.value = usr->CurrentModelFile;  s.date = DT;
   root.appendChild(StatusToNode(docType, s, eName));

   QString HOME=getenv("HOME");
   QFile xmlFile(HOME+"/.monacobct");
   xmlFile.open(QIODevice::WriteOnly);
   QTextStream txtStrm(&xmlFile);
   txtStrm << docType.toString();
   xmlFile.close();

#else
   QSettings settings;
   settings.setPath("localhost", group);
   // std::cout << "group = " << group << endl;
   // std::cout << "writeSettings: mName = " << comboBoxMachine->currentText() << endl;;
   // std::cout << "writeSettings: PWD = " << getenv("PWD") << endl;;
   settings.beginGroup("/"+group);
   settings.writeEntry("/"+group+"/"+LPATH+"/FOCUS", usr->FOCUS);
   settings.writeEntry("/"+group+"/"+LPATH+"/LHOME", usr->LHOME);
   settings.writeEntry("/"+group+"/"+LPATH+"/LBIN", usr->LBIN);
   settings.writeEntry("/"+group+"/"+LPATH+"/TELE", usr->TELE_DIR);

#ifdef REMOVED_Jun162006
   QString mName = ui->comboBoxMachine->currentText();
   settings.writeEntry("/"+group+"/"+LPATH+"/Machine", mName);
   QString mVendor = ui->comboBoxVendor->currentText();
   settings.writeEntry("/"+group+"/"+LPATH+"/Vendor", mVendor);
   QString mModel = ui->comboBoxModel->currentText();
   settings.writeEntry("/"+group+"/"+LPATH+"/Model", mModel);
#endif

   settings.writeEntry("/"+group+"/"+LPATH+"/ModelFile", usr->CurrentModelFile);
   settings.writeEntry("/"+group+"/"+LPATH+"/DATE_TIME", DT);
   settings.endGroup();
   // std::cout << "End of writeSetting" << endl;
#endif
}
// -----------------------------------------------------------------------------
void MainConsole::readSettings(QString group){
#ifdef XML

   QString HOME=getenv("HOME");
   std::cout << "xmlFile = " << HOME.toStdString()+"/.monacobct" << endl;
   QFile xmlFile(HOME+"/.monacobct");
   xmlFile.open(QIODevice::ReadOnly);

   QDomDocument docType("MonacoPath");
   docType.setContent(&xmlFile);
   xmlFile.close();

   QString mName="XVMC";
   QDomElement root = docType.documentElement();
   if (root.tagName() != mName) {
      std::cout << "Tag Name ("<<root.tagName().toStdString()<<") is different from " << mName.toStdString() << endl;
      exit(-1);
   }

   QDomNode n = root.firstChild();

   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         if(e.tagName() == "FOCUS")usr->FOCUS = e.attribute("value", "");
         if(e.tagName() == "LHOME")usr->LHOME = e.attribute("value", "");
         if(e.tagName() == "LBIN") usr->LBIN = e.attribute("value", "");
         if(e.tagName() == "TELE") usr->TELE_DIR = e.attribute("value", "");
         /*
         if(e.tagName() == mName){
        Status s;
        s.name = e.attribute("name", "");
        s.value = e.attribute("value", "");
        s.date = e.attribute("date", "");

        if(s.name == "FOCUS") usr->FOCUS = s.value;
        if(s.name == "LHOME") usr->LHOME = s.value;
        if(s.name == "LBIN")  usr->LBIN = s.value;
        if(s.name == "TELE")  usr->TELE_DIR = s.value;
     }
     */
      }
      n = n.nextSibling();
   }

#else
   QSettings settings;
   // settings.setPath("localhost", group);
   settings.insertSearchPath( QSettings::Unix, "/usr/local/Work/MonacoBCT");
   QString LPATH = getenv("USERNAME");
   settings.beginGroup("/"+group);
   usr->FOCUS = settings.readEntry("/"+group+"/"+LPATH+"/FOCUS", "/FOCUS");
   usr->LHOME = settings.readEntry("/"+group+"/"+LPATH+"/LHOME", usr->PWD);
   usr->LBIN = settings.readEntry("/"+group+"/"+LPATH+"/LBIN", usr->PWD);
   usr->TELE_DIR = settings.readEntry("/"+group+"/"+LPATH+"/TELE", usr->PWD);
#ifdef REMOVED_Jun162006
   usr->RecentMachine = settings.readEntry("/"+group+"/"+LPATH+"/Machine", "")
   usr->RecentVendor = settings.readEntry("/"+group+"/"+LPATH+"/Vendor", "");
   usr->RecentModel = settings.readEntry("/"+group+"/"+LPATH+"/Model", "");
   usr->RecentModelFile = settings.readEntry("/"+group+"/"+LPATH+"/ModelFile",
               "Not Available");
#endif
   settings.endGroup();
#endif
}  // End of void MainConsole::readSettings(QString group)
// -----------------------------------------------------------------------------
#ifdef REMOVED111607
QString MainConsole::readSettingString(QString keyWord){
   QString HOSTNAME = getenv("HOSTNAME");
   QString gName = HOSTNAME.section('.',0,0); // Group Name
   QString Entry = "/"+gName+"/" + keyWord;
   QSettings settings;
   settings.setPath("localhost", gName);
   settings.beginGroup("/"+gName);
   QString keyValue = settings.readEntry(Entry, "");
   settings.endGroup();
   return (keyValue);
}
// -----------------------------------------------------------------------------
void MainConsole::writeSettingString(QString keyWord, QString keyValue){
 QString HOSTNAME = getenv("HOSTNAME");
   QString gName = HOSTNAME.section('.',0,0); // Group Name

   QString Entry = "/"+gName+"/" + keyWord;
   QSettings settings;
   settings.setPath("localhost", gName);
   settings.beginGroup("/"+gName);
   settings.writeEntry(Entry, keyValue);
   settings.endGroup();
   // std::cout << "End of writeSetting" << endl;
}
#endif
// -----------------------------------------------------------------------------
void MainConsole::getTeleDir() {
    // Diaglog for choosing FOCUS directory
   usr->FOCUS = getenv ("FOCUS");
   QString teleDir = QFileDialog::getExistingDirectory(
               this,  "Choose a directory", usr->FOCUS,
               QFileDialog::ShowDirsOnly |
               QFileDialog::DontResolveSymlinks);

   ui->lineEditTeleDir->setText(teleDir);
   usr->TELE_DIR = teleDir;
    // Gets machines
   if (ui->lineEditLocalDir->text().length() > 0) getMCMachine();
}
// -----------------------------------------------------------------------------
void MainConsole::getLocalDir() {
    // Dialog for choosing Local Home (LHOME) directory
   usr->LHOME = getenv ("HOME");
   QString localDir = QFileDialog::getExistingDirectory(
               this,  "Choose a directory", usr->LHOME,
               QFileDialog::ShowDirsOnly |
               QFileDialog::DontResolveSymlinks);

   ui->lineEditLocalDir->setText(localDir);
   usr->LHOME = localDir;
}
// -----------------------------------------------------------------------------
QString MainConsole::pickDir(QString dirFrom, QString messageText) {
    // Pick a directory using Dialog and Browsing
   QString thisDir = QFileDialog::getExistingDirectory(
               this,  messageText, dirFrom,
               QFileDialog::ShowDirsOnly |
               QFileDialog::DontResolveSymlinks);

   return(thisDir);
}
// -----------------------------------------------------------------------------
QString MainConsole::mm2cm(QString mm) {
  // Converts string millimeter to string centimeter
  bool ok;
  float value = mm.toFloat(&ok)/10.0;
  mm.sprintf("%7.2f",value);
  mm.simplified();
  return(mm);
}
/* REMOVED
// -----------------------------------------------------------------------------
QString MainConsole::getParm(QString keyWord) {
  // Gets parameters for XiO parm file
  QString result = "";
  QString mName = ui->comboBoxMachine->currentText();
  QString pDir = ui->lineEditTeleDir->text() + "/" + mName + "/parm" ;
  // std::cout << pName << endl;
  // std::cout << pDir << endl;
  QFile pFile( pDir );
  if (pFile.exists()) {
     QTextStream stream( &pFile );
     QString sLine;
     pFile.open( QIODevice::ReadOnly );

     // File Version Number
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("version")) QTextStream(&sLine) >> result;

     // Description
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("model")) QTextStream(&sLine) >> result;

     // particle Charge
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("charge")) QTextStream(&sLine) >> result;
     if (keyWord.toLower().contains("particle")) QTextStream(&sLine) >> result;

     // Machine Year
     sLine = stream.readLine().simplified();

     // Machine Manufacturer
     sLine = stream.readLine().simplified();

     // Machine Model
     sLine = stream.readLine().simplified();

     // Target to Flattening Filter Distance
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("sfd")) {
        QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Nominal Energy
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("energy")) {
        QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Reference Depth
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("refdepth")) {
      QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Reference Distance (SCD/SAD)
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("refdist")) {
      QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Horizontal Scan Reference Position
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("hscanref")) {
      QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Vertical Scan Reference Position
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("vscanref")) {
      QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Maximum Field Width
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("maxfw")) {
      QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Maximum Field Length
     sLine = stream.readLine().simplified();
     if (keyWord.toLower().contains("maxfl")) {
      QTextStream(&sLine) >> result;
        result = mm2cm(result);
     }

     // Mimimum Field Width
     sLine = stream.readLine().simplified();

     // Mimimum Field Length
     sLine = stream.readLine().simplified();

     pFile.close();
  }
  return result;
}  // End of MainConsole::getParm(QString keyWord)
*/
// -----------------------------------------------------------------------------
void MainConsole::readParm() {
  QString mName = ui->comboBoxMachine->currentText();
  QString pDir = ui->lineEditTeleDir->text() + "/" + mName + "/parm" ;
  // std::cout << pName << endl;
  // std::cout << pDir << endl;
  QFile pFile( pDir );
  if (pFile.exists()) {
     QTextStream stream( &pFile );
     QString sLine;
     pFile.open( QIODevice::ReadOnly );

     // File Version Number
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->VERSION;

     // Description
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->MODEL;

     // particle Charge
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->CHARGE;
      if(usr->CHARGE == "0") usr->particle = "Photon";
      // std::cout << "usr->CHARGE = " << usr->CHARGE << endl;
      // std::cout << "usr->particle = " << usr->particle << endl;

     // Machine Year
     sLine = stream.readLine().simplified();

     // Machine Manufacturer
     sLine = stream.readLine().simplified();

     // Machine Model
     sLine = stream.readLine().simplified();

     // Target to Flattening Filter Distance
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->SFD;
     usr->SFD = mm2cm(usr->SFD);

     // Nominal Energy
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->E0;

     // Reference Depth
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->REFDEPTH;
     usr->REFDEPTH = mm2cm(usr->REFDEPTH);
      // std::cout << "REFDEPTH = " << usr->REFDEPTH << endl;

     // Reference Distance (SCD/SAD)
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->REFDIST;
     usr->REFDIST = mm2cm(usr->REFDIST);

     // Horizontal Scan Reference Position
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->HSCANREF;
     usr->HSCANREF = mm2cm(usr->HSCANREF);

     // Vertical Scan Reference Position
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->VSCANREF;
     usr->VSCANREF = mm2cm(usr->VSCANREF);

     // Maximum Field Width
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->MAXFW;
     usr->MAXFW = mm2cm(usr->MAXFW);

     // Maximum Field Length
     sLine = stream.readLine().simplified();
     QTextStream(&sLine) >> usr->MAXFL;
     usr->MAXFL = mm2cm(usr->MAXFL);

     // Mimimum Field Width
     sLine = stream.readLine().simplified();

     // Mimimum Field Length
     sLine = stream.readLine().simplified();

      // ADDED
      bool ok;
      float maxFW = usr->MAXFW.toFloat(&ok);
      float maxFL = usr->MAXFL.toFloat(&ok);
      float maxFS = maxFW;
      if (maxFL > maxFS) maxFS = maxFL;
      float ZS = usr->SFD.toFloat(&ok);
      float FFRad = maxFS/2.0*sqrt(2.0)*ZS/100.0;
      usr->FFRad.sprintf("%6.3f",FFRad);

     pFile.close();

  }
}  // End of void MainConsole::readParm()
// -----------------------------------------------------------------------------
void MainConsole::getMCMachine() {
   // std::cout << "getMCMachine:Recentmachine = " << usr->RecentMachine << endl;
   ui->comboBoxMachine->clear();  // clear the machine list
   ui->comboBoxOrigMachine->clear();  // clear the machine list
   QDir *dirMachine = new QDir;
   dirMachine->setPath(ui->lineEditTeleDir->text());
   // QTextStream (stdout) << "ui->lineEditTeleDir->text()" << ui->lineEditTeleDir->text() << endl;
   dirMachine->setNameFilters(QStringList("*"));
   QStringList mList = dirMachine->entryList(QDir::Dirs, QDir::Name);
   // QTextStream (stdout) << "mList = " << mList.join('|') << endl;
   for (QStringList::Iterator it = mList.begin(); it != mList.end(); ++it){
      QString pName = *it;
      //QTextStream (stdout) << "getMCMachine:pName = " << pName << endl;
      if (!pName.startsWith(".")) {
         QString pDir = ui->lineEditTeleDir->text() + "/" + pName + "/parm" ;
         // std::cout << "pName = " << pName.toStdString() << endl;
         // std::cout << "pDir = " << pDir.toStdString() << endl;
         QFile pFile( pDir );
         if (pFile.exists()) {
    /*  // REMOVED
              QTextStream stream( &pFile );
              QString sLine;
              pFile.open( QIODevice::ReadOnly );
              // skip 5 lines and set 6th line to sLine
              for (int ii=0;ii<6;ii++) sLine = stream.readLine();
              // std::cout << sLine.toLatin1() << endl;
              int i;
              QTextStream(&sLine) >> i;
              if (i == 0) {
                 QString aDir = ui->lineEditTeleDir->text() + *it + "/meas/in_air";
                 // std::cout << "aDir = " << aDir << endl;
                 QFile aFile( aDir );
                 // If the in_air directory exists, this machine must include MC data.
                 if (aFile.exists()) comboBoxMachine->insertItems(-1,pName);
                 // Set to the previous machine
                 // if (mName == usr->RecentMachine) comboBoxMachine->setCurrentText(mName);
              }
              pFile.close();
    */
            QTextStream stream( &pFile );
            QString sLine;
            pFile.open( QIODevice::ReadOnly );

           // File Version Number
            sLine = stream.readLine().simplified();
           // QTextStream(&sLine) >> usr->VERSION;  // REMOVED

           // Description
            sLine = stream.readLine().simplified();
           // QTextStream(&sLine) >> usr->MODEL;  // REMOVED

           // particle Charge
            sLine = stream.readLine().simplified();
           // QTextStream(&sLine) >> usr->CHARGE;  //REMOVED

           // Machine Year
            sLine = stream.readLine().simplified();

           // Machine Manufacturer
            sLine = stream.readLine().simplified();

           // Machine Model
            sLine = stream.readLine().simplified();

           int iModel;
            QTextStream(&sLine) >> iModel;
            if (iModel == 0) {
               QString aDir = ui->lineEditTeleDir->text() + pName + "/meas/in_air";
               // std::cout << "aDir = " << aDir << endl;
               QFile aFile( aDir );
               // If the in_air directory exists, this machine must include MC data.
               if (aFile.exists()) ui->comboBoxMachine->addItem(pName);
               if (aFile.exists()) ui->comboBoxOrigMachine->addItem(pName);
               // Set to the previous machine
               // if (mName == usr->RecentMachine) comboBoxMachine->setCurrentText(mName);
            }
            /*  // REMOVED
           // Target to Flattening Filter Distance
            sLine = stream.readLine().simplified();
            QTextStream(&sLine) >> usr->SFD;
            usr->SFD = mm2cm(usr->SFD);

           // Nominal Energy
            sLine = stream.readLine().simplified();
           QTextStream(&sLine) >> usr->E0;

           // Reference Depth
            sLine = stream.readLine().simplified();
           QTextStream(&sLine) >> usr->REFDEPTH;
            usr->REFDEPTH = mm2cm(usr->REFDEPTH);

           // Reference Distance (SCD/SAD)
            sLine = stream.readLine().simplified();
           QTextStream(&sLine) >> usr->REFDIST;
            usr->REFDIST = mm2cm(usr->REFDIST);

           // Horizontal Scan Reference Position
            sLine = stream.readLine().simplified();
           QTextStream(&sLine) >> usr->HSCANREF;
            usr->HSCANREF = mm2cm(usr->HSCANREF);

           // Vertical Scan Reference Position
            sLine = stream.readLine().simplified();
           QTextStream(&sLine) >> usr->VSCANREF;
            usr->VSCANREF = mm2cm(usr->VSCANREF);

           // Maximum Field Width
            sLine = stream.readLine().simplified();
           QTextStream(&sLine) >> usr->MAXFW;
            usr->MAXFW = mm2cm(usr->MAXFW);

           // Maximum Field Length
            sLine = stream.readLine().simplified();
           QTextStream(&sLine) >> usr->MAXFL;
            usr->MAXFL = mm2cm(usr->MAXFL);

           // Mimimum Field Width
            sLine = stream.readLine().simplified();

           // Mimimum Field Length
            sLine = stream.readLine().simplified();
            */
            pFile.close();

         }
      }
   }
   // Set to the previous machine
#ifdef REMOVED_Jun162006
   if (usr->RecentMachine == "")
      comboBoxMachine->setCurrentIndex(0);
   else
      for (int i=0; i<comboBoxMachine->count(); i++) {
         if (comboBoxMachine->text(i) == usr->RecentMachine)
            comboBoxMachine->setCurrentText(usr->RecentMachine);
      }
#else
   for (int i=0; i<ui->comboBoxMachine->count(); i++) {
       QString tmpName = ui->comboBoxMachine->itemText(i);
       //QTextStream (stdout) << i << "  getMCMachine:tmpName = " << tmpName << endl;
   }

    for (int i=0; i<ui->comboBoxMachine->count(); i++) {
        QString tmpName = ui->comboBoxMachine->itemText(i);
        //QTextStream (stdout) << "getMCMachine:tmpName = " << tmpName << endl;
        if (readLocalSetting(tmpName,"Lock") == "Unlocked") {
            ui->comboBoxMachine->setCurrentIndex(i);
            i = ui->comboBoxMachine->count();
        }
    }
#endif
} // End of void MainConsole::getMCMachine()
// -----------------------------------------------------------------------------
void MainConsole::machineChange() {
   // Machine File Initialization and Modification
   // Current Machine Name
   QString mName = ui->comboBoxMachine->currentText();

   if (usr->CurrentMachine.length() > 0 && usr->CurrentMachine != mName){
       if (readLocalSetting(usr->CurrentMachine,"Lock") == "Locked")
       writeLocalSetting(usr->CurrentMachine,"Lock","Unlocked");
   }

   // Check if a Local Machine directory exists. Otherwise create mName directory
   QString mDir = usr->LHOME+mName;
   // std::cout << "machineChange:mDir = " << mDir << endl;
   if (!isThereDir(mDir)) {
      // std::cout << "machineChange: Call initLocalSettings(mName) " << endl;
      initLocalSettings(mName);
   }
   writeLocalSetting(mName,"Machine",mName); // initialize local setting
   // Chech if there is afit diretory under usr->LHOME/usr->CurrentMachine directory
   // QString AFIT_DIR = mDir + "/afit";   // REMOVED
   // usr->afitDirExists = isThereDir(AFIT_DIR);  // REMOVED
   // makeDir(AFIT_DIR);  //REMOVED

   // Check if there is afit/data directory. If yes, activate AFIT group box.
   // QString AFIT_DIR_DATA = mDir + "/afit/data";  // REMOVED
   // makeDir(AFIT_DIR_DATA);  //REMOVED
   // if (isThereDir(AFIT_DIR_DATA)) ui->groupBoxAFIT->setEnabled(true);  //REMOVED

   // Initially All Group Boxes Disabled
   ui->groupBoxAFIT->setEnabled(false);
   ui->groupBoxMonoMC->setEnabled(false);
   ui->groupBoxWFIT->setEnabled(false);
   ui->groupBoxVerification->setEnabled(false);
   ui->groupBoxMCPB->setEnabled(false);
   ui->groupBoxPBComm->setEnabled(false);

    clearBdtValues(); // Clear BDT Values

   // Read Previous Local Settings
   QFile rcFile(mDir + "/" + mName.toLower() + "rc");
   if (rcFile.exists()) {
#ifdef REMOVED_Jun162006
      usr->CurrentMachine = readLocalSetting(mName,"Machine");
      usr->CurrentModel = readLocalSetting(mName,"Model");
      usr->CurrentVendor = readLocalSetting(mName,"Vendor");
      usr->CurrentModelFile = readLocalSetting(mName,"ModelFile");
#endif
   } else {
#ifndef XML
      //std::cout << "machineChange: Call initLocalSettings(mName) " << endl;
      initLocalSettings(mName);
#endif
   }

   if (readLocalSetting(mName,"Lock") == "Unlocked") {
       writeLocalSetting(mName,"Lock","Locked");
       usr->CurrentMachine = mName;  // Inserted June 19, 2006
   } else if (readLocalSetting(mName,"Lock") == "Locked") {
     int iAns = 2;
     iAns = QMessageBox::critical(this,
               "LOCKED Machine",
               mName + " may be in use, Do you want to unlock?",
               QMessageBox::Yes,
               QMessageBox::No,
               QMessageBox::NoButton);
     //std::cout <<"machineChange: iAns = " << iAns << endl;
     switch (iAns) {
        case QMessageBox::Yes:
            //std::cout << "machineChange: ANS = Yes" << endl;
            writeLocalSetting(mName,"Lock","Unlocked");
            break;
        case QMessageBox::No:
       //std::cout << "machineChange: ANS = No" << endl;
           for (int i=0; i<ui->comboBoxMachine->count(); i++) {
               QString tmpName = ui->comboBoxMachine->itemText(i);
               if (readLocalSetting(tmpName,"Lock") == "Unlocked") {
                   ui->comboBoxMachine->setCurrentIndex(i);
                   mName = tmpName;
                   writeLocalSetting(mName,"Lock","Locked");
                   usr->CurrentMachine = mName;
                   i = ui->comboBoxMachine->count();
               }
           }
           break;
        default:
             break;
     }
   }

   // Update Current Machine with the Previous Working Machine
#ifdef REMOVED_Jun162006
   if (usr->CurrentMachine != mName) {
      usr->CurrentMachine = mName;
      writeLocalSetting(mName,"MachineModified","Modified");
   } else {
      writeLocalSetting(mName,"MachineModified","NotModified");
   }

   // Update Current Model with the Previous Working Model
   if (usr->RecentModel != usr->CurrentModel)
      usr->CurrentModel = usr->RecentModel;
   getModels();
   // Update Current Vendor with the Previous Working Vendor
   if (usr->RecentVendor != usr->CurrentVendor)
      usr->CurrentVendor = usr->RecentVendor;
   getVendors();
#endif

   // Read XiO Parm
   readParm();
   writeLocalSetting(mName,"Energy",usr->E0);
   // Machine Parameter File
   QString mInfoFile = mName +".info";
   //std::cout << "Machine set to " + usr->CurrentMachine << endl;
   if (isThereFile(mDir, mInfoFile)) {
      QString modelFile = mDir + "/" + mInfoFile;
      getMachineInfo(modelFile);
      usr->comboBoxModelFile = mInfoFile;
      usr->CurrentModelFile = mInfoFile;  // Ignore the ModelFile in rc file
   }
#ifdef REMOVED_Jun162006
   if (isThereFile(mDir, mInfoFile)) { // Read existing info file (Why???)
      QString modelFile = mDir + "/" + mInfoFile;
      getMachineInfo(modelFile);
      if (usr->MODEL.length() > 0) usr->CurrentModel=usr->MODEL;
      // std::cout << "machineChange:MODEL= " << usr->CurrentModel << endl;
      comboBoxModel->setCurrentText(usr->CurrentModel);
      if (usr->VENDOR == "") usr->VENDOR = "Unknown";
      usr->CurrentVendor=usr->VENDOR;
      //std::cout << "machineChange:VENDOR= " << usr->CurrentVendor << endl;
      comboBoxVendor->setCurrentText(usr->CurrentVendor);
      usr->CurrentMachine = mName;
   }
#endif
   setenv("XVMC_WORK", (usr->LHOME+mName).toStdString().c_str(),1);  // For XVMC
   // FS1 and FS2 Update
   for (int i=0; i<ui->comboBox1stFS->count(); i++) {
      ui->comboBox1stFS->removeItem(i);
   }
   for (int i=0; i<ui->comboBox2ndFS->count(); i++) {
#ifdef XVMC
      ui->comboBox2ndFS->removeItem(0);
#else
      ui->comboBox2ndFS->removeItem(i);
#endif
   }

   readAirOpt();
   readRSD();
   readNU();
   readOffset();
   readMFS();
   readGridSizeVer();
   readWaterOpt();
   readOffAxisOpt();
   readOFAdjust();

   QString SMTH_FILE = mDir + "/smooth.inp";
   if (isThereFile(mDir, "smooth.inp")) readSMOOTH(SMTH_FILE);

   QString FIT_FILE = mDir + "/fit.inp";
   if (isThereFile(mDir, "fit.inp")) readFIT(FIT_FILE);

   //QTextStream (stdout) << "machineChange: call updateButtons()" << endl;
   updateButtons();
   //QTextStream (stdout) << "machineChange: call updateMachineInfo()" << endl;
   updateMachineInfo();
   //QTextStream (stdout) << "machineChange: call updateAirFitValues()" << endl;
   updateAirFitValues();
   //QTextStream (stdout) << "machineChange: call updateWaterFitValues()" << endl;
   updateWaterFitValues();

   // Update Existing BDT Information on GUI
   QString XVMC_WORK = usr->LHOME + mName;
   QString BDT_DIR = XVMC_WORK + "/dat/basedata";
   QString BDT_FILE = XVMC_WORK + "/dat/" + mName + ".bdt";
   QFile bdtFile(BDT_FILE);
   if (bdtFile.exists()) {
        readBDT(BDT_FILE);
        updateBdtValues();
        // std::cout << "usr->eEnergy = " << usr->eEnergy << endl;
        ui->groupBoxVerification->setEnabled(true);
   }
   else {
        ui->groupBoxVerification->setEnabled(false);
   }
   updatePBCommInfo();
   updatePBInfo();

   ui->textLabelTitle->setText("MC and PB Modeling for " + mName);

}  // End of void MainConsole::machineChange()
// -----------------------------------------------------------------------------
void MainConsole::getVendors() {
#ifdef REMOVED_Jun162006
   QStringList vList;
   vList << "Unknown" << "Elekta" << "Mitsubishi" << "Siemens"
         << "Varian";
   // comboBoxVendor->clear();
   // comboBoxVendor->addItems(vList);
   for ( QStringList::Iterator it = vList.begin(); it != vList.end(); ++it ){
      QString vName = *it;
      if (vName.toLower() == usr->CurrentVendor.toLower()) {
         comboBoxVendor->setCurrentText(vName);
      }
   }
#endif
}  // End of void MainConsole::getVendors()
// -----------------------------------------------------------------------------
void MainConsole::getModels() {
   // std::cout << "getModels:usr->CurrentModel=" << usr->CurrentModel << endl;
   // get Model list from Library
   ui->comboBoxModel->clear();
   ui->comboBoxModel->addItem("Unknown");
   QDir *dirLib = new QDir;
   QString LLIB = usr->LHOME + "lib";
   // QTextStream (stdout) << "getModels: LLIB = " << LLIB << endl;
   dirLib->setPath(LLIB);
   QString fileFilter = ui->comboBoxVendor->currentText() + ".*";
   // QTextStream (stdout) << "getModels: fileFilter = " << fileFilter << endl;
   dirLib->setNameFilters(QStringList(fileFilter));
   QString preText("");
   QString curText("");
   QStringList mList = dirLib->entryList(QDir::Files, QDir::Name); // Model List
   mList.sort();
   bool isUsrModel = false;
   for ( QStringList::Iterator it = mList.begin(); it != mList.end(); ++it ){
      QString mFileName = LLIB + "/" + *it;
      // QTextStream (stdout) << "getModels: mFileName = " << mFileName << endl;
      QFile mFile( mFileName );
      if (mFile.exists()) {
         QTextStream stream( &mFile );
         QString sLine;
         mFile.open( QIODevice::ReadOnly );
         while ( !stream.atEnd() ) {
            sLine = stream.readLine();
            QString strLine = sLine.toLatin1();
            if (strLine.count("MODEL",Qt::CaseSensitive) > 0){
               // QTextStream (stdout) << "getModels: strLine = " << strLine.count("MODEL",Qt::CaseSensitive) << " " << strLine << endl;
               curText = strLine.section('#',0,0).section('=',1,1);
               curText = curText.simplified();
            }
         }
         mFile.close();
         if (curText != preText) {
            preText = curText;
            ui->comboBoxModel->addItem(curText);
            if (curText == usr->CurrentModel) {  // Set to the previous working model
               // std::cout << "getModels Yes: curText=" << curText << "   preText=" << preText << endl;
               ui->comboBoxModel->setCurrentText(curText);
               // if (isThereFile(usr->LHOME+"/"+comboBoxMachine->currentText(),
               //       comboBoxMachine->currentText()+".info"))
               // usr->comboBoxModelFile = usr->CurrentModelFile;
               usr->comboBoxModelFile = *it;
               isUsrModel = true; // Yes, therse is a user machine parameter file.
               //std::cout << "getModels Yes: usr->comboBoxModelFile =" << usr->comboBoxModelFile << endl;
               // std::cout << "getModels Yes: usr->CurrentModelFile =" << usr->CurrentModelFile << endl;
            } else {
            //  std::cout << "getModels No: curText=" << curText << "   preText=" << preText << endl;
            // usr->comboBoxModelFile = *it;  // Set to new machine info file instead of ComboBox
            }
            if (!isUsrModel) {  // No corresponding model to current vendor
               if (isThereFile(usr->LHOME+ui->comboBoxMachine->currentText(),
                     ui->comboBoxMachine->currentText()+".info"))
                  usr->comboBoxModelFile = usr->CurrentModelFile;
               else
                  usr->comboBoxModelFile = *it;  // Set to new machine infor file instead of ComboBox
               ui->comboBoxModel->setCurrentText(curText);
            }
         }
      }
   // writeLocalSetting(comboBoxMachine->currentText(),
   //      "ModelFile", usr->CurrentModelFile);
   }
   //std::cout << "getModels: usr->comboBoxModelFile =" << usr->comboBoxModelFile << endl;
   //  std::cout << "getModels: usr->CurrentModelFile =" << usr->CurrentModelFile << endl;

}  // End of void MainConsole::getModels()
// -----------------------------------------------------------------------------
void MainConsole::setModelFile() {  // Find model file name to the selected model
   QDir *dirLib = new QDir;
   QString LLIB = usr->LHOME + "lib";
   dirLib->setPath(LLIB);
   QString fileFilter = ui->comboBoxVendor->currentText() + ".*";
   QTextStream (stdout) << "setModelFile: fileFilter = " << fileFilter << endl;
   dirLib->setNameFilters(QStringList(fileFilter));
   QString preText("");
   QString curText("");
   QStringList mList = dirLib->entryList(QDir::Files, QDir::Name); // Model List
   mList.sort();
   // QTextStream (stdout) << "setModelFile: mList = " << mList.length() << endl;
   for ( QStringList::Iterator it = mList.begin(); it != mList.end(); ++it ){
      QString mFileName = LLIB + "/" + *it;
      QFile mFile( mFileName );
      if (mFile.exists()) {
         QTextStream stream( &mFile );
         QString sLine;
         mFile.open( QIODevice::ReadOnly);
         while ( !stream.atEnd() ) {
            sLine = stream.readLine();
            QString strLine = sLine.toLatin1();
            if (strLine.count("MODEL", Qt::CaseSensitive) != -1) {
            // if (strLine.find("MODEL",0,true) != -1) {
               curText = strLine.section('#',0,0).section('=',1,1);
               curText = curText.simplified();
            }
         }
         mFile.close();

         if (curText != preText) {
            preText = curText;
            if (curText == ui->comboBoxModel->currentText()) {  // set to the previous setting
               usr->comboBoxModelFile = *it; // Set to new machine infor file instead of ComboBox
            }
         }
      }
   }
   QTextStream (stdout) << "setModelFile: ui->comboBoxModel->currentText() =" << ui->comboBoxModel->currentText() << endl;
   QTextStream (stdout) << "setModelFile: usr->comboBoxModelFile =" << usr->comboBoxModelFile << endl;
}  // End of void MainConsole::getModels()
// -----------------------------------------------------------------------------
void MainConsole::done() {
#ifdef REMOVED_Jun162006
   updateLocalSetting(comboBoxMachine->currentText());
   QString HOSTNAME = getenv("HOSTNAME");
   QString gName = HOSTNAME.section('.',0,0);
   writeSettings(gName);
#endif
   QString mName = ui->comboBoxMachine->currentText();
   if (usr->CurrentMachine == mName &&
   readLocalSetting(mName,"Lock") == "Locked") {
       writeLocalSetting(mName,"Lock","Unlocked");
   std::cout << "Normal Terminated (done) and " << mName.toStdString() << " is Unlocked for Now" << endl;
   }
   writeRSD();
   writeNU();
   writeOffset();
   writeMFS();
   close();
} // End of void MainConsole::done()
// -----------------------------------------------------------------------------
void MainConsole::cancel() {
   QString mName = ui->comboBoxMachine->currentText();
   if (readLocalSetting(mName,"Lock") == "Locked") {
       writeLocalSetting(mName,"Lock","Unlocked");
   std::cout << "Normal Terminated (cancel) and " << mName.toStdString() << " is Unlocked for Now" << endl;
   }
   close();
} // End of void MainConsole::done()
// -----------------------------------------------------------------------------
void MainConsole::machineInfoView() {
#ifdef REMOVED_Dec202006
   QString mName = ui->comboBoxMachine->currentText();
   QString mFile = usr->LHOME + mName + "/" + mName + ".info";
   // checkMachineModified();
   // if (usr->isModelModified) std::cout << "Machine or Model Changed" << endl;

   QString CMD;
   // std::cout << "Machine View: " << comboBoxMachine->currentText() << endl;
   if (QFile::exists(mFile)) {
     // Use the existing machine information file in usr->LHOME/usr->CurrentMachine
     CMD = usr->LHOME + "bin/machineinfo.exe"
      + " -fname " + mName + ".info"
      + " -model " + mName
      + " -vendor " + mName
      + " -MDIR " + ui->lineEditTeleDir->text()
      + " -LLIB " + usr->LHOME + mName
      + " -LHOME " + usr->LHOME;
   } else {
     // Use a machine information file in usr->LHOME/lib directory
     CMD = usr->LHOME + "bin/machineinfo.exe"
      + " -fname " + usr->comboBoxModelFile
      + " -model " + mName
      + " -vendor " + mName
      + " -MDIR " + ui->lineEditTeleDir->text()
      + " -LLIB " + usr->LHOME + "lib"
      + " -LHOME " + usr->LHOME;
   }

   // Executes the command
   mySystem(CMD);

 updateButtons();
#endif
}  // End of void MainConsole::machineInfoView()
// -----------------------------------------------------------------------------
void MainConsole::updateButtons() {
   QString mName = ui->comboBoxMachine->currentText();
   // QTextStream (stdout) << "updateButtons: mName = " << mName << endl;

   // Check if the machine specifications have been modified by the machineinfo

   ui->groupBoxAFIT->setEnabled(false);
   ui->groupBoxMonoMC->setEnabled(false);
   ui->groupBoxWFIT->setEnabled(false);
   ui->groupBoxVerification->setEnabled(false);
   ui->groupBoxMCPB->setEnabled(false);
   ui->groupBoxPBComm->setEnabled(false);

   if (readLocalSetting(mName, "MachineModified") == "Modified") {
        // Machine geometry information is different from XiO machine data or previous data
        // All procedures will start over
      // initLocalSettings(mName); // re-start
        ui->groupBoxAFIT->setEnabled(true);     // Starts from 1st Step
        ui->pushButtonAirFit->setEnabled(true); // Gets In-air Data and Pops Up Airfit GUI
      ui->pushButtonAirReview->setEnabled(false); // Review is not available YET
   } else {
        // Machine information is not modified
      // ui->pushButtonAirFit->setEnabled(false); // REMOVED
      // ui->pushButtonAirReview->setEnabled(false); // REMOVED
   }


    // getAirDataNew() changes AirData to "Modified"
   if (readLocalSetting(mName,"AirData") == "Modified") {
       if (readLocalSetting(mName,"AFIT") == "Done") {
          // ui->pushButtonAirFit->setEnabled(false);     // REMOVED
          // ui->pushButtonAirReview->setEnabled(true);   // REMOVED
          // ui->groupBoxMonoMC->setEnabled(true);        // REMOVED
          // getPRLists(); // Gets FS1 and FS2 lists  // REMOVED
       } else {
        //  ui->pushButtonAirFit->setEnabled(true);     // REMOVED
        //  ui->pushButtonAirReview->setEnabled(false);  // REMOVED
        //  ui->groupBoxMonoMC->setEnabled(false);  //REMOVED
       }
    } else {
       ui->pushButtonAirFit->setEnabled(true);
       ui->pushButtonAirReview->setEnabled(false);
    }

    if (readLocalSetting(mName,"AFIT") == "Done") {
         ui->pushButtonAirFit->setEnabled(false);
       ui->pushButtonAirReview->setEnabled(true);
         ui->groupBoxMonoMC->setEnabled(true); // Next Step
         getPRLists(); // Gets FS1 and FS2 lists
       ui->progressBarMonoMC->setMaximum(10000);
       ui->progressBarMonoMC->reset();
       ui->comboBox1stFS->setEnabled(true);
       ui->comboBox2ndFS->setEnabled(true);
       ui->pushButtonMonoStart->setEnabled(true);
         /*  REMOVED
       if (readLocalSetting(mName,"MonoMC") == "Done") {
           progressBarMonoMC->setMaximum(10000);
           progressBarMonoMC->setValue(10000);
           comboBox1stFS->setEnabled(false);
           comboBox2ndFS->setEnabled(false);
           ui->pushButtonMonoStart->setEnabled(false);

           ui->groupBoxWFIT->setEnabled(true);
       } else {
           progressBarMonoMC->setMaximum(10000);
           progressBarMonoMC->reset();
           comboBox1stFS->setEnabled(true);
           comboBox2ndFS->setEnabled(true);
           ui->pushButtonMonoStart->setEnabled(true);

           ui->groupBoxWFIT->setEnabled(false);
       }
         */
    } else {
         ui->pushButtonAirFit->setEnabled(true);
       ui->pushButtonAirReview->setEnabled(false);
         ui->groupBoxMonoMC->setEnabled(false); // Next Step
       ui->pushButtonMonoStart->setEnabled(false);
     }

    if (readLocalSetting(mName,"MonoMC") == "Done") {
       // Previous Steps
         ui->pushButtonAirFit->setEnabled(false);
       ui->pushButtonAirReview->setEnabled(true);
       ui->pushButtonMonoStart->setEnabled(true); // Previous Step must be unabled but enabled here becuase of electron correction
         ui->groupBoxMonoMC->setEnabled(true); // Previous Step must be unabled but enabled here becuase of electron correction
         ui->groupBoxWFIT->setEnabled(true); // Next Step
       ui->pushButtonWfit->setEnabled(true);  // Nest Step
       ui->pushButtonWfitReview->setEnabled(false);  // Nest Step
         /* Modified
       if (readLocalSetting(mName,"WaterData") == "Modified") {
          ui->pushButtonWfit->setEnabled(true);
          ui->pushButtonWfitReview->setEnabled(false);
       } else {
          ui->pushButtonWfit->setEnabled(false);
          ui->pushButtonWfitReview->setEnabled(false);
       }
         */
    }

    if (readLocalSetting(mName,"WaterData") == "Modified") {
      if (readLocalSetting(mName,"WFIT") == "Done") {
         // ui->pushButtonWfit->setEnabled(false);        // REMOVED
         // ui->pushButtonWfitReview->setEnabled(true);   // REMOVED
         // ui->groupBoxVerification->setEnabled(true);  // REMOVED
         // ui->groupBoxMCPB->setEnabled(true);  // REMOVED
      } else {
         // ui->pushButtonWfit->setEnabled(true);  // REMOVED
         // ui->pushButtonWfitReview->setEnabled(false); // REMOVED
         // ui->groupBoxVerification->setEnabled(false); // REMOVED
         // ui->groupBoxMCPB->setEnabled(false);  // REMOVED
      }
    } else {
      ui->pushButtonWfit->setEnabled(true);
      ui->pushButtonWfitReview->setEnabled(false);
     }

    if (readLocalSetting(mName,"WFIT") == "Done") {
       // Previous Steps
         ui->pushButtonAirFit->setEnabled(false);
       ui->pushButtonAirReview->setEnabled(true);
       ui->pushButtonMonoStart->setEnabled(true); // Previous Step must be unabled but enabled here becuase of electron correction
         ui->groupBoxMonoMC->setEnabled(true); // Previous Step must be unabled but enabled here becuase of electron correction
         ui->groupBoxWFIT->setEnabled(true);
       ui->pushButtonWfit->setEnabled(false);
       ui->pushButtonWfitReview->setEnabled(true);
         // Next Steps
         ui->groupBoxMC->setEnabled(true);
       ui->pushButtonVerification->setEnabled(true);
       ui->pushButtonVerificationReview->setEnabled(false);
         ui->groupBoxMCPB->setEnabled(true);
       ui->pushButtonPBComm->setEnabled(true);
       ui->pushButtonPBReview->setEnabled(false);

       if (readLocalSetting(mName,"PBMC") == "Done") {
         // ui->pushButtonPBMC->setEnabled(true); // It was false  // REMOVED
         // ui->groupBoxPBComm->setEnabled(true); // REMOVED
       } else {
         // ui->pushButtonPBMC->setEnabled(true);  // REMOVED
         // ui->groupBoxPBComm->setEnabled(false); // REMOVED
       }
         writeLocalSetting(mName,"Verification", "NotDone");
    } else {
         ui->groupBoxMC->setEnabled(false);	// Next Step
       ui->pushButtonVerification->setEnabled(false); // Next Step
       ui->pushButtonVerificationReview->setEnabled(true); // Next Step
         ui->groupBoxMCPB->setEnabled(false);	// Next Step
       ui->pushButtonPBComm->setEnabled(false); // Next Step
       ui->pushButtonPBReview->setEnabled(true);  // Next Step
     }

    if (readLocalSetting(mName,"Verification") == "Done") {
         ui->groupBoxMC->setEnabled(true);
       ui->pushButtonVerification->setEnabled(false);
       ui->pushButtonVerificationReview->setEnabled(true);
    } else {
         ui->groupBoxMC->setEnabled(true);
       ui->pushButtonVerification->setEnabled(true);
       ui->pushButtonVerificationReview->setEnabled(false);
     }

     if (readLocalSetting(mName,"PBMC") == "Done") {
       // Previous Steps
         ui->pushButtonAirFit->setEnabled(false);
       ui->pushButtonAirReview->setEnabled(true);
       ui->pushButtonMonoStart->setEnabled(true); // Previous Step must be unabled but enabled here becuase of electron correction
         ui->groupBoxMonoMC->setEnabled(true); // Previous Step must be unabled but enabled here becuase of electron correction
         ui->groupBoxWFIT->setEnabled(true); // Previous Step
       ui->pushButtonWfit->setEnabled(false); // Previous Step
       ui->pushButtonWfitReview->setEnabled(true); // Previous Step
       ui->pushButtonPBComm->setEnabled(false); // Next Step
       ui->pushButtonPBReview->setEnabled(true); // Next Step
         ui->groupBoxMCPB->setEnabled(false);	// Next Step
         ui->groupBoxPBComm->setEnabled(true);	// Next Step

       if (readLocalSetting(mName,"PBComm") == "Done") {
          // ui->pushButtonPBComm->setEnabled(true); // REMOVED
          // ui->pushButtonPBReview->setEnabled(true);  // REMOVED
          // ui->pushButtonPBPack->setEnabled(true);   // REMOVED
       } else {
          // ui->pushButtonPBComm->setEnabled(true); // REMOVED
          // ui->pushButtonPBReview->setEnabled(false);  // REMOVED
          // ui->pushButtonPBPack->setEnabled(false);   // REMOVED
       }
     } else {
       ui->pushButtonPBComm->setEnabled(true);
       ui->pushButtonPBReview->setEnabled(false);
         ui->groupBoxPBComm->setEnabled(false);	// Next Step
     }

     if (readLocalSetting(mName,"PBComm") == "Done") {
        ui->groupBoxMCPB->setEnabled(false);	// Next Step
        if (readLocalSetting(mName,"Validate") == "Done") {
            ui->pushButtonPBPack->setEnabled(false);
        } else {
            ui->pushButtonPBPack->setEnabled(true);
        }
     } else {
        ui->groupBoxMCPB->setEnabled(true);	// Next Step
     }
     // PB SSD Update Here
     //QTextStream (stdout) << "updateButtons: End of mName = " << mName << endl;
} // End of updateButtons()
// -----------------------------------------------------------------------------
/* REMOVED
void MainConsole::machineInfoViewOld() {
#ifdef REMOVED_Dec202006
   QString mName = ui->comboBoxMachine->currentText();
   QString mFile = usr->LHOME + mName + "/" + mName + ".info";
   // checkMachineModified();
   // if (usr->isModelModified) std::cout << "Machine or Model Changed" << endl;

   QString CMD;
   // std::cout << "Machine View: " << comboBoxMachine->currentText() << endl;
   if (QFile::exists(mFile)) {
     // Use the existing machine information file in usr->LHOME/usr->CurrentMachine
     CMD = usr->LHOME + "bin/machineinfo.exe"
      + " -fname " + mName + ".info"
      + " -model " + mName
      + " -vendor " + mName
      + " -MDIR " + ui->lineEditTeleDir->text()
      + " -LLIB " + usr->LHOME + mName
      + " -LHOME " + usr->LHOME;
   }
#ifdef REMOVED_Jun162006
   else {
     // Use a machine information file in usr->LHOME/lib directory
     CMD = usr->LHOME + "bin/machineinfo.exe"
      + " -fname " + usr->comboBoxModelFile
      + " -model " + mName
      + " -vendor " + mName
      + " -MDIR " + ui->lineEditTeleDir->text()
      + " -LLIB " + usr->LHOME + "lib"
      + " -LHOME " + usr->LHOME;
   }
#endif
   // Executes the command
   mySystem(CMD);
   // Check if the machine specifications have been modified in the machineinfo viwer
   ui->groupBoxAFIT->setEnabled(true);
   if (readLocalSetting(mName, "MachineModified") == "Modified") {
      initLocalSettings(mName);
      ui->pushButtonAirFit->setEnabled(false);
      ui->pushButtonAirReview->setEnabled(false);
   } else {
      if (readLocalSetting(mName,"AirData") == "Modified") {
         if (readLocalSetting(mName,"AFIT") == "Done") {
            ui->pushButtonAirFit->setEnabled(false);
            ui->pushButtonAirReview->setEnabled(true);
            ui->groupBoxMonoMC->setEnabled(true);
            getPRLists();
         } else {
            ui->pushButtonAirFit->setEnabled(true);
            ui->pushButtonAirReview->setEnabled(false);
            ui->groupBoxMonoMC->setEnabled(false);
         }
      } else {
         if (readLocalSetting(mName,"AFIT") == "Done") {
            ui->pushButtonAirReview->setEnabled(true);
            ui->groupBoxMonoMC->setEnabled(true);
            getPRLists();
         } else {
            ui->pushButtonAirReview->setEnabled(false);
            ui->groupBoxMonoMC->setEnabled(false);
         }
      }

      if (readLocalSetting(mName,"MonoMC") == "Done") {
         ui->groupBoxMonoMC->setEnabled(false);
         ui->groupBoxWFIT->setEnabled(true);
         if (readLocalSetting(mName,"WaterData") == "Modified") {
            if (readLocalSetting(mName,"WFIT") == "Done") {
               ui->pushButtonWfit->setEnabled(false);
               ui->pushButtonWfitReview->setEnabled(true);
               ui->groupBoxVerification->setEnabled(true);
               ui->groupBoxMCPB->setEnabled(true);
            } else {
               ui->pushButtonWfit->setEnabled(true);
               ui->pushButtonWfitReview->setEnabled(false);
               ui->groupBoxVerification->setEnabled(false);
               ui->groupBoxMCPB->setEnabled(false);
            }
         } else {
            ui->pushButtonWfit->setEnabled(false);
            if (readLocalSetting(mName,"WFIT") == "Done") {
               ui->pushButtonWfitReview->setEnabled(true);
               ui->groupBoxVerification->setEnabled(true);
               ui->groupBoxMCPB->setEnabled(true);
            } else {
               ui->pushButtonWfitReview->setEnabled(false);
               ui->groupBoxVerification->setEnabled(false);
               ui->groupBoxMCPB->setEnabled(false);
            }
         }
      }
      if (readLocalSetting(mName,"PBMC") == "Done") {
         ui->groupBoxMCPB->setEnabled(true); // It was false
         ui->groupBoxPBComm->setEnabled(true);
         ui->pushButtonPBComm->setEnabled(true);
         ui->pushButtonPBReview->setEnabled(false);
         ui->pushButtonPBPack->setEnabled(false);
      }
      if (readLocalSetting(mName,"PBComm") == "Done") {
         ui->groupBoxPBComm->setEnabled(true);
         ui->pushButtonPBComm->setEnabled(true); // true for a while
         ui->pushButtonPBReview->setEnabled(true);
         ui->pushButtonPBPack->setEnabled(true);
      }
      if (readLocalSetting(mName,"Validate") == "Done") {
         ui->groupBoxPBComm->setEnabled(true);
         ui->pushButtonPBPack->setEnabled(false);
      }
   }
#endif
}  // End of void MainConsole::machineInfoView()
*/
// -----------------------------------------------------------------------------
void MainConsole::checkMachineModifiedRemoved() {
   if (usr->CurrentMachine != ui->comboBoxMachine->currentText()) {
      usr->isMachineModified = true;
      // std::cout << "checkMachineModified: Machine Modified" << endl;
      // std::cout << "usr->CurrentMachine=" << usr->CurrentModel << endl;
   }
   if (usr->CurrentModel != ui->comboBoxModel->currentText()){
      usr->isModelModified = true;
      // std::cout << "checkMachineModified: Model Modified" << endl;
      // std::cout << "usr->CurrentModel=" << usr->CurrentModel << endl;
   }
   if (usr->CurrentVendor != ui->comboBoxVendor->currentText()) {
      usr->isModelModified = true;
      // std::cout << "checkMachineModified: Vendor Modified" << endl;
      // std::cout << "usr->CurrentVendor=" << usr->CurrentVendor << endl;
   }
   if (usr->CurrentModelFile != usr->comboBoxModelFile) {
      usr->isModelFileModified = true;
      // std::cout << "checkMachineModified: ModelFile Modified" << endl;
      // std::cout << "usr->CurrentModelFile=" << usr->CurrentModelFile << endl;
   }
}
// -----------------------------------------------------------------------------
bool MainConsole::isThereDir( QString dirStr) {
   bool dirExists = false;
   QDir *aDir = new QDir;
   aDir->setPath(dirStr);
   if(aDir->exists()) dirExists = true;
   return (dirExists);
}
// -----------------------------------------------------------------------------
bool MainConsole::isThereFile( QString dirStr, QString fileStr) {
   if (!isThereDir(dirStr)) return(false);
   if ( QFile::exists( dirStr+"/"+fileStr ) ) return (true);
   return (false);
}
// -----------------------------------------------------------------------------
void MainConsole::backupAfitDir() { // backup the previous working afit directory
   QDir *afitDir = new QDir;
   QDir *afitDirTmp = new QDir;
   QString mName = ui->comboBoxMachine->currentText();
   usr->AFIT_DIR = usr->LHOME + mName + "/afit";
   QDate date = QDate::currentDate();

   QString tmpAfitDir = usr->AFIT_DIR + "." + date.toString("MM.dd.yyyy");
   afitDir->setPath(usr->AFIT_DIR);
   afitDirTmp->setPath(tmpAfitDir);
   if(afitDir->exists()) {
      // std::cout << "rename " << usr->AFIT_DIR << " to " << tmpAfitDir << endl;
      if(afitDirTmp->exists()) {
         QString CMD = "rm -rf " + tmpAfitDir;
         // afitDirTmp->rmdir(tmpAfitDir, true);
         mySystem(CMD);
      }
      afitDir->rename(usr->AFIT_DIR, tmpAfitDir);
      usr->afitDirExists = false;
   }
}
// -----------------------------------------------------------------------------
bool MainConsole::backupDir(QString dirName) { // backup the previous working afit directory
   QDir *dir = new QDir;
   QDir *dirTmp = new QDir;
   // usr->WFIT_DIR = usr->LHOME + usr->CurrentMachine + "/wfit";
   QDate date = QDate::currentDate();

   QString dirTmpName = dirName + "." + date.toString("MM.dd.yyyy");
   dir->setPath(dirName);
   dirTmp->setPath(dirTmpName);
   if(dir->exists()) {
      // std::cout << "rename " << usr->AFIT_DIR << " to " << tmpAfitDir << endl;
      if(dirTmp->exists()) {
         QString CMD = "rm -rf " + dirTmpName;
         // afitDirTmp->rmdir(tmpAfitDir, true);
         mySystem(CMD);
      }
      dir->rename(dirName, dirTmpName);
      return (true);
   }
   return(false);
}
// -----------------------------------------------------------------------------
void MainConsole::makeDirOld(QString dirStr) {
   // Check if there is Machine directory. If not, make the directory.
   QString MachineDir = usr->LHOME + ui->comboBoxMachine->currentText();
   if (ui->comboBoxMachine->currentText().length() > 0 && !isThereDir(dirStr)) {
      QString CMD = "mkdir " + dirStr;
      mySystem(CMD);
   }
}
// -----------------------------------------------------------------------------
void MainConsole::makeDir(QString dirStr) {
   QString lastDir = dirStr.section('/',-1);
   QString dirPath = dirStr.section('/',0,-2);
   // QTextStream (stdout) << "makeDir: dirPath = " << dirPath << endl;
   // QTextStream (stdout) << "makeDir: lastDir = " << lastDir << endl;
   QDir *afitDir = new QDir;
   afitDir->setPath(dirStr);
   if(!afitDir->exists()) {
      afitDir->setPath(dirPath);
      afitDir->mkdir(lastDir);
      if(afitDir->exists())
        QTextStream (stdout) << "makeDir:" << dirStr << " is made." << endl;
   }
   else {
       QTextStream (stdout) << "makeDir:" << dirStr << " exists." << endl;
   }
}
// -----------------------------------------------------------------------------
void MainConsole::runAirFitNew() {
   getAirDataNew(); // Every time get the data from XiO
   // usr->E0 = readLocalSetting(comboBoxMachine->currentText(),"Energy");
   // Check machine directory exists in Loacl Home like Monaco
   QString mName = ui->comboBoxMachine->currentText();
   QString WORK_DIR = usr->LHOME + mName; // It may be same as XVMC_WORK
   if (!isThereDir(WORK_DIR)) makeDir(WORK_DIR);

   // Check afit directory exists
   QString AFIT_DIR = WORK_DIR + "/afit";
   if (!isThereDir(AFIT_DIR)) makeDir(AFIT_DIR);

#ifdef REMOVED_June262006
   // Check afit/afit.inp file exists
   QString AFIT_INP = AFIT_DIR + "/afit.inp";
   QFile afitInp(AFIT_INP);
   if (afitInp.exists()) {
      QString CMD = "cp -f " + AFIT_INP + " " + AFIT_INP + ".tmp";
      mySystem(CMD);
   }
#endif
   // Check afit/afit.inp.tmp file exists
   QString AFIT_INP_TMP = AFIT_DIR + "/afit.inp.tmp";
   QFile afitInpTmp(AFIT_INP_TMP);
   if (!afitInpTmp.exists()) {
      QString mInfoFile = usr->LHOME + mName + "/" + mName + ".info" ;
      getMachineInfo(mInfoFile);
      bool ok;
      float zx = -1;
      float zy = -1;
      float zm = -1;
      if (usr->SUJD != "" && usr->UJT != "")
#ifdef XVMC
         zy = usr->SUJD.toFloat(&ok) + usr->UJT.toFloat(&ok)/2.0;  // Original for XVMC old afit
#else
         zy = usr->SUJD.toFloat(&ok) - usr->UJT.toFloat(&ok)/2.0; // (+) to (-) for new afir  April 19, 2006
#endif

      if (usr->SLJD != "" && usr->LJT != "")
#ifdef XVMC
         zx = usr->SLJD.toFloat(&ok) + usr->LJT.toFloat(&ok)/2.0; // Original for XVMC old afit
#else
         zx = usr->SLJD.toFloat(&ok) - usr->LJT.toFloat(&ok)/2.0; // (+) to (-) for new afir  April 19, 2006
#endif

      if (usr->SMD != "" && usr->tMLC != "")
         zm = usr->SMD.toFloat(&ok) - usr->tMLC.toFloat(&ok)/2.0; // Original for XVMC old afit

      QString p0 = "0.90";   QString sw_p0 = "1";
      QString s0 = "0.15";   QString sw_s0 = "1";
      QString h0 = "0";      QString sw_h0 = "1";
      QString h1 = "0";      QString sw_h1 = "1";
      QString h2 = "0";      QString sw_h2 = "1";
      QString h3 = "0";      QString sw_h3 = "1";
      QString h4 = "0";      QString sw_h4 = "1";
      QString ss = "1.5";    QString sw_ss = "1";

      QString Z0 = "0.0";       QString sw_Z0 = " ";
      QString ZS = usr->SFD;    QString sw_ZS = " ";
      QString MX = " ";         QString sw_MX = usr->MX;
      QString MY = " ";         QString sw_MY = usr->MY;
      QString ZX = " ";   QString sw_ZX = "";  ZX.sprintf("%.3f",zx);
      QString ZY = " ";   QString sw_ZY = "";  ZY.sprintf("%.3f",zy);
      QString ZM = " ";   QString sw_ZM = "";  ZM.sprintf("%.3f",zm);
      QString ZI = "100";       QString sw_ZI = " ";
      QString XN = "0.0";       QString sw_XN = " ";
      QString YN = "0.0";       QString sw_YN = " ";
      QString ZN = "100.0";     QString sw_ZN = " ";
      QString WXN = usr->WXN;   QString sw_WXN = " ";
      QString WYN = usr->WYN;   QString sw_WYN = " ";

      float eE = 0.13 * usr->E0.toFloat(&ok) + 0.55;
      QString eEnergy = ""; eEnergy.sprintf("%.3f",eE);
#ifdef XVMC
      QString version = "2.6";
#else
      QString version = "3.0"; // For new afit
#endif

      usr->nLines = 0;
      QFile mFile(AFIT_DIR+"/afit.lst");
      if (mFile.exists()) {
         QTextStream stream( &mFile );
         QString sLine;
         mFile.open( QIODevice::ReadOnly);
         while ( !stream.atEnd() ) {
         sLine = stream.readLine();
         QString strLine = sLine.simplified();
         if (strLine.contains("#file name")) {
            usr->WXN = strLine.section("WX = ",1,1).section("WY = ",0,0);
            usr->WYN = strLine.section("WY = ",1,1);
         }
         if (strLine.length() > 1 && strLine.left(1) != "#")
            usr->nLines++;
         }
         mFile.close();
      }

      // write AFIT_INP
      QFile oFile(AFIT_INP_TMP);
      oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
      QTextStream oStream( &oFile );

      QString tmpStr;
      oStream << "version:   " << version << endl;
      oStream << "energy:   " << usr->E0 << endl;
      oStream << "p0:   " << p0 << "  " << sw_p0 << endl;
      oStream << "s0:   " << s0 << "  " << sw_s0 << endl;
      oStream << "h0:   " << h0 << "  " << sw_h0 << endl;
      oStream << "h1:   " << h1 << "  " << sw_h1 << endl;
      oStream << "h2:   " << h2 << "  " << sw_h2 << endl;
      oStream << "h3:   " << h3 << "  " << sw_h3 << endl;
      oStream << "h4:   " << h4 << "  " << sw_h4 << endl;
      oStream << "ss:   " << ss << "  " << sw_ss << endl;
      oStream << "Z0:   " << Z0 << "  " << sw_Z0 << endl;
      oStream << "ZS:   " << ZS << "  " << sw_ZS << endl;
      oStream << "ZM:   " << ZM << "  " << sw_MX << "  " << sw_MY << endl;
    if (usr->XIOMLC == "ElektaBM80leaf" || usr->XIOMLC == "PHILIPSMLC") {
      oStream << "ZX:   " << ZY << "  " << sw_ZX << endl;
      oStream << "ZY:   " << ZX << "  " << sw_ZY << endl;
 } else {
      oStream << "ZX:   " << ZX << "  " << sw_ZX << endl;
      oStream << "ZY:   " << ZY << "  " << sw_ZY << endl;
 }
      oStream << "ZI:   " << ZI << "  " << sw_ZI << endl;
      oStream << "XN:   " << XN << "  " << sw_XN << endl;
      oStream << "YN:   " << YN << "  " << sw_YN << endl;
      oStream << "ZN:   " << ZN << "  " << sw_ZN << endl;
      oStream << "WXN:   " << usr->WXN << "  " << sw_WXN << endl;
      oStream << "WYN:   " << usr->WYN << "  " << sw_WYN << endl;
      tmpStr.sprintf("%d", usr->nLines);
      oStream << "list:   afit.lst   " << tmpStr << endl;
    QDate date = QDate::currentDate();
      QString DT = date.toString("MM.dd.yyyy");
      oStream << "# Date: " << DT << endl;
   }

   QString LBIN = usr->LHOME + "bin";
   QString CMD = "cd " + AFIT_DIR + ";" + LBIN
#ifdef XVMC
               + "/airfit_xvmc.exe -i airfit.inp.tmp > afit.out";
#else
               + "/airfit.exe -i airfit.inp.tmp > afit.out";
#endif
   mySystem(CMD);
   if (readLocalSetting(mName, "AFIT") == "Done") {
      ui->pushButtonAirFit->setEnabled(false);
      ui->pushButtonAirReview->setEnabled(true);
      ui->groupBoxMonoMC->setEnabled(true);
      getPRLists();
   } else {
      ui->pushButtonAirFit->setEnabled(true);
      ui->pushButtonAirReview->setEnabled(false);
      ui->groupBoxMonoMC->setEnabled(false);
   }

    updateAirFitValues();

   // std::cout << "AirData = " << readLocalSetting(mName, "AirData") << endl;
   // std::cout << "AFIT = " << readLocalSetting(mName, "AFIT") << endl;
   updateButtons(); // Added on June 19, 2006
   // std::cout << " AirData = " << readLocalSetting(mName, "AirData") << endl;
   // std::cout << " AFIT = " << readLocalSetting(mName, "AFIT") << endl;
}
// -----------------------------------------------------------------------------
void MainConsole::plotAirFit() {
   QString LBIN = usr->LHOME + "bin";
   QString mName = ui->comboBoxMachine->currentText();
   QString CMD = "cd " + usr->LHOME + mName
         + "/afit; " + LBIN + "/plotAir.exe";
   mySystem(CMD);
}
// -----------------------------------------------------------------------------
void MainConsole::makeMonoVMC(QString plan) {
   QString mName = ui->comboBoxMachine->currentText();
   // read machine information from .info file
   QString modelFile = usr->LHOME + mName + "/" + mName + ".info" ;
   getMachineInfo(modelFile);

   QString MONO_DIR = usr->LHOME + mName + "/" + mName + "_MONO";
   QString MONO_DIR_INP = MONO_DIR + "/INPUT";
   makeDir(MONO_DIR);
   makeDir(MONO_DIR_INP);

   QString pEng = readLocalSetting(mName,"Energy").section('.',0,0);
   QString BDT = mName + "_GeoModel";
   QString BDTECON = mName + "_GeoModelEcon";

   QString DeviceType = "102";  // Monoenergy Photon
   QString matMLC = "tungsten";

   int nMLChalf = 0;
   int iMLChalf = 0;
   int iMLCstart = 0;
   int iMLCend = 0;

   bool ok;
   float uMLC = 0; // upper limit of MLC
   float lMLC = 0; // lower limit of MLC
   float maxFW = usr->MAXFW.toFloat(&ok)/10.0;
   float p1MLC = -0.5; // Closed MLC Position
   float p2MLC =  0.5; // Closed MLC Position
   if (usr->XIOMLC.contains("ElektaBM80leaf")) {
      p1MLC = -maxFW/2.0 - 0.1;
      p2MLC = -maxFW/2.0;
   }

   // std::cout << "isMLC = " << usr->isMLC << endl;
   // std::cout << "isMLC.compare = " << usr->isMLC.compare("1")<< endl;
   // std::cout << "makeMonoVMC:: usr->SSD = " << usr->SSD << endl;
   if (usr->isMLC.compare("1") == 0) {
      nMLChalf = usr->nMLC.toInt(&ok,10)/2;
      iMLChalf = usr->iMLC.toInt(&ok,10)/2;
      iMLCstart = (usr->nMLC.toInt(&ok,10)-usr->iMLC.toInt(&ok,10))/4+1;
      iMLCend = iMLCstart + usr->iMLC.toInt(&ok,10);
      // std::cout << "usr->SMD.toFloat = " << usr->SMD.toFloat(&ok) << endl;
      // std::cout << "usr->tMLC.toFloat = " << usr->tMLC.toFloat(&ok) << endl;
      uMLC = usr->SMD.toFloat(&ok) - usr->tMLC.toFloat(&ok)/2; // upper limit of MLC
      lMLC = usr->SMD.toFloat(&ok) + usr->tMLC.toFloat(&ok)/2; // lower limit of MLC
      p1MLC = -0.5; // Closed MLC Position
      p2MLC =  0.5; // Closed MLC Position
      if (usr->XIOMLC.contains("ElektaBM80leaf")) {
         p1MLC = -maxFW/2.0 - 0.1;
         p2MLC = -maxFW/2.0;
      }
   }

   QString fName = MONO_DIR_INP + "/" + plan + ".vmc";
   std::cout << "makeMonoVMC::Start Mono MC for " << fName.toStdString() << endl;
   QString FS1 = ui->comboBox1stFS->currentText();
   // float fs = 10.0; // Field Size
   float fs = FS1.toFloat(&ok)/10.0; // Field Size
   float halfFS = fs/2;
   int halfFSmm = (int)(10*fs/2);

   // Phantom Specifications: Voxel Size
   float xVoxelSize = 1.0;
   float yVoxelSize = 1.0;
   float zVoxelSize = 0.5;
   // Phantom Specifications: Phantom Size
   float xPhantomSize = 51.0;
   float yPhantomSize = 51.0;
   float zPhantomSize = 50.0;
   // Phantom Specifications: Number of Voxels
   int nX = (int)(xPhantomSize/xVoxelSize);
   int nY = (int)(yPhantomSize/yVoxelSize);
   int nZ = (int)(zPhantomSize/zVoxelSize);
   // Phantom Specifications: Phantom Center
   float xCenter = xPhantomSize/2;
   float yCenter = yPhantomSize/2;
   //  float zCenter = zPhantomSize/2;
   // Isocenter
   float zISO = 100.0 - usr->SSD.toFloat(&ok)/10.0;
   // std::cout << "makeMonoVMC:: zISO = " << zISO << endl;
   // Reference Depth
   float zRef = 10.0; // cm

   QFile oFile( fName );
   oFile.remove();
   oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite);
   QTextStream oStream( &oFile );

   QString tmpStr;
   oStream << "*PHANTOM        | " << endl;
   oStream << "-VOXELSIZE      | ";
   oStream << tmpStr.setNum(xVoxelSize,'f',2) << " ";
   oStream << tmpStr.setNum(yVoxelSize,'f',2) << " ";
   oStream << tmpStr.setNum(zVoxelSize,'f',2) << endl;
   oStream << "-DIMENSION      | ";
   oStream << tmpStr.setNum(nX,10) << " ";
   oStream << tmpStr.setNum(nY,10) << " ";
   oStream << tmpStr.setNum(nZ,10) << endl;
   oStream << "-CHANGE-DENSITY | ";
   oStream << "  1  " << tmpStr.setNum(nX,10) << "   ";
   oStream << "  1  " << tmpStr.setNum(nY,10) << "   ";
   oStream << "  1  " << tmpStr.setNum(nZ,10) << "   1.0000" << endl;
   oStream << "!" << endl;
   oStream << "*GLOBAL-DATA    | " << endl;
   oStream << "-WRITE-3D-DOSE  | 0 " << endl;
   // xy plane
   oStream << "-DEPTH-DOSE     | ";
   oStream << tmpStr.setNum(xCenter,'f',2) << " ";
   oStream << tmpStr.setNum(yCenter,'f',2) << " " << endl;
   // if (plan.compare("el_con") != 0)
   //   oStream << "-E-CUTOFF       | 0.25 " << endl;
   oStream << "-P0-CUTOFF-KERMA| 0.25 " << endl;
   oStream << "-P1-CUTOFF-KERMA| 2.00 " << endl;
   oStream << "-DOSE-TYPE      | 0  1.0" << endl;
   oStream << "-REFERENCE-POINT| ";
   oStream << tmpStr.setNum(xCenter,'f',2) << " ";
   oStream << tmpStr.setNum(yCenter,'f',2) << " ";
   oStream << tmpStr.setNum(zRef,'f',2) << endl;
   oStream << "-RANDOM-SET     | 23  45  67  89" << endl;
   oStream << "!" << endl;
   oStream << "*BEAM-PARAMETERS| " << endl;
   oStream << "-BEAM-WEIGHT    | 100.0 " << endl;
   oStream << "-DEVICE-TYPE    | " << DeviceType << endl;
   // oStream << "-APPLICATOR     | " << ConeSize << "x" << ConeSize << endl;

   float fSTD = ui->lineEditSTDev->text().toFloat(&ok);
   int iSTD = (int)(1000000/fSTD);
   // std::cout << "fSTD = " << fSTD << " iSTD = " << iSTD << endl;
   if (plan.contains("el_con")) {
      oStream << "-DEVICE-KEY     | " << BDTECON << endl;
      oStream << "-EVENT-NUMBER   | " << iSTD << "0 40  1  100" << endl;
      oStream << "-NOMINAL-ENERGY | " << usr->E << endl;
   } else {
      oStream << "-DEVICE-KEY     | " << BDT << endl;
      oStream << "-EVENT-NUMBER   | " << iSTD << " 40  1  100" << endl;
      QString Energy = plan.section("_",0,0).section("m",1,1)
            + "." + plan.section("_",1,1);
      oStream << "-NOMINAL-ENERGY | " << Energy << endl;
   }
   oStream << "-ISOCENTER      | ";
   oStream << tmpStr.setNum(xCenter,'f',2) << " ";
   oStream << tmpStr.setNum(yCenter,'f',2) << " ";
   oStream << tmpStr.setNum(zISO,'f',2) << endl;
   oStream << "-GANTRY-ANGLE   | F  0  360" << endl;
   oStream << "-TABLE-ANGLE    | 0" << endl;
   oStream << "-COLL-ANGLE     | 0" << endl;
   oStream << "-COLL-WIDTH-X   | " << fs << endl;
   oStream << "-COLL-WIDTH-Y   | " << fs << endl;
   //   ---------------------
   if (usr->isMLC.compare("1") == 0) {
      bool ok;
      QString SPACE;
      if (usr->MLCtype.compare("RNDFOCUS-MLC") == 0) SPACE = "   |";
      if (usr->MLCtype.compare("VARIAN-MLC") == 0) SPACE = "     |";
      if (usr->MLCtype.compare("DBLFOCUS-MLC") == 0) SPACE = "   |";
      if (usr->MLCtype.compare("SIMPLE-MLC") == 0) SPACE = "     |";
      if (usr->MLCtype.compare("ELEKTA-MLC") == 0) SPACE = "     |";
      oStream << "-" << usr->MLCtype << SPACE
              << "  " << usr->nMLC << "  " << usr->oMLC
              << "  " << matMLC << "  " << uMLC
              << "  " << lMLC << "  " << usr->cMLC
              << "  " << usr->rMLC << endl;

      int i = 0;
      float MLCpos = (usr->nMLC.toInt(&ok,10) - usr->iMLC.toInt(&ok,10))/2
         * usr->tkMLC.toFloat(&ok)
         + usr->iMLC.toInt(&ok,10)/2 * usr->thMLC.toFloat(&ok);

      //  std::cout << "MLCpos = " << MLCpos << endl;

      while (i < usr->nMLC.toInt(&ok,10)) {
         float tMLC = usr->tkMLC.toFloat(&ok);
         if (i >= iMLCstart && i < iMLCend) tMLC = usr->thMLC.toFloat(&ok);
         MLCpos = MLCpos - tMLC;
         float tMLCcm = tMLC/10.0;
         // std::cout << "MLCpos = " << MLCpos << "  halfFSmm = " << halfFSmm << endl;
         if (MLCpos >= halfFSmm) {
            oStream << tMLCcm << " " << p1MLC << "  " <<  p2MLC << endl;
         } else {
            float MLCposPlus = MLCpos + tMLC;
            if (MLCposPlus <= -halfFSmm)
               oStream << tMLCcm << "  " << p1MLC << "  " << p2MLC << endl;
            else
               oStream << tMLCcm << "  " << -halfFS << "  " << halfFS << endl;
         }
         i++;
      }
   }

   oStream << "!" << endl;
   oStream << "*END-INPUT      | " << endl;

   oFile.close();
}
// -----------------------------------------------------------------------------
bool MainConsole::checkMonoMC() {

   // std::cout << "checkMonoMC " << endl;
   QString mName = ui->comboBoxMachine->currentText();
   QString MONO_DIR = usr->LHOME + mName + "/" + mName + "_MONO";
   makeDir(MONO_DIR);

   QString pEng = readLocalSetting(mName,"Energy").section('.',0,0);
   QStringList planList;
   bool ok;
   if(pEng.simplified().toFloat(&ok) < 10.0) {
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "el_con";
   } else if (pEng.simplified().toFloat(&ok) < 15.0){
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
               << "m12_00" << "m13_00" << "m14_00" << "m15_00"
               << "el_con";
    } else if (pEng.simplified().toFloat(&ok) < 20.0){
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
               << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
               << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
               << "m22_00" << "m23_00" << "m24_00" << "m25_00"
               << "el_con";
   } else {
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
               << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
               << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
               << "m22_00" << "m23_00" << "m24_00" << "m25_00" << "m26_00"
               << "m27_00" << "m28_00" << "m29_00" << "m30_00"
               << "el_con";
   }

   QString FS1 = ui->comboBox1stFS->currentText();
   QString FS2 = ui->comboBox2ndFS->currentText();
   if (FS1 != "None") {
      QString FS;
      FS.sprintf("%d", FS1.toInt(&ok,10)/10);
      // std::cout << "FS = " << FS << endl;
      for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
         QString plan = *it;
#ifdef XVMC
         QString fName = MONO_DIR + "/" + FS+"x"+FS
                       + "/" + plan + "_p.pz0";
#else
         QString fName = MONO_DIR + "/" + FS+"x"+FS
                       + "/" + plan.toUpper() + "_BEAM1_PROFILE1.txt";
#endif
         std::cout << "Mono_File = " << fName.toStdString() << endl;
         QFile oFile( fName );
         if (!oFile.exists()) return(false);
      } // foreach planList
   }
#ifndef XVMC
   if (FS2 != "None") {
      QString FS;
      FS.sprintf("%d", FS2.toInt(&ok,10)/10);
      for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
         QString plan = *it;
         QString fName = MONO_DIR + "/" + FS+"x"+FS
                       + "/" + plan.toUpper() + "_BEAM1_PROFILE1.txt";
         QFile oFile( fName );
         if (!oFile.exists()) return(false);
      } // foreach planList
   }
#endif
   return(true);
}
// -----------------------------------------------------------------------------
void MainConsole::writeRefFS() {
   QString mName = ui->comboBoxMachine->currentText();
   QString FS1 = ui->comboBox1stFS->currentText();
   QString FS2 = ui->comboBox2ndFS->currentText();
   writeLocalSetting(mName,"FS1",FS1);
   writeLocalSetting(mName,"FS2",FS2);
}
// -----------------------------------------------------------------------------
void MainConsole::runMonoMC() {
 QString mName = ui->comboBoxMachine->currentText();

 writeAirOpt();

#ifdef XVMC
 runMonoXVMC();
#else
 runMonoVerify();
#endif
}
// -----------------------------------------------------------------------------
void MainConsole::runMonoXVMC() { // for XVMC Use

   QString mName = ui->comboBoxMachine->currentText();
   QString FS1 = ui->comboBox1stFS->currentText();
   QString FS;
   bool ok;
   FS.sprintf("%d", FS1.toInt(&ok,10)/10);
   QString MONO_DIR = usr->LHOME + mName + "/" + mName + "_MONO";
   QString MONO_DIR_INP = MONO_DIR + "/INPUT";
   makeDir(MONO_DIR);
   makeDir(MONO_DIR_INP);
   QString MONO_DIR_FS = MONO_DIR + "/" + FS + "x" + FS;
   std::cout << "runMOnoXVMC()::MONO_DIR_FS = " << MONO_DIR_FS.toStdString() << endl;
   makeDir(MONO_DIR_FS);

   ui->progressBarMonoMC->setMaximum(10000);
   ui->progressBarMonoMC->reset();
   int iProgress = 0;

#ifdef REMOVED_Jun162006
   QString qXVMC_HOME = "XVMC_HOME=" + usr->LHOME + "bin";
   QString qXVMC_WORK = "XVMC_WORK=" + usr->LHOME + mName;
   const char *XVMC_HOME = qXVMC_HOME.data();
   const char *XVMC_WORK = qXVMC_WORK.data();

   putenv(qstrdup(XVMC_HOME));
   putenv(qstrdup(XVMC_WORK));
#endif
   setenv("XVMC_HOME", (usr->LHOME + "bin").toStdString().c_str(), 1);
   setenv("XVMC_WORK", (usr->LHOME + mName).toStdString().c_str(), 1);
   std::cout << "runMonoXVMC()::" << endl;
   QString pEng = readLocalSetting(mName,"Energy").section('.',0,0);
   QStringList planList;
   if (ui->checkBoxElectronOnly->isChecked()) {
        planList << "el_con";
   }
   else {
   if(pEng.simplified().toFloat(&ok) < 10.0) {
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "el_con";
   } else if (pEng.simplified().toFloat(&ok) < 15.0){
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
               << "m12_00" << "m13_00" << "m14_00" << "m15_00"
               << "el_con";
    } else if (pEng.simplified().toFloat(&ok) < 20.0){
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
               << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
               << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
               << "m22_00" << "m23_00" << "m24_00" << "m25_00"
               << "el_con";
   } else {
      planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
               << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
               << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
               << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
               << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
               << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
               << "m22_00" << "m23_00" << "m24_00" << "m25_00" << "m26_00"
               << "m27_00" << "m28_00" << "m29_00" << "m30_00"
               << "el_con";
   }
   }
   QString patient = mName + "_MONO";
    QString qExe = "xvmc";
    if (ui->checkBoxOffAxis->isChecked()) qExe = "xvmc_offaxis";

#define PHOTON_SPLIT ON
   for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
      QString plan = *it;
#ifndef PHOTON_SPLIT
      makeMonoVMC(plan);
      QString LBIN = usr->LHOME + "bin";
      QString CMD = "cp -f " + MONO_DIR_INP + "/" + plan + ".vmc "
            + MONO_DIR + "; "
            + LBIN + "/" + qExe + " " + patient + " " + plan
            + " > " + MONO_DIR + "/" + plan + ".log";

      // std::cout << CMD << endl;

      mySystem(CMD);

      CMD = " mv " + MONO_DIR + "/" + plan + ".* " + MONO_DIR_FS;
      mySystem(CMD);
#else
      if (plan.contains("el_con")) updateBDT("XVMC_Mono_e",pEng);
      else                         updateBDT("XVMC_Mono_p",pEng);
      QString plan_p = plan + "_p";
      makeMonoVMC(plan_p);
      QString LBIN = usr->LHOME + "bin";
      QString CMD = "cp -f " + MONO_DIR_INP + "/" + plan_p + ".vmc "
            + MONO_DIR + "; "
            + LBIN + "/" + qExe +" " + patient + " " + plan_p
            + " > " + MONO_DIR + "/" + plan_p + ".log";
      // std::cout << CMD << endl;
      mySystem(CMD);
      CMD = " mv " + MONO_DIR + "/" + plan_p + ".* " + MONO_DIR_FS;
      mySystem(CMD);

      if (!plan.contains("el_con")) {
         updateBDT("XVMC_Mono_s",pEng);
          QString plan_s = plan + "_s";
          makeMonoVMC(plan_s);
          CMD = "cp -f " + MONO_DIR_INP + "/" + plan_s + ".vmc "
                    + MONO_DIR + "; "
                    + LBIN + "/" + qExe + " " + patient + " " + plan_s
                    + " > " + MONO_DIR + "/" + plan_s + ".log";
          // std::cout << CMD << endl;
          mySystem(CMD);
          CMD = " mv " + MONO_DIR + "/" + plan_s + ".* " + MONO_DIR_FS;
          mySystem(CMD);
      }
#endif
      iProgress += 500;
      int p = ui->progressBarMonoMC->value();
      while ( p <= iProgress) ui->progressBarMonoMC->setValue(p++);
      // std::cout << "Progress = " << p << "  iProgress = " << iProgress << endl;
   } // foreach planList

   for (int p = ui->progressBarMonoMC->value(); p <= 10000; p++)
      ui->progressBarMonoMC->setValue(p);
   if (checkMonoMC()) {
      writeLocalSetting(mName,"MonoMC","Done");
      ui->groupBoxWFIT->setEnabled(true);
      ui->pushButtonMonoStart->setEnabled(false);
      ui->comboBox1stFS->setEnabled(false);
      ui->comboBox2ndFS->setEnabled(false);
   } else {
      ui->groupBoxAFIT->setEnabled(true);
      writeLocalSetting(mName,"MonoMC","NotDone");
      ui->groupBoxWFIT->setEnabled(false);
      ui->comboBox1stFS->setEnabled(true);
      ui->comboBox2ndFS->setEnabled(true);
   }
   writeRefFS();
}
// -----------------------------------------------------------------------------
void MainConsole::runMonoVerify() { // for Verify Use
   bool ok;
   QString FS1 = ui->comboBox1stFS->currentText();
   QString FS2 = ui->comboBox2ndFS->currentText();
   mySystem("date");
   if (FS2 != "None") {
      QString FS;
      FS.sprintf("%d", FS2.toInt(&ok,10)/10);
      QString FScm;
      FScm.sprintf("%.1f", FS2.toFloat(&ok)/10.0);
      runMonoVerify(FS,FScm);
   }
   mySystem("date");
   if (FS1 != "None") {
      QString FS;
      FS.sprintf("%d", FS1.toInt(&ok,10)/10);
      QString FScm;
      FScm.sprintf("%.1f", FS1.toFloat(&ok)/10.0);
      runMonoVerify(FS,FScm);
      ui->groupBoxAFIT->setEnabled(false);
   }
   mySystem("date");
   writeRefFS();
}
// -----------------------------------------------------------------------------
void MainConsole::runMonoVerify(QString FS, QString FScm) { // for Verify Use

   QString mName = ui->comboBoxMachine->currentText();
   QString XVMC_HOME = usr->LHOME + "bin";
   QString XVMC_WORK = usr->LHOME + mName;
   QString MONO_DIR = XVMC_WORK + "/" + mName + "_MONO";
   makeDir(MONO_DIR);
   QString MONO_DIR_FS = XVMC_WORK + "/" + mName + "_MONO/" + FS +"x" + FS;
   makeDir(MONO_DIR_FS);
   // QString MONO_DIR_INP = MONO_DIR + "/INPUT";

   setenv ("XVMC_HOME", XVMC_HOME.toLatin1(), 1);
   setenv ("XVMC_WORK", XVMC_WORK.toLatin1(), 1);
   setenv ("MACHINE", mName.toLatin1(), 1);

   QString pEng = readLocalSetting(mName,"Energy").section('.',0,0);
   QStringList planList;
   bool ok;

    if (ui->checkBoxElectronOnly->isChecked()) {
        planList << "el_con";
    }
    else {
    if(pEng.simplified().toFloat(&ok) < 10.0) {
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "el_con";
    } else if (pEng.simplified().toFloat(&ok) < 15.0){
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
                << "m12_00" << "m13_00" << "m14_00" << "m15_00"
                << "el_con";
        } else if (pEng.simplified().toFloat(&ok) < 20.0){
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
                << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
                << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
                << "m22_00" << "m23_00" << "m24_00" << "m25_00"
                << "el_con";
    } else {
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
                << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
                << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
                << "m22_00" << "m23_00" << "m24_00" << "m25_00" << "m26_00"
                << "m27_00" << "m28_00" << "m29_00" << "m30_00"
                << "el_con";
    }
   }
/* // REMOVED
   QString TELE_DIR = ui->lineEditTeleDir->text() + mName;
   // File for In-Air Output Factors
   QString OP_FILE = TELE_DIR + "/op";
   QString RefFS = getOutputFactor(OP_FILE,5,0.0);
   RefFS = RefFS.section('|',1,1); // Reference Depth at isocenter in millimeter
   float refFS = RefFS.toFloat(&ok);
*/
   ui->progressBarMonoMC->setMaximum(10000);
   ui->progressBarMonoMC->reset();
   int iProgress = 0;

    // Count Number of Strings (Mono-energy)
    int nStrings = 0;
   for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it )
        nStrings++;
    int dProgress = 10000/nStrings;

    float fs = FScm.toFloat(&ok);

    // Unit is mm -----------------------------------
    int xPhantomSize = 400;
    int yPhantomSize = 400;
    int zPhantomSize = 400;
    QString VOXELSIZE = ui->comboBoxGridSize->currentText();
    if (fs < 3.0)  VOXELSIZE = "0.2";

    QString SSDmm = "1000";
    SSDmm = usr->SSD;
    int VoxelSizeMM = VOXELSIZE.toInt(&ok,10);
    int nXVoxels = xPhantomSize/VoxelSizeMM;
    int nYVoxels = yPhantomSize/VoxelSizeMM;
    int nZVoxels = zPhantomSize/VoxelSizeMM;

    if (ui->comboBoxVoxels->currentText() == "1") {
        nXVoxels++;
        nYVoxels++;
        xPhantomSize = nXVoxels * VoxelSizeMM;
        yPhantomSize = nYVoxels * VoxelSizeMM;
    }
    float xIsocenter = xPhantomSize/2.0;
    float yIsocenter = yPhantomSize/2.0;
    float zIsocenter = 1000.0 - SSDmm.toFloat(&ok);
    float zDepthMM = zPhantomSize/2.0;
    // ----------------------------------------------
/*
    QString MCVariance = "0.01";
    QString voxelSize = "0.5";
    QString nX = "81";
    QString nY = "81";
    QString nZ = "80";
    QString xCenter = "20.25";
    QString yCenter = "20.25";
    QString zCenter = "0.0";
    QString density = "1.0";
    QString xDepth = "20.25";
    QString yDepth = "20.25";
    QString zDepth = "20.00";
*/
    float STDev = ui->lineEditSTDev->text().toFloat(&ok) * 0.01;
   if (ui->comboBoxVoxels->currentText() == "4") STDev *= 2.0;
   if (ui->comboBoxVoxels->currentText() == "16") STDev *= 4.0;

    QString MCVariance = "";
    MCVariance.sprintf("%f", STDev);

    QString density = "1.0";
    QString Mode = "Mono";

    QString voxelSize = "";
    QString nX = "";
    QString nY = "";
    QString nZ = "";
    QString xCenter = "";
    QString yCenter = "";
    QString zCenter = "";
    QString xDepth = "";
    QString yDepth = "";
    QString zDepth = "";

    voxelSize.sprintf("%f",VoxelSizeMM/10.0);
    nX.sprintf("%d",nXVoxels);
    nY.sprintf("%d",nYVoxels);
    nZ.sprintf("%d",nZVoxels);
    xCenter.sprintf("%f",xIsocenter/10.0);
    yCenter.sprintf("%f",yIsocenter/10.0);
    zCenter.sprintf("%f",zIsocenter/10.0);
    xDepth.sprintf("%f",xIsocenter/10.0);
    yDepth.sprintf("%f",yIsocenter/10.0);
    zDepth.sprintf("%f",zDepthMM/10.0);

    /*
    if (ui->checkBox4Voxels->isChecked()) {
        MCVariance = "0.02";
         voxelSize = "0.5";
         nX = "80";
         nY = "80";
         nZ = "80";
        xCenter = "20.00";
        yCenter = "20.00";
        zCenter = "0.0";
        xDepth = "20.00";
        yDepth = "20.00";
        zDepth = "20.00";
         if (fs < 3.0 || ui->checkBox2mm->isChecked()) {
              voxelSize = "0.2";
              nX = "200";
              nY = "200";
              nZ = "200";
         }
     }
    */
   for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
      QString plan = *it;
      QString CLN_FILE = MONO_DIR_FS + "/" + plan;
      writeCLN(CLN_FILE, FScm);
      QString VER_FILE = MONO_DIR_FS + "/" + plan;
      QString OPTIONS = "MCvariance = "+MCVariance
                      + "; voxelSize = " + voxelSize
                      + "; nX = " + nX
                      + "; nY = " + nY
                      + "; nZ = " + nZ
                      + "; xCenter = " + xCenter
                      + "; yCenter = " + yCenter
                      + "; zCenter = " + zCenter
                      + "; density = " + density
                      + "; xDepth = " + xCenter
                      + "; yDepth = " + yCenter
                      + "; zDepth = " + zCenter
                      + "; Mode = " + Mode
                      + ";";

      QStringList xZLIST;
      QStringList yZLIST;
      writeVER(VER_FILE, xZLIST, yZLIST, OPTIONS);

      QString BDT_DIR = XVMC_WORK + "/dat/basedata";
      makeDir(BDT_DIR);
      QString BDT_FILE = XVMC_WORK + "/dat/" + mName + ".bdt";

      if (plan.contains("el_con"))
          updateBDT("Electron",pEng); // pEng is Maximum Photon Energy
      else {
          QString pEnergy = plan.section('m',1,1).section('_',0,0)+"."
                          + plan.section('m',1,1).section('_',1,1);
          bool ok;
          QString photoEng;
          photoEng.sprintf("%f",pEnergy.simplified().toFloat(&ok));
          updateBDT("Mono",photoEng);  // Mono-energy Photon Calculation
      }

      QString XVMC_HOME_BDT_DIR = XVMC_HOME + "/dat/basedata";
      makeDir(XVMC_HOME_BDT_DIR);
      QString CMD = "cp -f " + BDT_FILE + " " + XVMC_HOME_BDT_DIR;
      mySystem(CMD);

      QString LBIN = usr->LHOME + "bin";  // It may be same as XVMC_HOME
      CMD = "cd " + MONO_DIR_FS + "; "
#ifdef CMS_SPECT
            + LBIN + "/verify_cms " + plan + " > " + plan + ".log";
#else
            + LBIN + "/verify " + plan + " > " + plan + ".log";
#endif
      mySystem(CMD);

        if (ui->comboBoxVoxels->currentText() == "1") {
           CMD = "cd " + MONO_DIR_FS + "; mv -f " + plan + "_BEAM1_PROFILE1.txt "
             + plan.toUpper() + "_BEAM1_PROFILE1.txt; rm " + plan + "_BEAM* *.p*";
        }
        else {
            int nVoxels = ui->comboBoxVoxels->currentText().toInt(&ok,10);
            mergePDDs(MONO_DIR_FS, plan, nVoxels);
           CMD = "cd " + MONO_DIR_FS + "; rm " + plan + "_BEAM* *.p*";
      }
        mySystem(CMD);

        if (plan.contains("el_con") && ui->checkBoxElectronCorrect->isChecked()) {
            QString logFile = MONO_DIR_FS+"/"+plan + ".log";
            QString TotNHist = readLogfile(logFile,"CMS>doSimulation:getNParticles(0)=");
            QString TotElect = readLogfile(logFile,"CMS>doSimulation:electrons(0)=");
            // QString TotElect = readLogfile(logFile,"CMS>doSimulation:filterfail(0)=");

            float totNHist = TotNHist.simplified().toFloat(&ok);
            float totElect = TotElect.simplified().toFloat(&ok);
            float OFCF = totNHist/totElect; // Output Correction Factor
            scalePDD(MONO_DIR_FS, plan.toUpper(), OFCF);

            CMD = "cd " + MONO_DIR_FS + "; mv -f " + plan + "_BEAM1_PROFILE1.txt "
                 + plan + "_BEAM1_PROFILE1.txt";
            mySystem(CMD);
        }

      // CMD = "cd " + MONO_DIR_FS + "; rm " + plan + "_BEAM* *.p*";
      // mySystem(CMD);
      mySystem("date");

      iProgress += dProgress;
      int p = ui->progressBarMonoMC->value();
      while ( p <= iProgress) ui->progressBarMonoMC->setValue(p++);
      // std::cout << "Progress = " << p << "  iProgress = " << iProgress << endl;
   } // foreach planList

    // Make Progress Bar to 100% at end of progress
   for (int p = ui->progressBarMonoMC->value(); p <= 10000; p++)
      ui->progressBarMonoMC->setValue(p);

    // Check existance of all Mono-energy table as required
   if (checkMonoMC()) {
      writeLocalSetting(mName,"MonoMC","Done");
      ui->groupBoxWFIT->setEnabled(true);
      ui->pushButtonMonoStart->setEnabled(false);
      ui->comboBox1stFS->setEnabled(false);
      ui->comboBox2ndFS->setEnabled(false);
   }
   else {
      ui->groupBoxAFIT->setEnabled(true);
      writeLocalSetting(mName,"MonoMC","NotDone");
      ui->groupBoxWFIT->setEnabled(false);
      ui->comboBox1stFS->setEnabled(true);
      ui->comboBox2ndFS->setEnabled(true);
   }

    // Update buttons
   if (readLocalSetting(mName,"MonoMC") == "Done") {
      if (readLocalSetting(mName,"WaterData") == "Modified") {
            ui->groupBoxWFIT->setEnabled(true); // Next Step
         ui->pushButtonWfit->setEnabled(true);
         ui->pushButtonWfitReview->setEnabled(false);
      } else {
            ui->groupBoxWFIT->setEnabled(false); // Next Step
         ui->pushButtonWfit->setEnabled(false);
         ui->pushButtonWfitReview->setEnabled(false);
      }
   }
}
// -----------------------------------------------------------------------------
void MainConsole::getMachineInfo(QString modelFile) {
   // std::cout << "usr->CurrentModelFile = " << usr->CurrentModelFile << endl;
   QString mName = ui->comboBoxMachine->currentText();
#ifdef REMOVED_Jun162006
   if (modelFile == "") {
      usr->VENDOR=readLocalSetting(mName,"Vendor");
      modelFile = usr->LHOME + mName + "/"
            + readLocalSetting(mName,"ModelFile");
   }
#endif
   modelFile = usr->LHOME + mName + "/" + mName + ".info";
   QFile mFile( modelFile );
   if (mFile.exists()) {
      QTextStream stream( &mFile );
      QString sLine;
      mFile.open( QIODevice::ReadOnly );
      // std::cout << modelFile << " is open" << endl;
      while ( !stream.atEnd() ) {
         sLine = stream.readLine();
         QString strLine = sLine.toLatin1();
         QString keyWord = strLine.section('=',0,0);
         QString keyValueTmp = strLine.section('#',0,0).section('=',1,1);
         QString keyValue = keyValueTmp.simplified();
         // std::cout << "keyWord = " << keyWord << "  keyValue = " << keyValue << endl;
         // if (keyWord.find("MODEL",0,true) != -1)  usr->MODEL = keyValue;
         if (keyWord.count("MODEL",Qt::CaseSensitive) != -1)  usr->MODEL = keyValue;
         if (keyWord.contains("MODEL"))  usr->MODEL = keyValue;
         if (keyWord.contains("VENDOR")) usr->VENDOR = keyValue;
         if (keyWord.contains("CHARGE")) {
                usr->CHARGE = keyValue;
            }
         if (keyWord.contains("E0"))     usr->E0 = keyValue;
         if (keyWord.contains("MLCtype")) usr->MLCtype = keyValue;
         if (keyWord.contains("MAXFW"))  usr->MAXFW = keyValue;
         if (keyWord.contains("AMXFL"))  usr->MAXFL = keyValue;
         if (keyWord.contains("ESCD"))   usr->ESCD = keyValue;
         if (keyWord.contains("SMD"))    usr->SMD = keyValue;
         if (keyWord.contains("SUJD"))   usr->SUJD = keyValue;
         if (keyWord.contains("SLJD"))   usr->SLJD = keyValue;
         if (keyWord.contains("SFD"))    usr->SFD = keyValue;
         if (keyWord.contains("tMLC"))   usr->tMLC = keyValue;
         if (keyWord.contains("UJT"))    usr->UJT = keyValue;
         if (keyWord.contains("LJT"))    usr->LJT = keyValue;
         if (keyWord.contains("rMLC"))   usr->rMLC = keyValue;
         if (keyWord.contains("cMLC"))   usr->cMLC = keyValue;
         if (keyWord.contains("MX"))     usr->MX = keyValue;
         if (keyWord.contains("MY"))     usr->MY = keyValue;
         if (keyWord.contains("oMLC"))   usr->oMLC = keyValue;
         if (keyWord.contains("isMLC"))  usr->isMLC = keyValue;
         if (keyWord.contains("nMLC"))   usr->nMLC = keyValue;
         if (keyWord.contains("iMLC"))   usr->iMLC = keyValue;
         if (keyWord.contains("tkMLC"))  usr->tkMLC = keyValue;
         if (keyWord.contains("thMLC"))  usr->thMLC = keyValue;
         if (keyWord.contains("XIOMLC"))  usr->XIOMLC = keyValue;
         if (keyWord.contains("AVAL"))  usr->AVAL = keyValue;
         if (keyWord.contains("ZVAL"))  usr->ZVAL = keyValue;
      }
   }
   mFile.close();
    if (usr->CHARGE == "0" || usr->CHARGE == "") usr->particle = "Photon";
}
// -----------------------------------------------------------------------------
void MainConsole::get10x10PDD() {
   // Check machine directory exists in Loacl Home like Monaco
   QString mName = ui->comboBoxMachine->currentText();
   QString WORK_DIR = usr->LHOME + mName; // It may be same as XVMC_WORK
   if (!isThereDir(WORK_DIR)) makeDir(WORK_DIR);
   bool ok;
   QString Energy =
      readLocalSetting(mName,"Energy");
   QString Eng;
   Eng.sprintf("%02d",(int)Energy.toFloat(&ok));

   QString WFIT_DIR = WORK_DIR + "/wfit";
   if (!isThereDir(WFIT_DIR)) makeDir(WFIT_DIR);

   QString FS1 = ui->comboBox1stFS->currentText();
   QString FS2 = ui->comboBox2ndFS->currentText();

#ifdef XVMC
   QString WFIT_DIR_X = WORK_DIR + "/wfit/x"+Eng;
   if (!isThereDir(WFIT_DIR_X)) makeDir(WFIT_DIR_X);
#else
   // Get Field Size involved in MWFIT
   QString FS1INT;
   if (FS1 != "None") FS1INT.sprintf("%d", FS1.toInt(&ok,10)/10);

   QString WFIT_DIR_FS1 = WORK_DIR + "/wfit/"+FS1INT+"x"+FS1INT;
   if (!isThereDir(WFIT_DIR_FS1)) makeDir(WFIT_DIR_FS1);

   QString FS2INT;
   if (FS2 != "None") FS2INT.sprintf("%d", FS2.toInt(&ok,10)/10);
   // Check afit directory exists

   QString WFIT_DIR_FS2 = WORK_DIR + "/wfit/"+FS2INT+"x"+FS2INT;
   if (!isThereDir(WFIT_DIR_FS2)) makeDir(WFIT_DIR_FS2);
#endif

   QString SSD = "1000"; // 1000 mm

   QString TELE_DIR = ui->lineEditTeleDir->text();
   QString OP_FILE = TELE_DIR +"/"+mName+"/op";
   QString FS = getOutputFactor(OP_FILE,0,0.0);  // Field Size
   QString OF = getOutputFactor(OP_FILE,1,0.0);  // Output Factor
   QStringList FSList = FS.split("|",QString::SkipEmptyParts);
   QStringList OFList = OF.split("|",QString::SkipEmptyParts);

   // std::cout << "usr->REFDEPTH = " << usr->REFDEPTH << endl;
   // std::cout << "usr->SAD = " << usr->SAD << endl;
   float ssd = usr->SAD.toFloat(&ok) - usr->REFDEPTH.toFloat(&ok)*10;
   SSD.sprintf("%d",(int)ssd);
   usr->SSD = SSD;

   QString OF1;
   QString OF2;
   int iFS = 0;
   for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it) {
     QString FieldSize = *it;
     QString FieldWidth = FieldSize.section('x',0,0).section('.',0,0);
     if (FieldWidth.simplified() == FS1) OF1 = OFList[iFS];
     if (FieldWidth.simplified() == FS2) OF2 = OFList[iFS];
     iFS++;
   }

   // std::cout << "OF1 = " << OF1 << endl;
   // std::cout << "OF2 = " << OF2 << endl;
   // Check pr0 file exists
   if (FS1 != "None") {
     QString FW = FS1;  // 100 mm or 104 mm
     QString FL = FS1;  // 100 mm or 104 mm

     QString PR = TELE_DIR+"/"+mName+"/meas/pr"+SSD+"."+FW+"."+FL;
     // std::cout << "PR = " << PR << endl;

     QFile prFile(PR);
     if (prFile.exists()) {
#ifdef XVMC
      QString zFileName = WFIT_DIR_X + "/meas" + Eng + "mv.dd";
#else
      QString zFileName = WFIT_DIR_FS1 + "/measur.dd";
#endif

      float factor = 100.0 * OF1.toFloat(&ok);
      float normPoint = 100.0;  //  (in mm) will be used in this case of iOpt = 11
      float xOffset = ui->floatSpinBoxOffset->text().toFloat(&ok);
#ifdef XVMC
      int   iOpt = 10;   // Normalize at xp and Multiply the Factor
      factor = 100.0;
#else
      int   iOpt = 11;   // Normalize at the xOffset and Multiply the Factor
#endif
      std::cout << "zFileNname = " << zFileName.toStdString() << endl;
      getPDD(PR,zFileName,normPoint,factor,xOffset,iOpt);
     }
   }
#ifndef XVMC
   // Check pr0 file exists
   if (FS2 != "None") {
     QString FW = FS2;  // 30 mm
     QString FL = FS2;  // 30 mm
     QString PR = TELE_DIR+"/"+mName+"/meas/pr"+SSD+"."+FW+"."+FL;
     QFile prFile(PR);
     if (prFile.exists()) {
      QString zFileName = WFIT_DIR_FS2 + "/measur.dd"; // Apr 21, 2006 for new mwfit

      float factor = 100.0 * OF2.toFloat(&ok);
      float normPoint = 100.0;  // (in mm) will be used in this case of iOpt = 10
      float xOffset = ui->floatSpinBoxOffset->text().toFloat(&ok);
      int   iOpt = 11;   // Normalize at Y Max and Multiply the Factor
      getPDD(PR,zFileName,normPoint,factor,xOffset,iOpt);
     }
   }
#endif
   QString WFIT_INP = WFIT_DIR + "/wfit.inp";
   if (!isThereFile(WFIT_DIR, "wfit.inp")) {
      QFile oFile(WFIT_INP);
      oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
      QTextStream oStream( &oFile );

      QString tmpStr;
#ifdef XVMC
      oStream << "version:  1.0" << endl;
#else
      oStream << "version:  4.0" << endl;
#endif
      oStream << "energy:   " << Energy << endl;
      oStream << "norm:     1550   1" << endl;
      oStream << "lval:      3.5   1" << endl;
      oStream << "bval:     0.45   1" << endl;
      oStream << "aval:     9.0    0" << endl;
      oStream << "zval:     0.5    0" << endl;
      oStream << "Emin:     0.25   0" << endl;
      oStream << "Emax:     " << Energy <<   "   1" << endl;
      oStream << "pcon:     0.005  1" << endl;
#ifndef XVMC
      if (FS1 != "None") oStream << "dir:     " + FS1INT+"x"+FS1INT << endl;
      if (FS2 != "None") oStream << "dir:     " + FS2INT+"x"+FS2INT << endl;
      oStream << "econ:   EL_CON_BEAM1_PROFILE1.txt" << endl;
#endif
      oFile.close();
   }
    ui->pushButtonWfit->setEnabled(true);
    ui->pushButtonWfitReview->setEnabled(false);
    writeLocalSetting(mName,"WaterData","Modified");
     // std::cout << "get10x10PDD() is normally done" << endl;
}
// -----------------------------------------------------------------------------
void MainConsole::runWaterFit() {
   writeOffset();
   get10x10PDD(); // get 10x10 PDD from XiO file
   QString mName = ui->comboBoxMachine->currentText();
   if (!checkMonoMC()) {
      QMessageBox::information( this,
                  "No Mono-energy Monte Carlo PDDs exist\n",
                  "Please check the data and calculate Mono-energy PDD Table!" );
      ui->pushButtonWfit->setEnabled(false);
      ui->pushButtonWfitReview->setEnabled(false);
   } else {
      // Check machine directory exists in Loacl Home like Monaco
      QString WORK_DIR = usr->LHOME + mName;
      if (!isThereDir(WORK_DIR)) makeDir(WORK_DIR);
      // Check afit directory exists

      QString WFIT_DIR = WORK_DIR + "/wfit";
      if (!isThereDir(WFIT_DIR)) makeDir(WFIT_DIR);

      QString Energy = readLocalSetting(mName,"Energy");

      bool ok;
      QString EE;
      EE.sprintf("%02d",(int)Energy.toFloat(&ok));

      QString pEng = Energy.section('.',0,0);
#ifndef XVMC
      QString FS1 = readLocalSetting(mName,"FS1");
      QString FS2 = readLocalSetting(mName,"FS2");

      QString FS1INT;
      QString FS2INT;
      if (FS1 != "None") FS1INT.sprintf("%d", FS1.toInt(&ok,10)/10);
      if (FS2 != "None") FS2INT.sprintf("%d", FS2.toInt(&ok,10)/10);

      QString PDD_DIR_FS1 = WFIT_DIR + "/" + FS1INT+"x"+FS1INT;
      makeDir(PDD_DIR_FS1);

      QString PDD_DIR_FS2 = WFIT_DIR + "/" + FS2INT+"x"+FS2INT;
      if (FS2 != "None") makeDir(PDD_DIR_FS2);
#endif

#ifdef XVMC
      QString PDD_DIR_X = WORK_DIR + "/wfit/x"+EE;
      if (!isThereDir(PDD_DIR_X)) makeDir(PDD_DIR_X);
      QString MONO_DIR_X = WORK_DIR + "/" + mName + "_MONO/10x10";
#else
      // Copy Mono-energy table
      QString MONO_DIR_FS1 = WORK_DIR + "/" + mName + "_MONO/" + FS1INT+"x"+FS1INT;
      QString MONO_DIR_FS2 = WORK_DIR + "/" + mName + "_MONO/" + FS2INT+"x"+FS2INT;
#endif
      QStringList planList;
    if(pEng.simplified().toFloat(&ok) < 10.0) {
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "el_con";
    } else if (pEng.simplified().toFloat(&ok) < 15.0){
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
                << "m12_00" << "m13_00" << "m14_00" << "m15_00"
                << "el_con";
        } else if (pEng.simplified().toFloat(&ok) < 20.0){
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
                << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
                << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
                << "m22_00" << "m23_00" << "m24_00" << "m25_00"
                << "el_con";
    } else {
        planList << "m00_25" << "m00_50" << "m00_75" << "m01_00" << "m01_25"
                << "m01_50" << "m01_75" << "m02_00" << "m02_50" << "m03_00"
                << "m03_50" << "m04_00" << "m04_50" << "m05_00" << "m06_00"
                << "m07_00" << "m08_00" << "m09_00" << "m10_00" << "m11_00"
                << "m12_00" << "m13_00" << "m14_00" << "m15_00" << "m16_00"
                << "m17_00" << "m18_00" << "m19_00" << "m20_00" << "m21_00"
                << "m22_00" << "m23_00" << "m24_00" << "m25_00" << "m26_00"
                << "m27_00" << "m28_00" << "m29_00" << "m30_00"
                << "el_con";
    }
#ifdef XVMC
      float zc[5000], dc[5000], ec[5000];
      float zp[5000], dp[5000], ep[5000];
      float zs[5000], ds[5000], es[5000];
#endif
      for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
         QString plan = *it;
#ifdef XVMC
         QString fName1 = MONO_DIR_X + "/" + plan + "_p.pz0";
         // QString CMD = "cp -f " + fName1 + " " + PDD_DIR_X;
         QString fName2 = MONO_DIR_X + "/" + plan + "_s.pz0";
         // CMD = "cp -f " + fName2 + " " + PDD_DIR_X;
         // std::cout << CMD << endl;

     ifstream p_file;
     p_file.open(fName1.toStdString().c_str(),ios::in);
     if (p_file.bad()) {
            std::cout << "ERROR: Primary PZ file in runWaterFit" << endl;
        exit (-1);
     }
     char comment[256] = ""; // Comment
     char xLabel[256] = "";  // string for x label
     char yLabel[256] = "";  // string for y label
     char zLabel[256] = "";  // string for z label
     char xPos[256] = "";
     char yPos[256] = "";

     char line0[256] = "";  // lines to read from file
     p_file.getline(line0,sizeof(line0));
     istringstream line_stream0(line0);
     line_stream0 >> comment >> xLabel >> xPos;
     // std::cout << "Line0= " << comment << xLabel << xPos << endl;

     char line1[256] = "";  // lines to read from file
     p_file.getline(line1,sizeof(line1));
     istringstream line_stream1(line1);
     line_stream1 >> comment >> yLabel >> yPos;
     // std::cout << "Line1= " << comment << yLabel << yPos << endl;

     char line2[256] = "";  // lines to read from file
     p_file.getline(line2,sizeof(line2));
     istringstream line_stream2(line2);
     line_stream2 >> comment >> zLabel;
     // std::cout << "Line2= " << comment << zLabel << endl;

         int ic = 0;
     while (!p_file.eof()) {
            char line[256] = "";  // lines to read from file
            p_file.getline(line,sizeof(line));
            istringstream line_streamc(line);
            line_streamc >> zp[ic] >> dp[ic] >> ep[ic];
            ic++;
         }
         int nc = ic-2;
         p_file.close();

     if (plan.contains("el_con")) fName2 = fName1;
     ifstream s_file;
     s_file.open(fName2.toStdString().c_str(),ios::in);
     if (s_file.bad()) {
            std::cout << "ERROR: Scatter PZ file in runWaterFit" << endl;
        exit (-1);
     }

     s_file.getline(line0,sizeof(line0));
     s_file.getline(line1,sizeof(line1));
     s_file.getline(line2,sizeof(line2));

         ic = 0;
     while (!s_file.eof()) {
            char line[256] = "";  // lines to read from file
            s_file.getline(line,sizeof(line));
            istringstream line_streamc(line);
            line_streamc >> zs[ic] >> ds[ic] >> es[ic];
        ic++;
         }
         s_file.close();

         QString qP0 = ui->lineEditP0Air->text();
     float p0 = qP0.toFloat(&ok);
     if (plan.contains("el_con")) p0 = 0.5;
     usr->p0 = qP0;

         QString fName0 = PDD_DIR_X + "/" + plan + ".pz0";

     QFile oFile(fName0);
     oFile.remove();
     oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
     QTextStream oStream( &oFile );
         oStream << "#  " << xLabel << "    " << xPos << endl;
         oStream << "#  " << yLabel << "    " << yPos << endl;
         oStream << "#  Z" << endl;
     for (int i=0; i<=nc; i++) {
        zc[i] = zp[i];
        dc[i] = dp[i] * p0 + ds[i] * (1.0-p0);
        ec[i] = ep[i] * p0 + es[i] * (1.0-p0);
        // std::cout << i << "  " << dp[i] << "  " << ds[i] << "  " << dc[i] << endl;
            oStream << zc[i] << "  "  << dc[i] << "  " << ec[i] << endl;
     }
     oFile.close();

#else
         QString fName1 = MONO_DIR_FS1 + "/" + plan.toUpper() + "_BEAM1_PROFILE1.txt";
         QString CMD = "cp -f " + fName1 + " " + PDD_DIR_FS1;

         // std::cout << CMD << endl;
         QFile oFile1( fName1 );
         if (oFile1.exists()) {
            mySystem(CMD);
         }
#endif

#ifndef XVMC
         if (FS2 != "None") {
           QString fName2 = MONO_DIR_FS2 + "/" + plan.toUpper() + "_BEAM1_PROFILE1.txt";
           QString CMD = "cp -f " + fName2 + " " + PDD_DIR_FS2;
           QFile oFile2( fName2 );
           if (oFile2.exists()) {
              mySystem(CMD);
           }
         }
#endif
      } // foreach planList
      // Executes the command
      //  std::cout << CMD << endl;
      ui->pushButtonWfitReview->setEnabled(false);

      QString CMD = "cd " + WFIT_DIR + "; "
#ifdef CMS_SPECT
                  + usr->LHOME + "bin/waterfit_cms.exe";
#elif XVMC
                  + usr->LHOME + "bin/waterfit_xvmc.exe";
#else
                  + usr->LHOME + "bin/waterfit.exe";
#endif
      mySystem(CMD);
      updateWaterFitValues();
        updateBdtValues();
   }
   // Check if the wfit have been done by using waterfit.exe
   if (readLocalSetting(mName,"WFIT") == "Done") {
      ui->pushButtonWfitReview->setEnabled(true);
        ui->pushButtonAdjustNorm->setFlat(true);
      usr->isWfitDone = true;
      QString ddName = usr->LHOME + mName + "/wfit/results.diff";
      QFile ddFile( ddName );
      QString ddNameTmp = usr->LHOME + mName + "/wfit/results.diff.tmp";
      QFile ddFileTmp( ddNameTmp );
      if (ddFile.exists() || ddFileTmp.exists()) {
         ui->pushButtonWfit->setEnabled(false);
         ui->pushButtonWfitReview->setEnabled(true);
         ui->groupBoxVerification->setEnabled(true);
         ui->groupBoxMCPB->setEnabled(true);
         ui->pushButtonPBMC->setEnabled(true);
      } else {
         ui->pushButtonWfit->setEnabled(true);
         ui->pushButtonWfitReview->setEnabled(false);
         ui->groupBoxVerification->setEnabled(false);
         ui->groupBoxMCPB->setEnabled(false);
         ui->pushButtonPBMC->setEnabled(false);
      }
   } else {
      ui->pushButtonWfit->setEnabled(true);
      ui->pushButtonWfitReview->setEnabled(false);
      ui->groupBoxVerification->setEnabled(false);
      ui->groupBoxMCPB->setEnabled(false);
      ui->pushButtonPBMC->setEnabled(false);
   }
}
/*
// -----------------------------------------------------------------------------
void MainConsole::plotWaterFit() {
   // Check machine directory exists in Loacl Home like Monaco
   QString mName = ui->comboBoxMachine->currentText();
   QString WORK_DIR = usr->LHOME + mName;
   if (!isThereDir(WORK_DIR)) makeDir(WORK_DIR);
   // Check afit directory exists
   QString WFIT_DIR = WORK_DIR + "/wfit";
   if (!isThereDir(WFIT_DIR)) makeDir(WFIT_DIR);

   // If Status/MCMONO is not "Done"
   QString wfitInp = WFIT_DIR+ "/wfit.inp";
   QFile wfitInpFile(wfitInp);
   QString wfitOut = WFIT_DIR + "/wfit.out";
   QFile wfitOutFile(wfitOut);
   QString ddName = WFIT_DIR + "/results.diff";
   QFile ddFile(ddName);
   QString ddNameTmp = WFIT_DIR + "/results.diff.tmp";
   QFile ddFileTmp(ddNameTmp);
   if (ddFileTmp.exists()) {
      QString CMD = "cp -f " + ddNameTmp + " " + ddName;
      mySystem(CMD);
   }
   if (wfitInpFile.exists() && wfitOutFile.exists() && ddFile.exists()) {
      QString CMD = "cd " + WFIT_DIR + "; "
#ifdef CMS_SPECT
                  + usr->LHOME + "bin/waterfit_cms.exe";
#elif XVMC
                  + usr->LHOME + "bin/waterfit_xvmc.exe";
#else
                  + usr->LHOME + "bin/waterfit.exe";
#endif
      mySystem(CMD);
      writeLocalSetting(mName,"WFIT", "Done");
        updateBdtValues();
   }
   // Check if the wfit have been done by using waterfit.exe
   if (readLocalSetting(mName,"WFIT") == "Done") {
      ui->pushButtonWfitReview->setEnabled(true);
      usr->isWfitDone = true;
      QString ddName = usr->LHOME + mName + "/wfit/results.diff";
      QFile ddFile( ddName );
      QString ddNameTmp = usr->LHOME + mName + "/wfit/results.diff.tmp";
      QFile ddFileTmp( ddNameTmp );
      if (ddFile.exists() || ddFileTmp.exists()) {
         ui->pushButtonWfit->setEnabled(false);
         ui->pushButtonWfitReview->setEnabled(true);
         ui->groupBoxVerification->setEnabled(true);
         ui->groupBoxMCPB->setEnabled(true);
         ui->pushButtonPBMC->setEnabled(true);
      } else {
         ui->pushButtonWfit->setEnabled(true);
         ui->pushButtonWfitReview->setEnabled(false);
         ui->groupBoxVerification->setEnabled(false);
         ui->groupBoxMCPB->setEnabled(false);
      }
   } else {
      ui->pushButtonWfit->setEnabled(true);
      ui->pushButtonWfitReview->setEnabled(false);
      ui->groupBoxVerification->setEnabled(false);
      ui->groupBoxMCPB->setEnabled(false);
   }
}
*/
// -----------------------------------------------------------------------------
void MainConsole::runVerification() {
   QString mName = ui->comboBoxMachine->currentText();
   // std::cout << "runVerification: mName = " << mName << endl;
   // Setting XVMC_HOME and XVMC_WORK
   setenv("XVMC_WORK", (usr->LHOME+mName).toLatin1(), 1);

   // writeLocalSetting(mName,"RSD", ui->floatSpinBoxRSD->text());
   writeRSD();
   writeNU();
   writeWaterOpt();
   writeOffAxisOpt();

   QString XVMC_WORK = (usr->LHOME + mName).toLatin1();
   QString BDT_DIR = XVMC_WORK + "/dat/basedata";
   QString BDT_DIR_FILE = XVMC_WORK + "/dat/basedata/"+ mName + ".bdt";
   QString BDT_FILE = XVMC_WORK + "/dat/" + mName + ".bdt";


   bool ok;
   float rsd = -10.0 * ui->floatSpinBoxRSD->text().toFloat(&ok);
   vmc->RSD.setNum(rsd,'f',2);
   //  std::cout << CMD << endl;
   ui->pushButtonVerificationReview->setEnabled(false);
   QString Energy = readLocalSetting(mName,"Energy");
   // updateBDT("Spectrum", "Max");
    usr->ID = "1"; // Photon Calculation
    writeBDT(BDT_FILE);
    writeBDT(BDT_DIR_FILE);
   QString WORK_DIR = usr->LHOME + mName;
   QString PT_NAME = mName;
   QString PT_DIR = WORK_DIR + "/" + PT_NAME;
   QString planList = getWaterData(PT_NAME);

   QString Opt = "";
   if (ui->checkBoxOffAxis->isChecked()) Opt = " -opt OffAxis ";

   QString CMD = "cd " + PT_DIR + "; "
#ifdef CMS_SPECT
                + usr->LHOME + "bin/duallistbox_cms.exe -l \""
#elif XVMC
                + usr->LHOME + "bin/duallistbox_xvmc.exe " + Opt + "-l \""
#else
                + usr->LHOME + "bin/duallistbox.exe -l \""
#endif
                + planList + "\" -d";
   // QTextStream (stdout) << "CMD = " << CMD << endl;
   mySystem(CMD);

  QString gridSizeSmall = "";
  gridSizeSmall.sprintf("%d",ui->comboBoxGridSizeSmall->currentText().toInt());
  QString gridSizeLarge = "";
  gridSizeLarge.sprintf("%d",ui->comboBoxGridSizeLarge->currentText().toInt());
  writeLocalSetting(mName,"smallGridSize",gridSizeSmall);
  writeLocalSetting(mName,"largeGridSize",gridSizeLarge);

   ui->pushButtonVerificationReview->setEnabled(true);
   writeLocalSetting(mName,"Verification", "Done");
}
// -----------------------------------------------------------------------------
void MainConsole::plotVerification() {
 QString LBIN = usr->LHOME + "bin";
 QString mName = ui->comboBoxMachine->currentText();
 bool ok;
 QString qsFactor = ui->lineEditOFAdjust->text();
 float fFactor=1.0/qsFactor.toFloat(&ok);
 if (ui->checkBoxOFAdjust->isChecked()) qsFactor.sprintf("%8.5f", fFactor);
 else qsFactor = "1.000";
 QString CMD =  "cd " + usr->LHOME + mName + "/" + mName + ";"
#ifdef XVMC
               + LBIN + "/plotComp_xvmc.exe -d \"calc\" -f " + qsFactor;
#else
               + LBIN + "/plotComp.exe -d \"calc\" -f " + qsFactor;
#endif
 mySystem(CMD);
}
// -----------------------------------------------------------------------------
void MainConsole::runPBVerification() {
   QString mName = ui->comboBoxMachine->currentText();
   QString SSDcm = ui->comboBoxSSD->currentText();  // Current PBComm SSD and Grid Size
   // Setting XVMC_HOME and XVMC_WORK
   QString XVMC_WORK = usr->LHOME + mName;
   setenv("XVMC_WORK", XVMC_WORK.toLatin1(), 1);
   // std::cout << XVMC_WORK << endl;
   bool ok;
   float rsd = -10.0 * ui->floatSpinBoxRSD->text().toFloat(&ok);
   vmc->RSD.setNum(rsd,'f',2);
   //  std::cout << CMD << endl;
   ui->pushButtonPBReview->setEnabled(false);
   QString Energy = readLocalSetting(mName,"Energy");

   QString XVMC_HOME = usr->LHOME+"bin";
   QString XVMC_HOME_BDT_DIR = XVMC_HOME + "/dat/basedata";
   QString RPB_DIR = XVMC_WORK + "/dat/basedata";
   makeDir(RPB_DIR);
   QString RPB_FILE = XVMC_WORK + "/dat/" + mName + "_" + SSDcm + ".RPB"; // Feb 1, 2007 Changed by JOKim
   QFile RPBFile(RPB_FILE);
    if (RPBFile.exists()) {
    QString PT_NAME = "PB_" + SSDcm;
    QString PT_DIR = XVMC_WORK + "/" + PT_NAME;
    makeDir(PT_DIR);
    QString QuickCMD = "cp " + RPB_FILE + " " + XVMC_HOME_BDT_DIR + "/" + mName + ".RPB";
    //std::cout << QuickCMD << endl;
    mySystem(QuickCMD);
    QuickCMD = "cp " + RPB_FILE + " " + PT_DIR + "/" + mName + ".RPB";
    mySystem(QuickCMD);

    QString planList = getWaterData(PT_NAME);

    QString CMD = "cd " + PT_DIR + "; "
#ifdef CMS_SPECT
                 + usr->LHOME + "bin/duallistbox_cms.exe -l \""
#elif XVMC
                 + usr->LHOME + "bin/duallistbox_xvmc.exe -l \""
#else
                 + usr->LHOME + "bin/duallistbox.exe -l \""
#endif
                 + planList + "\" -d";
    mySystem(CMD);
    ui->pushButtonPBReview->setEnabled(true);
    writeLocalSetting(mName,"PBVerification", "Done");
        updatePBInfo();
    }
}
// -----------------------------------------------------------------------------
void MainConsole::plotPBVerification() {
 QString LBIN = usr->LHOME + "bin";
 QString mName = ui->comboBoxMachine->currentText();
 QString SSDcm = "_" + ui->comboBoxSSD->currentText();
 if (ui->tabWidget->currentIndex()==5) SSDcm = "_" + ui->comboBoxPBSSD->currentText();
 QString CMD =  "cd " + usr->LHOME + mName + "/PB" + SSDcm + ";"
#ifdef XVMC
               + LBIN + "/plotComp_xvmc.exe -d \"calc\" -f \"1.000\"";
#else
               + LBIN + "/plotComp.exe -d \"calc\" -f \"1.000\"";
#endif
 mySystem(CMD);
}
// -----------------------------------------------------------------------------
void MainConsole::plotPBMCComparison() {
 QString LBIN = usr->LHOME + "bin";
 QString mName = ui->comboBoxMachine->currentText();
 QString SSDcm = "_" + ui->comboBoxSSD->currentText();
 if (ui->tabWidget->currentIndex()==5) SSDcm = "_" + ui->comboBoxPBSSD->currentText();
 QString CMD =  "cd " + usr->LHOME + mName + "/PB" + SSDcm + ";"
#ifdef XVMC
               + LBIN + "/plotPBvsMC_xvmc.exe -d \"calc\"";
#else
               + LBIN + "/plotPBvsMC.exe -d \"calc\"";
#endif
 mySystem(CMD);
}
// -----------------------------------------------------------------------------
void MainConsole::plotPBvsPBMC() {
 QString LBIN = usr->LHOME + "bin";
 QString mName = ui->comboBoxMachine->currentText();
 QString SSDcm = "_" + ui->comboBoxSSD->currentText();
 if (ui->tabWidget->currentIndex()==5) SSDcm = "_" + ui->comboBoxPBSSD->currentText();
 QString CMD =  "cd " + usr->LHOME+mName+";"
               + LBIN + "/plotPBvsPBMC.exe -d " + SSDcm.section('_',1,2);
 // std::cout << CMD << endl;
 mySystem(CMD);
}
// -----------------------------------------------------------------------------
void MainConsole::runPBMCxvmc() {  // Maybe for XVMC

 makePBVMC();

 QString mName = ui->comboBoxMachine->currentText();
 QString MC_DIR = usr->LHOME + mName + "/" + mName;
 QString MC_DIR_INP = MC_DIR + "/INPUT";

 QString XVMC_WORK = usr->LHOME+mName;
 QString XVMC_HOME = usr->LHOME+"bin";
 setenv("MACHINE",mName.toLatin1(),1);

 ui->progressBarPBMC->setMaximum(37000);
 ui->progressBarPBMC->reset();
 int iProgress = 0;

 QStringList planList;
#ifdef PHANTOM_40x40x30
 // planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
 planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#else
 planList << "2" << "4" << "6" << "8" << "10" << "15" << "20";
#endif
 QString patient = mName;

 QString qExe = "xvmc";
 if (ui->checkBoxOffAxis->isChecked()) qExe = "xvmc_offaxis";

 bool ok;
 for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
  QString plan = *it;
  QString LBIN = XVMC_HOME;
  QString CMD = "cp -f " + MC_DIR_INP + "/PB" + plan + "x" + plan + ".vmc "
        + MC_DIR + "; "
        + XVMC_HOME + "/" + qExe + " " + patient + " PB" + plan + "x" + plan
        + " > " + MC_DIR + "/PB" + plan + "x" + plan + ".log ";
  // std::cout << CMD << endl;

  mySystem(CMD);

  iProgress += (int) (100*pow(plan.toFloat(&ok)/2.0,2.0));
  int p = ui->progressBarPBMC->value();
  while ( p <= iProgress) ui->progressBarPBMC->setValue(p++);
  // std::cout << "Progress = " << p << "  iProgress = " << iProgress << endl;
 } // foreach planList
 for (int p = ui->progressBarPBMC->value(); p <= 37000; p++)
      ui->progressBarPBMC->setValue(p);

 // Needs something to conver XVMC to VERIFY for PBComm run

 if (checkPBMC()) {
  writeLocalSetting(mName,"PBMC", "Done");
 }
 else {
  writeLocalSetting(mName,"PBMC", "NotDone");
 }
}
// -----------------------------------------------------------------------------
void MainConsole::runPBMC() {
 // Virtual Measurement
 QString mName = ui->comboBoxMachine->currentText();
 QString MC_DIR = usr->LHOME + mName + "/" + mName;
 QString MC_DIR_INP = MC_DIR + "/INPUT";

 QString XVMC_WORK = usr->LHOME+mName;
 QString XVMC_HOME = usr->LHOME+"bin";
 setenv("MACHINE",mName.toLatin1(),1);

 QString CLN_FILE = MC_DIR_INP + "/D6MV";
 QFile cFile(CLN_FILE+".CLN");
 if (!cFile.exists()) writeCLN(CLN_FILE,"");

 QString VER_FILE = MC_DIR_INP + "/D6MV";
 QString OPTIONS = "";
 QStringList xZLIST;
 QStringList yZLIST;
 QFile vFile(VER_FILE+".VER");
 if (!vFile.exists()) writeVER(VER_FILE, xZLIST, yZLIST, OPTIONS);

 QString BDT_DIR = XVMC_WORK + "/dat/basedata";
 makeDir(BDT_DIR);
 QString BDT_FILE = XVMC_WORK + "/dat/" + mName + ".bdt";
 // second 1.4 should be modified to 1.5 for new energy spectrum in Verify
 QString CMD = "cat " + BDT_FILE
     + " | sed s/'BASE-DATA-FILE-VERSION:  1.4'/'BASE-DATA-FILE-VERSION:  1.4'/g > "
     + BDT_DIR + "/" + mName + ".bdt";
  // QString CMD = "cp -f " + BDT_FILE + " " + BDT_DIR;
  mySystem(CMD);

  // CMD = "ln -sf " + BDT_DIR + " " + XVMC_HOME + "/dat";
  // mySystem(CMD);

  QString XVMC_HOME_BDT_DIR = XVMC_HOME + "/dat/basedata";
  makeDir(XVMC_HOME_BDT_DIR);
  // QString CMD = "cp -f " + BDT_FILE + " " + BDT_DIR;
  CMD = "cp -f " + BDT_FILE + " " + XVMC_HOME_BDT_DIR;
  mySystem(CMD);

  CMD = "cp -f " + CLN_FILE + ".CLN " + MC_DIR + "/D6MV.CLN";
  mySystem(CMD);

  CMD = "cp -f " + VER_FILE + ".VER " + MC_DIR + "/D6MV.VER";
  mySystem(CMD);
#ifdef CMS_SPECT
  CMD = "cd " + MC_DIR + ";" + XVMC_HOME+"/verify_cms.exe -i D6MV";
#else
  CMD = "cd " + MC_DIR + ";" + XVMC_HOME+"/verify.exe -i D6MV";
#endif
  mySystem(CMD);


 QString VER = MC_DIR + "/D6MV.VER";
 // std::cout << "SSD = " << readVER(VER) << endl;
 QString ISOmm = readVER(VER, "zISO");
 QString GridSize = readVER(VER, "GRIDSIZE");
 bool ok;
 int SSDmm = 1000 - ISOmm.toInt(&ok,10);
 QString SSDcm; SSDcm.sprintf("%d",(int)(SSDmm/10));
 QString PB_DIR = XVMC_WORK + "/PBComm_"+SSDcm+"cm_"+GridSize+"mm";
 makeDir(PB_DIR);

 if (isThereDir(MC_DIR) && isThereDir(PB_DIR) ) {
   QString CMD = "cp -f " + MC_DIR + "/D6MV*.* " + PB_DIR;
   mySystem(CMD);

 if (readLocalSetting(mName,"PBMC") == "Done") {
    ui->pushButtonPBMC->setEnabled(false);
    ui->groupBoxPBComm->setEnabled(true);
    ui->pushButtonPBComm->setEnabled(true);
    ui->pushButtonPBReview->setEnabled(false);
    ui->pushButtonPBPack->setEnabled(false);
    writeLocalSetting(mName, "PBComm", "NotDone");
 } else {
    ui->groupBoxMCPB->setEnabled(true);
    ui->pushButtonPBMC->setEnabled(true);
    ui->groupBoxPBComm->setEnabled(true); // It was false but temporarily true
 }
 }
 // PB SSD Update Here
 updatePBCommInfo();
}
// -----------------------------------------------------------------------------
void MainConsole::runPBMCverification() {
 // Virtual Measurement
 QString mName = ui->comboBoxMachine->currentText();
 QString MC_DIR = usr->LHOME + mName + "/" + mName;
 QString MC_DIR_INP = MC_DIR + "/INPUT";

 QString XVMC_WORK = usr->LHOME+mName;
 QString XVMC_HOME = usr->LHOME+"bin";
 setenv("MACHINE",mName.toLatin1(),1);

 QString SSD = ui->comboBoxSSD->currentText();
 QString PBComm_DIR = usr->LHOME + mName + "/PBComm_"+SSD;
 if (isThereDir(PBComm_DIR)) {

     QString CLN_FILE = MC_DIR_INP + "/D6MV";
     QFile cFile(CLN_FILE+".CLN");
     // if (!cFile.exists()) writePBCLN(CLN_FILE,"");
     writePBCLN(CLN_FILE,"");

     QString VER_FILE = MC_DIR_INP + "/D6MV";
     QString OPTIONS = "Mode=PBMC_";
     QStringList xZLIST;
     QStringList yZLIST;
     QFile vFile(VER_FILE+".VER");
     // if (!vFile.exists()) writeVER(VER_FILE, xZLIST, yZLIST, OPTIONS);
     writeVER(VER_FILE, xZLIST, yZLIST, OPTIONS);

     QString RPB_DIR = XVMC_WORK + "/dat/basedata";
     makeDir(RPB_DIR);
     QString RPB_FILE = XVMC_WORK + "/dat/" + mName +"_" + SSD + ".RPB";

      QString XVMC_HOME_RPB_DIR = XVMC_HOME + "/dat/basedata";
      makeDir(XVMC_HOME_RPB_DIR);
      // QString CMD = "cp -f " + RPB_FILE + " " + RPB_DIR;
      QString CMD = "cp -f " + RPB_FILE + " " + XVMC_HOME_RPB_DIR + "/" + mName + ".RPB";
      mySystem(CMD);
      CMD = "cp -f " + RPB_FILE + " " + MC_DIR + "/" + mName + ".RPB";
      mySystem(CMD);


      CMD = "cp -f " + CLN_FILE + ".CLN " + MC_DIR + "/D6MV.CLN";
      mySystem(CMD);

      CMD = "cp -f " + VER_FILE + ".VER " + MC_DIR + "/D6MV.VER";
      mySystem(CMD);
    #ifdef CMS_SPECT
      CMD = "cd " + MC_DIR + ";" + XVMC_HOME+"/verify_cms.exe -i D6MV";
    #else
      CMD = "cd " + MC_DIR + ";" + XVMC_HOME+"/verify.exe -i D6MV";
    #endif
      mySystem(CMD);


     QString VER = MC_DIR + "/D6MV.VER";
     // std::cout << "SSD = " << readVER(VER) << endl;
     QString ISOmm = readVER(VER, "zISO");
     QString GridSize = readVER(VER, "GRIDSIZE");
     bool ok;
     int SSDmm = 1000 - ISOmm.toInt(&ok,10);
     QString SSDcm; SSDcm.sprintf("%d",(int)(SSDmm/10));
     QString PB_DIR = XVMC_WORK + "/PBMC_"+SSDcm+"cm_"+GridSize+"mm";
     makeDir(PB_DIR);

     if (isThereDir(MC_DIR) && isThereDir(PB_DIR) ) {
    QString CMD = "cp -f " + MC_DIR + "/D6MV*.* " + PB_DIR;
    mySystem(CMD);

     if (readLocalSetting(mName,"PBMC") == "Done") {
     ui->pushButtonPBMC->setEnabled(false);
     ui->groupBoxPBComm->setEnabled(true);
     ui->pushButtonPBComm->setEnabled(true);
     ui->pushButtonPBReview->setEnabled(false);
     ui->pushButtonPBPack->setEnabled(false);
     // writeLocalSetting(mName, "PBComm", "NotDone");
     } else {
     ui->groupBoxMCPB->setEnabled(true);
     ui->pushButtonPBMC->setEnabled(true);
     ui->groupBoxPBComm->setEnabled(true); // It was false but temporarily true
     }
     }
     // PB SSD Update Here
     updatePBCommInfo();
 }
}
// -----------------------------------------------------------------------------
void MainConsole::makePBVMC() {
 // read machine information from .info file
 QString mName = ui->comboBoxMachine->currentText();
 QString modelFile = usr->LHOME + mName + "/" + mName + ".info" ;
 getMachineInfo(modelFile);

 QString MC_DIR = usr->LHOME + mName + "/" + mName;
 QString MC_DIR_INP = MC_DIR + "/INPUT";
 makeDir(MC_DIR);
 makeDir(MC_DIR_INP);

 QString BDT = mName;

 QStringList fsList;
#ifdef PHANTOM_40x40x30
 // fsList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
 fsList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#elif PHANTOM_40x40x40
 // fsList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
 fsList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#else
 fsList << "2" << "4" << "6" << "8" << "10" << "15" << "20";
#endif

 QStringList zDepth;
   zDepth << "0.2" << "0.5" << "1.0" << "2.0" << "3.0" << "4.0" << "5.0" << "6.0"
   << "6.5" << "7.0" << "7.5" << "8.0" << "8.5" << "9.0" << "9.5" << "10.0"
   << "10.5" << "11.0" << "11.5" << "12.0" << "12.5" << "13.0" << "13.5" << "14.0"
   << "14.5" << "15.0" << "15.5" << "16.0" << "16.5" << "17.0" << "17.5" << "18.0"
   << "18.5" << "19.0" << "19.5" << "20.0" << "20.5" << "21.0" << "21.5" << "22.0"
   << "22.5" << "23.0" << "23.5" << "24.0" << "24.5" << "25.0" << "25.5" << "26.0"
   << "27.0" << "28.0" << "29.0"
#ifdef PHANTOM_40x40x40
    << "30.0" << "31.0" << "32.0" << "33.0" << "34.0"
   << "35.0" << "36.0" << "37.0" << "38.0" << "39.0"
#endif
   ;  // Do not remove this line

 QStringList xDepth;
 xDepth << "15.0";
 QStringList yDepth;
 yDepth << "15.0";

 QString DeviceType = "103";  // Polyenergy Photon
 QString matMLC = "tungsten";

 int nMLChalf = 0;
 int iMLChalf = 0;
 int iMLCstart = 0;
 int iMLCend = 0;

 bool ok;
 float uMLC = 0; // upper limit of MLC
 float lMLC = 0; // lower limit of MLC
 float maxFW = usr->MAXFW.toFloat(&ok)/10.0;
 float p1MLC = -0.5; // Closed MLC Position
 float p2MLC =  0.5; // Closed MLC Position
 if (usr->XIOMLC.contains("ElektaBM80leaf")) {
    p1MLC = -maxFW/2.0 - 0.1;
    p2MLC = -maxFW/2.0;
 }

 if (usr->isMLC.compare("1") == 0) {
  nMLChalf = usr->nMLC.toInt(&ok,10)/2;
  iMLChalf = usr->iMLC.toInt(&ok,10)/2;
  iMLCstart = (usr->nMLC.toInt(&ok,10)-usr->iMLC.toInt(&ok,10))/4+1;
  iMLCend = iMLCstart + usr->iMLC.toInt(&ok,10);

  uMLC = usr->SMD.toFloat(&ok) - usr->tMLC.toFloat(&ok)/2; // upper limit of MLC
  lMLC = usr->SMD.toFloat(&ok) + usr->tMLC.toFloat(&ok)/2; // lower limit of MLC
  p1MLC = -0.5; // Closed MLC Position
  p2MLC =  0.5; // Closed MLC Position
   if (usr->XIOMLC.contains("ElektaBM80leaf")) {
      p1MLC = -maxFW/2.0 - 0.1;
      p2MLC = -maxFW/2.0;
   }
 }
 for ( QStringList::Iterator it = fsList.begin(); it != fsList.end(); ++it ) {
  QString fieldsize = *it;
  QString fName = MC_DIR_INP + "/" + "PB" + fieldsize + "x" + fieldsize + ".vmc";
  // std::cout << fName << endl;
  bool ok;
  float fs = fieldsize.toFloat(&ok); // Field Size
  float halfFS = fs/2;
  int halfFSmm = (int)(10*fs/2);

  // Phantom Specifications: Voxel Size
  float xVoxelSize = 0.2;
  float yVoxelSize = 0.2;
  float zVoxelSize = 0.2;
  // Phantom Specifications: Phantom Size
  float xPhantomSize = 30.0;
  float yPhantomSize = 30.0;
  float zPhantomSize = 30.0;
#ifdef PHANTOM_40x40x40
  xPhantomSize = 40.0;
  yPhantomSize = 40.0;
  zPhantomSize = 40.0;
#endif
#ifdef PHANTOM_40x40x30
  xPhantomSize = 40.0;
  yPhantomSize = 40.0;
  zPhantomSize = 30.0;
#endif
  // Phantom Specifications: Number of Voxels
  int nX = (int)(xPhantomSize/xVoxelSize);
  int nY = (int)(yPhantomSize/yVoxelSize);
  int nZ = (int)(zPhantomSize/zVoxelSize);
  // Phantom Specifications: Phantom Center
  float xCenter = xPhantomSize/2;
  float yCenter = yPhantomSize/2;
  //  float zCenter = zPhantomSize/2;
  // float SID = 100.0;
  float SSD = 850.0;  // SSD = 85.0 cm
  float RSD = -10.0;
  // Isocenter
  float zISO = 100 - SSD;
  // Reference Depth
  float zRef = 10.0; // cm

  QString Energy = readLocalSetting(mName,"Energy");

  QFile oFile( fName );
  oFile.remove();
  oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
  QTextStream oStream( &oFile );

  QString tmpStr;
  oStream << "*PHANTOM        | " << endl;
  oStream << "-VOXELSIZE      | ";
  oStream << tmpStr.setNum(xVoxelSize,'f',2) << " ";
  oStream << tmpStr.setNum(yVoxelSize,'f',2) << " ";
  oStream << tmpStr.setNum(zVoxelSize,'f',2) << endl;
  oStream << "-DIMENSION      | ";
  oStream << tmpStr.setNum(nX,10) << " ";
  oStream << tmpStr.setNum(nY,10) << " ";
  oStream << tmpStr.setNum(nZ,10) << endl;
  oStream << "-CHANGE-DENSITY | ";
  oStream << "  1  " << tmpStr.setNum(nX,10) << "   ";
  oStream << "  1  " << tmpStr.setNum(nY,10) << "   ";
  oStream << "  1  " << tmpStr.setNum(nZ,10) << "   1.0000" << endl;
  oStream << "!" << endl;
  oStream << "*GLOBAL-DATA    | " << endl;
  oStream << "-WRITE-3D-DOSE  | 0 " << endl;

  for ( QStringList::Iterator xit = xDepth.begin(); xit != xDepth.end(); ++xit ) {
   for ( QStringList::Iterator zit = zDepth.begin(); zit != zDepth.end(); ++zit ) {
    oStream << "-X-PROFILE      | ";
    oStream << *xit << " ";
    oStream << *zit << " " << endl;
   }
  }
  for ( QStringList::Iterator yit = yDepth.begin(); yit != yDepth.end(); ++yit ) {
   for ( QStringList::Iterator zit = zDepth.begin(); zit != zDepth.end(); ++zit ) {
    oStream << "-Y-PROFILE      | ";
    oStream << *yit << " ";
    oStream << *zit << " " << endl;
   }
  }
  for ( QStringList::Iterator xit = xDepth.begin(); xit != xDepth.end(); ++xit ) {
   for ( QStringList::Iterator yit = yDepth.begin(); yit != yDepth.end(); ++yit ) {
    oStream << "-Z-PROFILE      | ";
    oStream << *xit << " ";
    oStream << *yit << " " << endl;
   }
  }
  oStream << "-XY-PLANE       | " << zPhantomSize/2.0 << endl;
  oStream << "-XZ-PLANE       | " << yPhantomSize/2.0 << endl;
  oStream << "-YZ-PLANE       | " << xPhantomSize/2.0 << endl;
  if (!ui->checkBoxMBSF->isChecked()) {
     oStream << "-DOSE-TYPE      | 4" << endl;
  }
  else {
     oStream << "-DOSE-TYPE      | 5" << endl;
  }
  oStream << "-P0-CUTOFF-KERMA| 0.25 " << endl;
  oStream << "-P1-CUTOFF-KERMA| 2.00 " << endl;
  oStream << "-REFERENCE-POINT| ";
  oStream << tmpStr.setNum(xCenter,'f',2) << " ";
  oStream << tmpStr.setNum(yCenter,'f',2) << " ";
  oStream << tmpStr.setNum(zRef,'f',2) << endl;
  oStream << "-RANDOM-SET     | 23  45  67  89" << endl;
  oStream << "!" << endl;
  oStream << "*BEAM-PARAMETERS| " << endl;
  oStream << "-BEAM-WEIGHT    | 100.0 " << endl;
  oStream << "-DEVICE-TYPE    | " << DeviceType << endl;
  oStream << "-DEVICE-KEY     | " << BDT << endl;
  oStream << "-EVENT-NUMBER   | " << RSD << " 40  1  100" << endl;
  oStream << "-NOMINAL-ENERGY | " << Energy << endl;
  oStream << "-ISOCENTER      | ";
  oStream << tmpStr.setNum(xCenter,'f',2) << " ";
  oStream << tmpStr.setNum(yCenter,'f',2) << " ";
  oStream << tmpStr.setNum(zISO,'f',2) << endl;
  oStream << "-GANTRY-ANGLE   | F  0  360" << endl;
  oStream << "-TABLE-ANGLE    | 0" << endl;
  oStream << "-COLL-ANGLE     | 0" << endl;
  oStream << "-COLL-WIDTH-X   | " << fs << endl;
  oStream << "-COLL-WIDTH-Y   | " << fs << endl;
  //   ---------------------
  if (usr->isMLC.compare("1") == 0) {
   bool ok;
   QString SPACE;
   if (usr->MLCtype.compare("RNDFOCUS-MLC") == 0) SPACE = "   |";
   if (usr->MLCtype.compare("VARIAN-MLC") == 0) SPACE = "     |";
   if (usr->MLCtype.compare("DBLFOCUS-MLC") == 0) SPACE = "   |";
   if (usr->MLCtype.compare("SIMPLE-MLC") == 0) SPACE = "     |";
   if (usr->MLCtype.compare("ELEKTA-MLC") == 0) SPACE = "     |";

   if (usr->MLCtype.compare("VARIAN-MLC") != 0
       && usr->MLCtype.compare("VARIAN-MLC") != 0
       && usr->MLCtype.compare("None") != 0 ) {
    oStream << "-"  << usr->MLCtype << SPACE
      << "  " << usr->nMLC << "  " << usr->oMLC
      << "  " << matMLC << "  " << uMLC
      << "  " << lMLC << "  " << usr->cMLC
      << "  " << usr->rMLC << endl;

    int i = 0;
    float MLCpos = (usr->nMLC.toInt(&ok,10)-usr->iMLC.toInt(&ok,10))/2
         * usr->tkMLC.toFloat(&ok)
         + usr->iMLC.toInt(&ok,10)/2 * usr->thMLC.toFloat(&ok);

    while (i < usr->nMLC.toInt(&ok,10)) {
     float tMLC = usr->tkMLC.toFloat(&ok);
     if (i >= iMLCstart && i < iMLCend) tMLC = usr->thMLC.toFloat(&ok);
     MLCpos = MLCpos - tMLC;
     float tMLCcm = tMLC/10.0;
     if (MLCpos >= halfFSmm) {
      oStream << tMLCcm << " " << p1MLC << "  " <<  p2MLC << endl;
     } else {
      float MLCposPlus = MLCpos + tMLC;
      if (MLCposPlus <= -halfFSmm)
       oStream << tMLCcm << "  " << p1MLC << "  " << p2MLC << endl;
      else
       oStream << tMLCcm << "  " << -halfFS << "  " << halfFS << endl;
      }
       i++;
      }
   }
  }

  oStream << "!" << endl;
  oStream << "*END-INPUT      | " << endl;

  oFile.close();
 } // foreach fsList
}
// -----------------------------------------------------------------------------
bool MainConsole::checkPBMC() {
 QString mName = ui->comboBoxMachine->currentText();
 QString MC_DIR = usr->LHOME + mName + "/" + mName;
 makeDir(MC_DIR);

 QStringList planList;
#ifdef PHANTOM_40x40x30
// planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
 planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#elif PHANTOM_40x40x40
// planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
 planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#else
 planList << "2" << "4" << "6" << "8" << "10" << "15" << "20";
#endif

 for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
  QString plan = *it;
  QString fName = MC_DIR + "/PB" + plan + "x" + plan + ".pz0";
  QFile oFile( fName );
  if (!oFile.exists()) return(false);
 } // foreach planList
 return(true);
}
// -----------------------------------------------------------------------------
void MainConsole::runPBCommOld() {
 bool ok;
 float rsd = -10.0 * ui->floatSpinBoxRSD->text().toFloat(&ok);
 QString RSD;
 RSD.setNum(rsd,'f',2);
 //  std::cout << CMD << endl;
 QString Energy = readLocalSetting(ui->comboBoxMachine->currentText(),"Energy");
 QString CMD = usr->LHOME + "bin/runPBComm.csh "
       + usr->LHOME + " "
       + ui->lineEditTeleDir->text()
       + ui->comboBoxMachine->currentText() + " "
       + ui->comboBoxMachine->currentText() + " "
       + Energy.section('.',0,0) + " " + RSD + " "
       + readLocalSetting(ui->comboBoxMachine->currentText(),"Energy");
 mySystem(CMD);
}
// -----------------------------------------------------------------------------
void MainConsole::runPBComm() {
 QString mName = ui->comboBoxMachine->currentText();

 QString XVMC_WORK = usr->LHOME+mName;
 QString XVMC_HOME = usr->LHOME+"bin";
 setenv("MACHINE",mName.toLatin1(),1);

 QString MC_DIR = XVMC_WORK + "/" + mName;
 QString SSDcm = ui->comboBoxSSD->currentText();
 QString PB_DIR = XVMC_WORK + "/PBComm_"+SSDcm;
 // makeDir(PB_DIR);
 // QString PB_DIR_RST = XVMC_WORK + "/PBComm_"+SSDcm+"/results";
 // makeDir(PB_DIR_RST);

  if (isThereDir(MC_DIR) && isThereDir(PB_DIR) ) {
    // QString CMD = "cp -f " + MC_DIR + "/D6MV*.* " + PB_DIR;
    // mySystem(CMD);

    QString SMT_FILE = XVMC_WORK + "/smooth.inp";
    writeSMOOTH(SMT_FILE);

    QString FIT_FILE = XVMC_WORK + "/fit.inp";
    writeFIT(FIT_FILE);

     QString PBCommEXE = "PBComm";
     if (!ui->checkBoxFixU1->isChecked()) PBCommEXE = "PBComm_Fit";
    if (isThereDir(PB_DIR)) {
       // QString CMD = "cp -R " + PB_DIR_RST + " " + PB_DIR + "/../RPB/";
       // mySystem(CMD);
       QString Energy = readLocalSetting(mName,"Energy");
       QString FitAllFileds = "n";
       if (ui->checkBoxFitAll->isChecked()) FitAllFileds = "y";
       QString CMD = "cd " + XVMC_WORK + "; echo " + Energy.section('.',0,0)
           + " > tmp.dat;" + XVMC_HOME + "/"+PBCommEXE+" PBComm_" + SSDcm + " "
           + FitAllFileds + " < tmp.dat >& PBComm_"+SSDcm+".log; rm tmp.dat";
       // std::cout << CMD << endl;
       mySystem(CMD);
    }
    QString RPB_DIR = XVMC_WORK + "/RPB";
    if (isThereDir(RPB_DIR)) {
         if (isThereDir(RPB_DIR + "_" + SSDcm)) mySystem("rm -rf "+RPB_DIR + "_" + SSDcm);
       QString CMD = "mv " + RPB_DIR + " " + RPB_DIR + "_" + SSDcm;
       mySystem(CMD);
    }

    QString Energy = readLocalSetting(mName,"Energy");
    QString PBBaseData = XVMC_WORK + "/PBBaseData_" + Energy.section('.',0,0)+"MV.RPB";
    QFile RPBFile(PBBaseData);
     if (RPBFile.exists()) {
         QString CMD = "cp " + PBBaseData + " " + XVMC_WORK + "/dat/" + mName + "_" + SSDcm + ".RPB";
         mySystem(CMD);
     }
  }
  writeLocalSetting(mName, "PBComm", "Done");
  if (readLocalSetting(mName,"PBComm") == "Done") {
     ui->groupBoxPBComm->setEnabled(true);
     ui->pushButtonPBComm->setEnabled(true);  // true for for a while
     ui->pushButtonPBReview->setEnabled(true);
     ui->pushButtonPBPack->setEnabled(true);
     writeLocalSetting(mName, "Validate", "NotDone");
  } else {
     ui->groupBoxPBComm->setEnabled(true);
     ui->pushButtonPBComm->setEnabled(true);
     ui->pushButtonPBReview->setEnabled(false);
     ui->pushButtonPBPack->setEnabled(false);
  }
}
// -----------------------------------------------------------------------------
void MainConsole::plotPBComm() {
 QString mName = ui->comboBoxMachine->currentText();
 QString SSDcm = ui->comboBoxSSD->currentText();
 QString LBIN = usr->LHOME + "bin";
 QString CMD = "cd " + usr->LHOME + mName
       + ";" + LBIN + "/plotPB.exe -d PBComm_" + SSDcm;
 mySystem(CMD);
}
// -----------------------------------------------------------------------------
void MainConsole::plotPBCommPara() {
 QString mName = ui->comboBoxMachine->currentText();
 QString mInfoFile = usr->LHOME + mName + "/" + mName + ".info" ;
 getMachineInfo(mInfoFile);
 QString MLCtype = "";
 if (usr->XIOMLC.contains("ElektaBM80leaf")) MLCtype = "elektaModuleaf";
 if (usr->XIOMLC.contains("MILLENNIUM120")) MLCtype = "varian60";
 if (usr->XIOMLC.contains("PHILIPSMLC")) MLCtype = "elekta40";
 if (usr->XIOMLC.contains("SIEMENSV50")) MLCtype = "siemens27";
 if (usr->XIOMLC.contains("VARIAN26")) MLCtype = "varian52";
 if (usr->XIOMLC.contains("VARIAN40")) MLCtype = "varian40";
 if (usr->XIOMLC.contains("OPTIFOCUS82")) MLCtype = "siemens41";
 //std::cout << "usr->XIOMLC = " << usr->XIOMLC << endl;
 //std::cout << "MLCType = " << MLCtype << endl;
 QString SSDcm = ui->comboBoxSSD->currentText();
 QString LBIN = usr->LHOME + "bin";
 QString CMD = "cd " + usr->LHOME + mName + "/RPB_" + SSDcm
       + ";" + LBIN + "/plotRPB.exe -MLCType " + MLCtype;
 mySystem(CMD);
}
// -----------------------------------------------------------------------------
QString MainConsole::getKeyValue(QString strLine) {
   QString keyValue = strLine.section(':',1,1);
   return (keyValue.simplified());
}
// -----------------------------------------------------------------------------
void MainConsole::updateBDT14() {
    /* // REMOVED
   QString DATDIR = usr->LHOME + comboBoxMachine->currentText() + "/dat";
   QString fName = DATDIR + "/" + comboBoxMachine->currentText() + ".bdt";

   // define bool variables
   QString version          = "";      bool version_found       = false;
   QString entry            = "";      bool entry_found         = false;
   QString ptl_type         = "";      bool ptl_type_found      = false;
   QString energy           = "";      bool energy_found        = false;
   QString model_id         = "";      bool model_id_found      = false;
   QString begin_parameter  = "";      bool begin_para_found    = false;
   QString end_parameter    = "";      bool end_para_found      = false;

   // initialization
   QString p_pri            = "";      bool p_pri_found         = false;
   QString p_sct            = "";
   QString pri_distance     = "";      bool pri_distance_found  = false;
   QString sigma_pri        = "";      bool sigma_pri_found     = false;
   QString horn_0           = "";      bool horn_0_found        = false;
   QString horn_1           = "";      bool horn_1_found        = false;
   QString horn_2           = "";      bool horn_2_found        = false;
   QString horn_3           = "";      bool horn_3_found        = false;
   QString horn_4           = "";      bool horn_4_found        = false;
   QString sct_distance     = "";      bool sct_distance_found  = false;
   QString sigma_sct        = "";      bool sigma_sct_found     = false;
   QString col_mdistance    = "";      bool col_mdistance_found = false;
   QString col_cdistance    = "";      bool col_cdistance_found = false;
   QString col_xdistance    = "";      bool col_xdistance_found = false;
   QString col_ydistance    = "";      bool col_ydistance_found = false;
   QString norm_value       = "";      bool norm_value_found    = false;
   QString gray_mu_dmax     = "";      bool gray_mu_dmax_found  = false;
   QString energy_min       = "";      bool energy_min_found    = false;
   QString energy_max       = "";      bool energy_max_found    = false;
   QString l_value          = "";      bool l_value_found       = false;
   QString b_value          = "";      bool b_value_found       = false;
   QString a_value          = "";      bool a_value_found       = false;
   QString p_con            = "";      bool p_con_found         = false;
   QString distance_con     = "";      bool distance_con_found  = false;
   QString radius_con       = "";      bool radius_con_found    = false;
   QString e_mean_con       = "";      bool e_mean_con_found    = false;
   QString nu_value         = "0.45";  bool nu_value_found      = false;

   // read input file
   QFile iFile( fName );
   if ( iFile.open( QIODevice::ReadOnly ) ) {
   QTextStream stream( &iFile );
   QString line;
   while ( !stream.atEnd() ) {
      line = stream.readLine(); // line of text excluding '\n'
      QString strLine = line.toLatin1();
      if (strLine.contains("BASE-DATA-FILE-VERSION:") != 0) {
         version = getKeyValue(strLine); version_found = true;
      }
      if (strLine.contains("BASE-DATA-ENTRY") != 0) {
         entry_found = true;
      }
      if (strLine.contains("PARTICLE-TYPE:") != 0) {
        ptl_type = getKeyValue(strLine); ptl_type_found = true;
      }
      if (strLine.contains("NOMINAL-ENERGY:") != 0) {
         energy = getKeyValue(strLine); energy_found = true;
      }
      if (strLine.contains("BEAM-MODEL-ID:") != 0) {
         model_id = getKeyValue(strLine); model_id_found = true;
      }
      if (strLine.contains("BEGIN-PARAMETERS") != 0) {
         begin_para_found = true;
      }
      if (strLine.contains("PRIMARY-PHOTONS:") != 0) {
         p_pri = getKeyValue(strLine); p_pri_found = true;
      }
      if (strLine.contains("PRIMARY-DIST:") != 0) {
         pri_distance = getKeyValue(strLine); pri_distance_found = true;
      }
      if (strLine.contains("PRIMARY-SIGMA:") != 0) {
         sigma_pri = getKeyValue(strLine); sigma_pri_found = true;
      }
      if (strLine.contains("SCATTER-DIST:") != 0) {
         sct_distance = getKeyValue(strLine); sct_distance_found = true;
      }
      if (strLine.contains("SCATTER-SIGMA:") != 0) {
         sigma_sct = getKeyValue(strLine); sigma_sct_found = true;
      }
      if (strLine.contains("PRIMARY-HORN0:") != 0) {
         horn_0 = getKeyValue(strLine); horn_0_found = true;
      }
      if (strLine.contains("PRIMARY-HORN1:") != 0) {
         horn_1 = getKeyValue(strLine); horn_1_found = true;
      }
      if (strLine.contains("PRIMARY-HORN2:") != 0) {
         horn_2 = getKeyValue(strLine); horn_2_found = true;
      }
      if (strLine.contains("PRIMARY-HORN3:") != 0) {
         horn_3 = getKeyValue(strLine); horn_3_found = true;
      }
      if (strLine.contains("PRIMARY-HORN4:") != 0) {
         horn_4 = getKeyValue(strLine); horn_4_found = true;
      }
      if (strLine.contains("COLM-DIST:") != 0) {
         col_mdistance = getKeyValue(strLine); col_mdistance_found = true;
      }
      if (strLine.contains("COLC-DIST:") != 0) {
         col_cdistance = getKeyValue(strLine); col_cdistance_found = true;
      }
      if (strLine.contains("COLX-DIST:") != 0) {
         col_xdistance = getKeyValue(strLine); col_xdistance_found = true;
      }
      if (strLine.contains("COLY-DIST:") != 0) {
         col_ydistance = getKeyValue(strLine); col_ydistance_found = true;
      }
      if (strLine.contains("NORM-VALUE:") != 0) {
         norm_value = getKeyValue(strLine); norm_value_found = true;
      }
      if (strLine.contains("GY/MU-DMAX:") != 0) {
         gray_mu_dmax = getKeyValue(strLine); gray_mu_dmax_found = true;
      }
      if (strLine.contains("ENERGY-MAX:") != 0) {
         energy_max = getKeyValue(strLine); energy_max_found = true;
      }
      if (strLine.contains("ENERGY-MIN:") != 0) {
         energy_min = getKeyValue(strLine); energy_min_found = true;
      }
      if (strLine.contains("L-VALUE:") != 0) {
         l_value = getKeyValue(strLine); l_value_found = true;
      }
      if (strLine.contains("B-VALUE:") != 0) {
         b_value = getKeyValue(strLine); b_value_found = true;
      }
      if (strLine.contains("A-VALUE:") != 0) {
         a_value = getKeyValue(strLine); a_value_found = true;
      }
      if (strLine.contains("CHARGED-PARTICLES:") != 0) {
         p_con = getKeyValue(strLine); p_con_found = true;
      }
      if (strLine.contains("CHARGED-DIST:") != 0) {
         distance_con = getKeyValue(strLine); distance_con_found = true;
      }
      if (strLine.contains("CHARGED-RADIUS:") != 0) {
         radius_con = getKeyValue(strLine); radius_con_found = true;
      }
      if (strLine.contains("CHARGED-E-MEAN:") != 0) {
         e_mean_con = getKeyValue(strLine); e_mean_con_found = true;
      }
      if (strLine.contains("NU-VALUE:") != 0) {
         nu_value = getKeyValue(strLine); nu_value_found = true;
      }
      if (strLine.contains("END-PARAMETERS") != 0) {
         end_para_found = true;
      }
   }
   }
   iFile.close();

   nu_value = ui->lineEditNUval->text();

   QFile oFile( fName );
   oFile.remove();
   oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
   QTextStream oStream( &oFile );

   // oStream << "BASE-DATA-FILE-VERSION:  1.3" << endl;
   oStream << "BASE-DATA-FILE-VERSION:  1.4" << endl;
   oStream << "BASE-DATA-ENTRY" << endl;
   oStream << "   PARTICLE-TYPE:     " << ptl_type << endl;
   oStream << "   NOMINAL-ENERGY:    " << energy << endl;
   if (ptl_type.contains("Photon")) model_id = "1";
   oStream << "   BEAM-MODEL-ID:     " << model_id << endl;
   oStream << "   BEGIN-PARAMETERS" << endl;
   oStream << "      PRIMARY-PHOTONS:    " << p_pri << endl;
   oStream << "      PRIMARY-DIST:       " << pri_distance << endl;
   oStream << "      PRIMARY-SIGMA:      " << sigma_pri << endl;
   oStream << "      PRIMARY-HORN0:      " << horn_0 << endl;
   oStream << "      PRIMARY-HORN1:      " << horn_1 << endl;
   oStream << "      PRIMARY-HORN2:      " << horn_2 << endl;
   oStream << "      PRIMARY-HORN3:      " << horn_3 << endl;
   oStream << "      PRIMARY-HORN4:      " << horn_4 << endl;
   oStream << "      SCATTER-DIST:       " << sct_distance << endl;
   oStream << "      SCATTER-SIGMA:      " << sigma_sct << endl;
   oStream << "      COLM-DIST:          " << col_mdistance << endl;
   oStream << "      COLC-DIST:          " << col_cdistance << endl;
   oStream << "      COLX-DIST:          " << col_xdistance << endl;
   oStream << "      COLY-DIST:          " << col_ydistance << endl;
   oStream << "      NORM-VALUE:         " << norm_value << endl;
   oStream << "      GY/MU-DMAX:         " << gray_mu_dmax << endl;
   oStream << "      ENERGY-MIN:         " << energy_min << endl;
   oStream << "      ENERGY-MAX:         " << energy_max << endl;
   oStream << "      L-VALUE:            " << l_value << endl;
   oStream << "      B-VALUE:            " << b_value << endl;
   oStream << "      A-VALUE:            " << a_value << endl;
   oStream << "      NU-VALUE:           " << nu_value << endl;
   oStream << "      CHARGED-PARTICLES:  " << p_con << endl;
   oStream << "      CHARGED-DIST:       " << distance_con << endl;
   oStream << "      CHARGED-RADIUS:     " << radius_con<< endl;
   oStream << "      CHARGED-E-MEAN:     " << e_mean_con << endl;
   oStream << "   END-PARAMETERS" << endl;

   oFile.close();
    */
}
// -----------------------------------------------------------------------------
void MainConsole::updateBDT(QString MODEL_ID, QString ENERGY_MAX) {
   QString mName = ui->comboBoxMachine->currentText();
   QString DATDIR = usr->LHOME + mName + "/dat";
   QString fName = DATDIR + "/" + mName + ".bdt";
   if (MODEL_ID.contains("XVMC_Mono_p") || MODEL_ID.contains("XVMC_Mono_s"))
       fName = DATDIR + "/" + mName + "_GeoModel.bdt";
   if (MODEL_ID.contains("XVMC_Mono_e"))
       fName = DATDIR + "/" + mName + "_GeoModelEcon.bdt";
   std::cout << "BDT = " << fName.toStdString() << endl;
   readBDT(fName);

   // nu_value = ui->lineEditNUval->text();

   QFile oFile( fName );
   oFile.remove();
   oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
   QTextStream oStream( &oFile );

   // oStream << "BASE-DATA-FILE-VERSION:  1.3" << endl;
#ifdef XVMC
   oStream << "BASE-DATA-FILE-VERSION:  1.4" << endl;
#else
   oStream << "BASE-DATA-FILE-VERSION:  1.5" << endl;
#endif
   oStream << "BASE-DATA-ENTRY" << endl;
   oStream << "   PARTICLE-TYPE:     " << usr->particle << endl;
   oStream << "   NOMINAL-ENERGY:    " << usr->E << endl;
   // if (ptl_type.contains("Photon")) usr->ID = "1";
   usr->ID = "1";
   if (MODEL_ID.contains("Mono")) usr->ID = "0";
   if (MODEL_ID.contains("XVMC_Mono")) usr->ID = "-1";
   oStream << "   BEAM-MODEL-ID:     " << usr->ID << endl;
   oStream << "   BEGIN-PARAMETERS" << endl;
   if (MODEL_ID.contains("XVMC_Mono_p")) {
      oStream << "      PRIMARY-PHOTONS:     1.0" << endl;
   } else if (MODEL_ID.contains("XVMC_Mono_s")) {
      oStream << "      PRIMARY-PHOTONS:     0.0" << endl;
   } else {
      oStream << "      PRIMARY-PHOTONS:    " << usr->p0 << endl;
   }
#ifdef XVMC
   oStream << "      PRIMARY-DIST:       " << usr->Z0 << endl;
#endif
   oStream << "      PRIMARY-SIGMA:      " << usr->s0 << endl;
   oStream << "      PRIMARY-HORN0:      " << usr->h0 << endl;
   oStream << "      PRIMARY-HORN1:      " << usr->h1 << endl;
   oStream << "      PRIMARY-HORN2:      " << usr->h2 << endl;
   oStream << "      PRIMARY-HORN3:      " << usr->h3 << endl;
   oStream << "      PRIMARY-HORN4:      " << usr->h4 << endl;
   oStream << "      SCATTER-DIST:       " << usr->ZS << endl;
   oStream << "      SCATTER-SIGMA:      " << usr->ss << endl;
#ifdef XVMC
   oStream << "      COLM-DIST:          " << usr->ZM << endl;
   oStream << "      COLC-DIST:          " << usr->ZC << endl;
   oStream << "      COLX-DIST:          " << usr->ZX << endl;
   oStream << "      COLY-DIST:          " << usr->ZY << endl;
#endif

   if (MODEL_ID.contains("Mono")) {
      usr->pcon = "0.000";
      usr->normValue = "1.000";
      usr->gy_mu_dmax = "0.010";
   }
   if (MODEL_ID.contains("Electron") || MODEL_ID.contains("XVMC_Mono_e")) {
      usr->pcon = "1.000";
      usr->normValue = "1.000";
      usr->gy_mu_dmax = "0.010";
      // e_mean_con = ui->lineEditEmeanAir->text();
      // std::cout << "e_mean_con = " << e_mean_con << endl;
   }
   usr->nu = ui->lineEditNUval->text();
   oStream << "      NORM-VALUE:         " << usr->normValue << endl;
   oStream << "      GY/MU-DMAX:         " << usr->gy_mu_dmax << endl;
   oStream << "      ENERGY-MIN:         " << usr->Emin << endl;
   if (ENERGY_MAX.contains("Max"))
      oStream << "      ENERGY-MAX:         " << usr->Emax << endl;
   else
    oStream << "      ENERGY-MAX:         " << ENERGY_MAX << endl;
    oStream << "      L-VALUE:            " << usr->lval << endl;
    oStream << "      B-VALUE:            " << usr->bval << endl;
    oStream << "      A-VALUE:            " << usr->aval << endl;
    oStream << "      Z-VALUE:            " << usr->zval << endl;
    // oStream << "      NU-VALUE: 0.45" << endl;
    oStream << "      NU-VALUE:           " << usr->nu << endl;
    oStream << "      CHARGED-PARTICLES:  " << usr->pcon << endl;
    oStream << "      CHARGED-DIST:       " << usr->ZE << endl;
    oStream << "      CHARGED-RADIUS:     " << usr->FFRad << endl;
    oStream << "      CHARGED-E-MEAN:     " << usr->eEnergy << endl;
    oStream << "   END-PARAMETERS" << endl;

   oFile.close();
}
// -----------------------------------------------------------------------------
void MainConsole::runVerify() { // For Verify Use

 QString mName = ui->comboBoxMachine->currentText();
 QString MC_DIR = usr->LHOME + mName + "/" + mName;
 QString MC_DIR_INP = MC_DIR + "/INPUT";

 QString XVMC_WORK = usr->LHOME+mName;
 QString XVMC_HOME = usr->LHOME+"bin";
 setenv("XVMC_WORK",XVMC_WORK.toLatin1(),1);
 setenv("XVMC_HOME",XVMC_HOME.toLatin1(),1);
 setenv("MACHINE",mName.toLatin1(),1);

 QString CLN_FILE = MC_DIR_INP + "/D6MV";
 writeCLN(CLN_FILE,"");

 QString VER_FILE = MC_DIR_INP + "/D6MV";
 QString OPTIONS = "";
 QStringList xZLIST;
 QStringList yZLIST;
 writeVER(VER_FILE, xZLIST, yZLIST, OPTIONS);

 QString BDT_DIR = XVMC_WORK + "/dat/basedata";
 makeDir(BDT_DIR);
 QString BDT_FILE = XVMC_WORK + "/dat/" + mName + ".bdt";
#ifdef VERSION14
 // second 1.4 should be modified to 1.5 for new energy spectrum in Verify
 QString CMD = "cat " + BDT_FILE
     + " | sed s/'BASE-DATA-FILE-VERSION:  1.4'/'BASE-DATA-FILE-VERSION:  1.4'/g > "
     + BDT_DIR + "/" + mName + ".bdt";
#else
  QString CMD = "cp -f " + BDT_FILE + " " + BDT_DIR + "/" + mName + ".bdt";
#endif
  mySystem(CMD);

  QString XVMC_HOME_BDT_DIR = XVMC_HOME + "/dat/basedata";
  makeDir(XVMC_HOME_BDT_DIR);
  // QString CMD = "cp -f " + BDT_FILE + " " + BDT_DIR;
  CMD = "cp -f " + BDT_FILE + " " + XVMC_HOME_BDT_DIR;
  mySystem(CMD);

  // CMD = "ln -sf " + BDT_DIR + " " + XVMC_HOME + "/dat";
  // mySystem(CMD);

  CMD = "cp -f " + CLN_FILE + ".CLN " + MC_DIR + "/D6MV.CLN";
  mySystem(CMD);

  CMD = "cp -f " + VER_FILE + ".VER " + MC_DIR + "/D6MV.VER";
  mySystem(CMD);
#ifdef CMS_SPECT
  CMD = "cd " + MC_DIR + ";" + XVMC_HOME+"/verify_cms.exe -i D6MV";
#else
  CMD = "cd " + MC_DIR + ";" + XVMC_HOME+"/verify.exe -i D6MV";
#endif
  mySystem(CMD);

  if (readLocalSetting(mName,"PBMC") == "Done") {
     ui->pushButtonPBMC->setEnabled(true); // It was false
     ui->groupBoxMCPB->setEnabled(true); // It was false
     ui->groupBoxPBComm->setEnabled(true);
     ui->pushButtonPBComm->setEnabled(true);
     ui->pushButtonPBReview->setEnabled(false);
     ui->pushButtonPBPack->setEnabled(false);
     writeLocalSetting(mName, "PBComm", "NotDone");
  } else {
     ui->groupBoxMCPB->setEnabled(true);
     ui->pushButtonPBMC->setEnabled(true);
     ui->groupBoxPBComm->setEnabled(true); // It was false but true temporarily
  }
}
// -----------------------------------------------------------------------------
void MainConsole::PBPack() {
 QString MFile = ui->comboBoxMachine->currentText();
 QString XDir = ui->lineEditTeleDir->text()+"/" + MFile;
 QString MDir = usr->LHOME+MFile;
 QString ZDir = MDir+"/"+MFile+"_Pack";

 QString DATDIR = usr->LHOME + MFile + "/dat";
 QString BDTFile = DATDIR + "/" + MFile + ".bdt";
/*
 std::cout << "cp -R " << XDir << " " << ZDir << endl;
 makeDir(ZDir+"/XVMC");
 std::cout << "echo 000010bd > " << ZDir << "/XVMC/linac.bdt" << endl;
 std::cout << "unix2dos < " << BDTFile << " > " << ZDir << "/XVMC/linac.bdt" << endl;

 makeDir(ZDir+"/MonPB");
 QString Energy = readLocalSetting(comboBoxMachine->currentText(),"Energy");
 std::cout << "echo 000010bc > " << ZDir << "/MonPB/linac.rpb" << endl;
 std::cout << "unix2dos < " << MDir + "/PBBaseData_" + Energy.section('.',0,0)
                          +"MV.RPB" << " >> " << ZDir << "/MonPB/linac.rpb" << endl;

 std::cout << "zip -mr " << ZDir+".zip " << ZDir << endl;
*/
 QString LBIN = usr->LHOME + "bin";
 QString CMD = "cp -R " + XDir + "/ " + ZDir;
 QString Energy = readLocalSetting(MFile,"Energy");
 QString SSDcm = ui->comboBoxPBSSD->currentText();
 mySystem(CMD);
 makeDir(ZDir+"/XVMC");
 makeDir(ZDir+"/MonPB");
 CMD = "echo 000010bd > " + ZDir + "/XVMC/" + MFile + ".bdt";
 mySystem(CMD);
#ifdef VERSION14
 CMD = "cat " + BDTFile
     + " | sed s/'BASE-DATA-FILE-VERSION:  1.4'/'BASE-DATA-FILE-VERSION:  1.3'/g >> "
     + ZDir + "/XVMC/" + MFile + ".bdt";
#else
 CMD = "cat " + BDTFile + " >> " + ZDir + "/XVMC/" + MFile + ".bdt";
#endif
 mySystem(CMD);
 CMD = "unix2dos < " + ZDir + "/XVMC/" + MFile + ".bdt >> "
     + ZDir + "/XVMC/linac.bdt";
 mySystem(CMD);
 CMD = "echo 000010bc > " + ZDir + "/MonPB/" + MFile + ".rpb";
 mySystem(CMD);
 // CMD = "cat " + MDir + "/PBBaseData_" + Energy.section('.',0,0) + "MV.RPB"
 CMD = "cat " + MDir + "/dat/"+MFile+"_"+SSDcm+".RPB"
     + " >> " + ZDir + "/MonPB/" + MFile + ".rpb";
 mySystem(CMD);
 CMD = "unix2dos < " + ZDir + "/MonPB/" + MFile + ".rpb >> "
     + ZDir + "/MonPB/linac.rpb";
 mySystem(CMD);

 CMD = "cd " + ZDir + "; cp valid valid.new; "
       + LBIN + "/validation.exe;"
       "mv valid valid.old; mv valid.new valid";
 mySystem(CMD);

 CMD = "cd " + MDir + "; zip -mr " + ZDir+".zip " + MFile
     + "_Pack" + " > /dev/null";
 mySystem(CMD);

 if (readLocalSetting(MFile,"Validate") == "Done")
    ui->pushButtonPBPack->setEnabled(false);
}
// -----------------------------------------------------------------------------
void MainConsole::getAirDataNew() {
 // Check machine directory exists in Loacl Home like Monaco
 QString mName = ui->comboBoxMachine->currentText();
 QString WORK_DIR = usr->LHOME + mName;
 if (!isThereDir(WORK_DIR)) makeDir(WORK_DIR);
 // Check afit directory exists
 QString AFIT_DIR = WORK_DIR + "/afit";
 if (!isThereDir(AFIT_DIR)) makeDir(AFIT_DIR);
 // Check afit/data directory exists
 QString AFIT_DIR_DATA = AFIT_DIR + "/data";
 if (!isThereDir(AFIT_DIR_DATA)) makeDir(AFIT_DIR_DATA);

 // XiO TEL Directory
 QString MDIR = ui->lineEditTeleDir->text() + mName;
 // File for In-Air Output Factors
 QString OP_FILE = MDIR + "/op";
 QString OA_FILE = MDIR + "/oa";
 QString rect_OP_FILE = MDIR + "/rect_op";
 QString rect_OA_FILE = MDIR + "/rect_oa";

 bool passed = false;
 // Check Existing
 if (isThereFile(MDIR, "oa") &&
     isThereFile(MDIR, "rect_oa")) {
     passed = true;
 } else {
     QMessageBox::warning( this,
     "WARNING: Missing File(s)",
     "In-air output factor file(s) missed..."
     "Check op, oa, and rect_oa files\n"
     "in XiO Beam Data Directory.\n" + MDIR,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
 }
 if (passed) {
    QString FS = getOutputFactor(OA_FILE,10,0.0);
    QString OF = getOutputFactor(OA_FILE,11,0.0);
    QString SAD = getOutputFactor(OA_FILE,15,0.0);
    SAD = SAD.section('|',0,0); // for SAD
    bool ok;
    FS = FS + getOutputFactor(rect_OA_FILE,12,SAD.toFloat(&ok));
    OF = OF + getOutputFactor(rect_OA_FILE,13,SAD.toFloat(&ok));
    QStringList FSList = FS.split("|",QString::SkipEmptyParts);
    QStringList OFList = OF.split("|",QString::SkipEmptyParts);
    float maxFS = 0.0; int iFS = 0; int iFSmax = 0;
    QString maxField = "";
    float *xFW = (float *) calloc (100, sizeof(float));
    float *yFL = (float *) calloc (100, sizeof(float));
    for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it){
      QString FieldSize = *it;
      float xFS = FieldSize.section('x',0,0).toFloat(&ok);
      float yFS = FieldSize.section('x',1,1).toFloat(&ok);
      xFW[iFS] = xFS;
      yFL[iFS] = yFS;
      //std::cout << iFS << "  " << xFS << " x " << yFS << endl;
      QString IN_AIR_DIR = MDIR+"/meas/in_air";
      QString FW = FieldSize.section('x',0,0).section(".",0,0);
      QString FL = FieldSize.section('x',1,1).section(".",0,0);
      QString PR_FILE = "pr0."+FW+"."+FL;
      // std::cout << IN_AIR_DIR+"/"+PR_FILE << endl;
      if (xFS*yFS >= maxFS && isThereFile(IN_AIR_DIR, PR_FILE)) {
         maxFS = xFS*yFS;
         iFSmax = iFS;
         maxField = FieldSize;
      }
      iFS++;
    }

    float *outputFactor = (float *) calloc (iFS, sizeof(float));
    iFS = 0;
    float maxOF = 0.0;
    for (QStringList::Iterator it = OFList.begin(); it != OFList.end(); ++it){
      QString OutputFactor = *it;
      bool ok;

      // std::cout << iFS << "  " << xFS << " x " << yFS << endl;
      // std::cout << xFW[iFS] << " x " << yFL[iFS] << endl;
      outputFactor[iFS] = OutputFactor.toFloat(&ok);
      if(ui->checkBoxMBSF->isChecked()){
            float openX1 = xFW[iFS]/20.0;
            float openX2 = xFW[iFS]/20.0;
            float openY1 = yFL[iFS]/20.0;
            float openY2 = yFL[iFS]/20.0;
            //printf ("MONITORBSF> Field Size: X1=%4.2f  X2=%4.2f  Y1=%4.2f  Y2=%4.2f\n",
            //         openX1, openX2, openY1, openY2);
            // Calculate Monitor Backscatter Factor, MBSF
            float rY1 = 1.54 - 8.45E-2 * openY1 + 4.47E-5 * openY1*openY1*openY1;
            float rX1 = 0.40 - 1.87E-2 * openX1;
            float pY1 = 3.95E-2*openY1 - 3.55E-5 * openY1*openY1*openY1;
            //printf ("MONITORBSF> rY1 rX1 pY1 = %e %e %e\n", rY1, rX1, pY1);
            float rY2 = 1.54 - 8.45E-2 * openY2 + 4.47E-5 * openY2*openY2*openY2;
            float rX2 = 0.40 - 1.87E-2 * openX2;
            float pY2 = 3.95E-2*openY2 - 3.55E-5 * openY2*openY2*openY2;
            //printf ("MONITORBSF> rY2 rX2 pY2 = %e %e %e\n", rY2, rX2, pY2);
            float rY = rY1 + rY2;
            float rX = (rX1 + rX2)*(pY1 + pY2);
            float rFS = rX + rY;
            //printf ("MONITORBSF> rY rX rFS = %e %e %e\n", rY, rX, rFS);

            openX1 = 5.0;
            openX2 = 5.0;
            openY1 = 5.0;
            openY2 = 5.0;
            // Calculate Monitor Backscatter Factor, MBSF
            rY1 = 1.54 - 8.45E-2 * openY1 + 4.47E-5 * openY1*openY1*openY1;
            rX1 = 0.40 - 1.87E-2 * openX1;
            pY1 = 3.95E-2*openY1 - 3.55E-5 * openY1*openY1*openY1;
            //printf ("MONITORBSF> rY1 rX1 pY1 = %e %e %e\n", rY1, rX1, pY1);
            rY2 = 1.54 - 8.45E-2 * openY2 + 4.47E-5 * openY2*openY2*openY2;
            rX2 = 0.40 - 1.87E-2 * openX2;
            pY2 = 3.95E-2 *openY2- 3.55E-5 * openY2*openY2*openY2;
            //printf ("MONITORBSF> rY2 rX2 pY2 = %e %e %e\n", rY2, rX2, pY2);
            rY = rY1 + rY2;
            rX = (rX1 + rX2)*(pY1 + pY2);
            float rREF = rX + rY;
            //printf ("MONITORBSF> rY rX rREF = %e %e %e\n", rY, rX, rREF);

            float MBSF = (1.0 + rREF/100.)/(1.0 + rFS/100.0);

         outputFactor[iFS] *= (MBSF*MBSF);
      }

      if (iFS == iFSmax) {
         maxOF = outputFactor[iFS];
      }
      iFS++;
    }

    // std::cout << "maxFS = " << maxFS << "  maxOF = " << maxOF << endl;

    // Check afit/diff directory exists
    QString AFIT_DIR_DIFF = AFIT_DIR + "/diff";
    if (!isThereDir(AFIT_DIR_DIFF)) makeDir(AFIT_DIR_DIFF);

    // Check afit/afit.lst file exists
    QString AFIT_LST = AFIT_DIR + "/afit.lst";
    QFile afitLst(AFIT_LST);
    if (afitLst.exists()) afitLst.remove();

    bool passed = true;
    FILE *ostrm = fopen(AFIT_LST.toLatin1(),"wt");
    fprintf (ostrm, "#file name              fit prof        ");
    fprintf (ostrm, "X       Y       Z               ");
    fprintf (ostrm, "WX = %d ",
    maxField.section('x',0,0).section('.',0,0).toInt(&passed,10)/10);
    usr->WXN.sprintf("%d",
    maxField.section('x',0,0).section('.',0,0).toInt(&passed,10)/10);
    fprintf (ostrm, "WY = %d \n",
    maxField.section('x',1,1).section('.',0,0).toInt(&passed,10)/10);
    usr->WYN.sprintf("%d",
    maxField.section('x',1,1).section('.',0,0).toInt(&passed,10)/10);

    iFS = 0;
    float xOffset = 0.0; float yOffset = 0.0; float zOffset =0.0;
    for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it){
      QString fs = *it;
      QString fw = fs.section('x',0,0).section('.',0,0);
      QString fl = fs.section('x',1,1).section('.',0,0);

      float factor = 1.0;
      float MBSF = 1.0; // Monitor Backscatter Correction Factor by Markus Alber's Eq. (UMich Eq)
      if (usr->XIOMLC.contains("VARIAN26")
                || usr->XIOMLC.contains("VARIAN40")
                || usr->XIOMLC.contains("MILLENNIUM120")) {
            float xMaxFS = 400.0;
            float yMaxFS = 400.0;
            float xRefFS = sqrt(maxFS);
            float yRefFS = sqrt(maxFS);
            float FW = fw.toFloat(&ok);
            float FL = fl.toFloat(&ok);
            float St = 0.025; // Monitor Backscatter Factor of Y Jaw (TJaws)
            float Sp = 0.010; // Monitor Backscatter Factor of X Jaw (PJaws)
            float Sref = (1.0 - yRefFS/yMaxFS)*St + (1.0-xRefFS/xMaxFS)*(yRefFS/yMaxFS)*Sp;
            float Stot = (1.0 - FL/yMaxFS)*St + (1.0-FW/xMaxFS)*(FL/yMaxFS)*Sp;
            MBSF = (1.0-Sref)/(1.0-Stot);
            // std::cout << FL << " " << FW << " " << xRefFS << " " << yRefFS << " " << xMaxFS << " " << yMaxFS << " " << MBSF << endl;
        }
      if (maxOF != 0.0) {
            factor = outputFactor[iFS]/maxOF * 100 * MBSF;
         // std::cout << iFS << "  " << outputFactor[iFS]/maxOF*100 << " * " << MBSF << " = " << factor << endl;
        }

      // Check pr0 file exists
      QString PR = MDIR+"/meas/in_air/pr0."+fw+"."+fl;
      QFile prFile(PR);

      if (prFile.exists()) {
        QString zFileName = "data/z" + fw + "x" + fl + ".000";
        fprintf (ostrm, "%s\t%d\t%s\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
#ifdef XVMC
                 zFileName.toStdString().c_str(), 1, "Z",
#else
                 zFileName.toStdString().c_str(), 1, "z",
#endif
                xOffset, yOffset, zOffset,
                fw.toFloat(&ok)/10.0, fl.toFloat(&ok)/10.0);
        getPDD(PR,AFIT_DIR+"/"+zFileName,1000.0,factor,0.0,11);
      }

      // Check ocrr.0.0 file exists
      QString xOCRR = MDIR+"/meas/in_air/ocrr.0.0."+fw+"."+fl;
      QFile xOcrrFile(xOCRR);

      if (xOcrrFile.exists()) {
        QString xFileName = "data/x" + fw + "x" + fl;
        QString SDDs =
           getOCRR(xOCRR,AFIT_DIR+"/"+xFileName,1000.0,factor,0.0,11);
        QStringList SDDList = SDDs.split("|",QString::SkipEmptyParts);

        for (QStringList::Iterator sdd = SDDList.begin();
                                   sdd != SDDList.end(); ++sdd){
          QString fileName = xFileName + "." + *sdd;
          QString scanDepth = *sdd;
          bool ok;
          fprintf (ostrm, "%s\t%d\t%s\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
#ifdef XVMC
                   fileName.toStdString().c_str(), 1, "X",
#else
                   fileName.toStdString().c_str(), 1, "x",
#endif
                   xOffset, yOffset, scanDepth.toFloat(&ok),
                fw.toFloat(&ok)/10.0, fl.toFloat(&ok)/10.0);
        }
      }

      // Check ocrr.0.90 file exists
      QString yOCRR = MDIR+"/meas/in_air/ocrr.0.90."+fw+"."+fl;
      QFile yOcrrFile(yOCRR);

      if (yOcrrFile.exists()) {
        QString yFileName = "data/y" + fw + "x" + fl;
        QString SDDs =
           getOCRR(yOCRR,AFIT_DIR+"/"+yFileName,1000.0,factor,0.0,11);
        QStringList SDDList = SDDs.split("|",QString::SkipEmptyParts);

        for (QStringList::Iterator sdd = SDDList.begin();
                                   sdd != SDDList.end(); ++sdd){
          QString fileName = yFileName + "." + *sdd;
          QString scanDepth = *sdd;
          bool ok;
          fprintf (ostrm, "%s\t%d\t%s\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
#ifdef XVMC
                   fileName.toStdString().c_str(), 1, "Y",
#else
                   fileName.toStdString().c_str(), 1, "y",
#endif
                   xOffset, yOffset, scanDepth.toFloat(&ok),
                fw.toFloat(&ok)/10.0, fl.toFloat(&ok)/10.0);
        }
      }

      iFS++;
    }

    fclose(ostrm);
    // ui->pushButtonAirFit->setEnabled(true); // REMOVED
    // ui->pushButtonAirReview->setEnabled(false);  // REMOVED
    writeLocalSetting(mName,"AirData","Modified");
 } // if (passed)
}
// -----------------------------------------------------------------------------
void MainConsole::getPRLists() {
 // Gets field sizes (FS1 and FS2) from op file
 // XiO TEL Directory
 QString mName = ui->comboBoxMachine->currentText();
 QString TELE_DIR = ui->lineEditTeleDir->text() + mName;
 QString OP_FILE = TELE_DIR + "/op";

 QString FS1 = readLocalSetting(mName,"FS1").simplified();
 QString FS2 = readLocalSetting(mName,"FS2").simplified();

 for (int i=0; i<ui->comboBox1stFS->count(); i++) {
    ui->comboBox1stFS->removeItem(i);
 }
 for (int i=0; i<ui->comboBox2ndFS->count(); i++) {
    ui->comboBox2ndFS->removeItem(i);
 }

 if (FS1 == "") {
   FS1 = "None";
   if (ui->comboBox1stFS->count() > 0) ui->comboBox1stFS->removeItem(0);
   ui->comboBox1stFS->addItem(FS1);
 }
 if (FS2 == "") {
   FS2 = "None";
   if (ui->comboBox2ndFS->count() > 0) ui->comboBox1stFS->removeItem(0);
   ui->comboBox2ndFS->addItem(FS2);
 }
 QString FS = getOutputFactor(OP_FILE,0,0.0);  // Field Size
 // std::cout << "getPRLists:: FS = " << FS << endl;
 QString ssd = "1000"; // 1000 mm
 ssd = usr->SSD;
 // std::cout << "usr->SSD = " << usr->SSD << " in getPRList " << endl;

 QStringList FSList = FS.split("|",QString::SkipEmptyParts);
 for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it) {
   QString qFS = *it;
   QString FW = qFS.section('x',0,0).section('.',0,0);
   QString FL = qFS.section('x',1,1).section('.',0,0);
   // std::cout << "getPRLists:: FW x FL (FS1 and FS2) = " << FW  << " x " << FL << endl;
   // Check it already exists in the FS1 list
   bool FS1NotExist = true;
   for (int i=0; i<ui->comboBox1stFS->count(); i++) {
     if (ui->comboBox1stFS->itemText(i) == FW) FS1NotExist = false;
   }
   // Check it already exists in the FS2 list
   bool FS2NotExist = true;
   for (int i=0; i<ui->comboBox2ndFS->count(); i++) {
     if (ui->comboBox2ndFS->itemText(i) == FW) FS2NotExist = false;
   }
   // Check pr0 file exists
   QString PR_FILE = TELE_DIR+"/meas/pr"+ssd+"."+FW+"."+FL;
   QFile prFile(PR_FILE);
   if (prFile.exists()) {
     if (FS1NotExist) ui->comboBox1stFS->addItem(FW);
     if (FS2NotExist) ui->comboBox2ndFS->addItem(FW);
   }
 }

 for (int i=0; i<ui->comboBox1stFS->count(); i++) {
   if (ui->comboBox1stFS->itemText(i) == "100") ui->comboBox1stFS->setCurrentIndex(i);
   if (ui->comboBox1stFS->itemText(i) == "104") ui->comboBox1stFS->setCurrentIndex(i); // For Beam Modulator
   if (FS1 != "" && ui->comboBox1stFS->itemText(i) == FS1) ui->comboBox1stFS->setCurrentIndex(i);
   if (FS1 != "None") ui->comboBox1stFS->setCurrentText(FS1);
   // std::cout << "getPRLists:: FS1 = " << FS1 << " " << comboBox1stFS->itemText(i) << endl;
 }
 for (int i=0; i<ui->comboBox2ndFS->count(); i++) {
   if (ui->comboBox2ndFS->itemText(i) == "30") ui->comboBox2ndFS->setCurrentIndex(i);
   if (ui->comboBox2ndFS->itemText(i) == "32") ui->comboBox2ndFS->setCurrentIndex(i); // For Beam Modulator
   if (FS2 != "" && ui->comboBox2ndFS->itemText(i) == FS2) ui->comboBox2ndFS->setCurrentIndex(i);
   if (FS2 != "None") ui->comboBox2ndFS->setCurrentText(FS2);
   // std::cout << "getPRLists:: FS2 = " << FS2 << " " << comboBox2ndFS->itemText(i) << endl;
 }
} // End of getPRList()
// -----------------------------------------------------------------------------
QString MainConsole::getWaterData(QString PT_NAME) {
 // Make input files and get water data
 QString planList = "";
 // Check machine directory exists in Loacl Home like Monaco
 QString mName = ui->comboBoxMachine->currentText();
 QString WORK_DIR = usr->LHOME + mName;
 QString PT_DIR = WORK_DIR + "/" + PT_NAME;
 // XiO TEL Directory
 QString TELE_DIR = ui->lineEditTeleDir->text() + mName;
 // Check and Make machine directory
 if (!isThereDir(PT_DIR)) makeDir(PT_DIR);
 // Check measurement directory exists
 QString PT_DIR_MEAS = PT_DIR + "/meas";
 if (!isThereDir(PT_DIR_MEAS)) makeDir(PT_DIR_MEAS);
 // Check calculation directory exists
 QString PT_DIR_CALC = PT_DIR + "/calc";
 if (!isThereDir(PT_DIR_CALC)) makeDir(PT_DIR_CALC);
 // Check VMC Incput directory exists
 QString PT_DIR_INP = PT_DIR + "/INPUT";
 if (!isThereDir(PT_DIR_INP)) makeDir(PT_DIR_INP);


 // Multi-Beam Plan
 int iBeam = 0;

 QString Energy =
         readLocalSetting(mName,"Energy");

 QString mInfoFile = mName + ".info";
 if (isThereFile(WORK_DIR, mInfoFile)) {
    getMachineInfo(WORK_DIR+"/"+mInfoFile);
 }

 bool ok;
 QString EE = Energy.section('.',0,0);

 // File for  Output Factors
 QString OP_FILE = TELE_DIR + "/op";
 QString RECT_OP_FILE = TELE_DIR + "/rect_op";

 bool passed = false;
 // Check Existing
 if (isThereFile(TELE_DIR, "op")
//   && isThereFile(TELE_DIR, "rect_op")
   ) {
     passed = true;
 } else {
     QMessageBox::warning( this,
     "WARNING: Missing File(s)",
     "In-air output factor file(s) missed..."
     "Check op file\n"
     "in XiO Beam Data Directory.\n" +
     TELE_DIR,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
 }
 if (passed) {
    QString FS = getOutputFactor(OP_FILE,0,0.0);  // Field Size
    QString OF = getOutputFactor(OP_FILE,1,0.0);  // Output Factor
    // FS = FS + getOutputFactor(RECT_OP_FILE,2,0.0);
    // OF = OF + getOutputFactor(RECT_OP_FILE,3,0.0);
    QStringList FSList = FS.split("|",QString::SkipEmptyParts);
    QStringList OFList = OF.split("|",QString::SkipEmptyParts);
    float maxFS = 0.0; int iFS = 0; int iFSmax = 0;
    QString maxField = "";
    for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it){
      QString FieldSize = *it;
      float xFS = FieldSize.section('x',0,0).toFloat(&ok);
      float yFS = FieldSize.section('x',1,1).toFloat(&ok);
      //std::cout << iFS << "  " << xFS << " x " << yFS << endl;
      if (xFS*yFS >= maxFS) {
         maxFS = xFS*yFS;
         iFSmax = iFS;
         maxField = FieldSize;
      }
      iFS++;
    }

    float *outputFactor = (float *) calloc (iFS, sizeof(float));
    iFS = 0;
    float maxOF = 0.0;
    for (QStringList::Iterator it = OFList.begin(); it != OFList.end(); ++it){
      QString OutputFactor = *it;
      // std::cout << iFS << "  " << xFS << " x " << yFS << endl;
      outputFactor[iFS] = OutputFactor.toFloat(&ok);
      if (iFS == iFSmax) {
         maxOF = outputFactor[iFS];
      }
      iFS++;
    }

    // std::cout << "maxFS = " << maxFS << "  maxOF = " << maxOF << endl;
    iFS = 0;
    // float xOffset = 0.0; float yOffset = 0.0; float zOffset =0.0;
    QString ssd = "1000"; // 1000 mm
    ssd = usr->SSD;

    for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it) {
      QString FS = *it;
      QString FW = FS.section('x',0,0).section('.',0,0);
      QString FL = FS.section('x',1,1).section('.',0,0);

      // Phantom Specifications: Voxel Size
      vmc->voxelSize.x = "0.5";
      vmc->voxelSize.y = "0.5";
      vmc->voxelSize.z = "0.5";

      // std::cout << "FW = " << FW.toFloat(&ok) << " FL = " << FL.toFloat(&ok) << endl;
      QString voxelLarge = ui->comboBoxGridSizeLarge->currentText();
      if (FW.toFloat(&ok) < 70 || FL.toFloat(&ok) < 70)
         voxelLarge = ui->comboBoxGridSizeSmall->currentText();
      vmc->voxelSize.x = "0."+voxelLarge;
      vmc->voxelSize.y = "0."+voxelLarge;
      vmc->voxelSize.z = "0."+voxelLarge;

      float xVoxelSize = vmc->voxelSize.x.toFloat(&ok);
      float yVoxelSize = vmc->voxelSize.y.toFloat(&ok);
      float zVoxelSize = vmc->voxelSize.z.toFloat(&ok);
      // Phantom Specifications: Phantom Size
      float xPhantomSize = 50.5;
      float yPhantomSize = 50.5;
      float zPhantomSize = 40.0;
      // Phantom Specifications: Number of Voxels
      int nX = (int)(xPhantomSize/xVoxelSize);
      int nY = (int)(yPhantomSize/yVoxelSize);
      int nZ = (int)(zPhantomSize/zVoxelSize);
      // nX = 81; nY = 81; nZ = 80;
      vmc->dimension.x.sprintf("%d", nX);
      vmc->dimension.y.sprintf("%d", nY);
      vmc->dimension.z.sprintf("%d", nZ);
      if (voxelLarge == "1") {
          vmc->dimension.x = "501"; vmc->dimension.y = "501"; vmc->dimension.z = "400";
          xPhantomSize = 50.1; yPhantomSize = 50.1; zPhantomSize = 40.0;
      }
      if (voxelLarge == "2") {
          vmc->dimension.x = "251"; vmc->dimension.y = "251"; vmc->dimension.z = "200";
          xPhantomSize = 50.2; yPhantomSize = 50.2; zPhantomSize = 40.0;
      }
      if (voxelLarge == "3") {
          vmc->dimension.x = "167"; vmc->dimension.y = "167"; vmc->dimension.z = "133";
          xPhantomSize = 50.1; yPhantomSize = 50.1; zPhantomSize = 39.9;
      }
      if (voxelLarge == "4") {
          vmc->dimension.x = "125"; vmc->dimension.y = "125"; vmc->dimension.z = "100";
          xPhantomSize = 50.0; yPhantomSize = 50.0; zPhantomSize = 40.0;
      }
      if (voxelLarge == "5") {
          vmc->dimension.x = "101";  vmc->dimension.y = "101";  vmc->dimension.z = "80";
          xPhantomSize = 50.5; yPhantomSize = 50.5; zPhantomSize = 40.0;
      }
      if (voxelLarge == "6") {
          vmc->dimension.x = "83";  vmc->dimension.y = "83";  vmc->dimension.z = "67";
          xPhantomSize = 49.8; yPhantomSize = 49.8; zPhantomSize = 40.2;
      }
      if (voxelLarge == "7") {
          vmc->dimension.x = "71";  vmc->dimension.y = "71";  vmc->dimension.z = "57";
          xPhantomSize = 49.7; yPhantomSize = 49.7; zPhantomSize = 39.9;
      }
      if (voxelLarge == "8") {
          vmc->dimension.x = "63";  vmc->dimension.y = "63";  vmc->dimension.z = "50";
          xPhantomSize = 50.4; yPhantomSize = 50.4; zPhantomSize = 40.0;
      }

      // Density
      vmc->nDensity = 1;
      vmc->density[iBeam].x1 = "1";
      vmc->density[iBeam].x2 = vmc->dimension.x;
      vmc->density[iBeam].y1 = "1";
      vmc->density[iBeam].y2 = vmc->dimension.y;
      vmc->density[iBeam].z1 = "1";
      vmc->density[iBeam].z2 = vmc->dimension.z;
      vmc->density[iBeam].rho = "1.0";
      //
      vmc->write3dDose = "1";
      vmc->doseType = "4";
      if (ui->checkBoxMBSF->isChecked())vmc->doseType = "5";

      vmc->photoFactor = "1.0";

      float SSD = ssd.toFloat(&ok)/10;  // SSD = 100 cm
      // Phantom Specifications: Phantom Center
      float xCenter = xPhantomSize/2;
      float yCenter = yPhantomSize/2;
      float zCenter = zPhantomSize/2;
      // Number of Beams
      vmc->nBeams = 1;
      // Isocenter
      vmc->beam[iBeam].isocenter.x.sprintf("%.3f", xCenter);
      vmc->beam[iBeam].isocenter.y.sprintf("%.3f", yCenter);
      float zISO = 100 - SSD;
      vmc->beam[iBeam].isocenter.z.sprintf("%.3f", zISO);
      // Reference Depth
      float zRef = 10.0; // cm
      // Reference Point
      vmc->referencePoint.x.sprintf("%.3f", xCenter);
      vmc->referencePoint.y.sprintf("%.3f", yCenter);
      vmc->referencePoint.z.sprintf("%.3f", zRef);
      // Number of Fractions
      vmc->numFractions = "1";
      vmc->randomSet = "12345678";
      // Event Number
      vmc->beam[iBeam].waterHistory = "-10";
      vmc->beam[iBeam].historyRepeat = "50";
      vmc->beam[iBeam].futherRepeat = "1";
      vmc->beam[iBeam].batch = "20";
      // Gantry
      vmc->beam[iBeam].gantryMode = "F";
      vmc->beam[iBeam].gantryStart = "0";
      vmc->beam[iBeam].gantryStop = "360";
      vmc->beam[iBeam].tableAngle = "0";
      vmc->beam[iBeam].collAngle = "0";
      vmc->beam[iBeam].nominalEnergy = EE;
      // Irregular Field
      vmc->beam[iBeam].irregField = "0";
      // VMC Input Predefinitions
      vmc->beam[iBeam].beamWeight = "100.0";
      vmc->beam[iBeam].deviceKey = mName;
      vmc->beam[iBeam].deviceType = "103";  // Polyenergy Photon
      vmc->beam[iBeam].mlcMaterial = "tungsten";
      vmc->beam[iBeam].mlcOrient = usr->oMLC;
      vmc->beam[iBeam].nLeaves = usr->nMLC;
      vmc->beam[iBeam].mlcRadius = usr->rMLC;
      vmc->beam[iBeam].mlcCenter = usr->cMLC;
      vmc->beam[iBeam].mlcMode = "MLC";
      vmc->beam[iBeam].mlcModel = usr->MLCtype.section("-",0,0);

      int nMLChalf = 0;
      int iMLChalf = 0;
      int iMLCstart = 0;
      int iMLCend = 0;

      float uMLC = 0; // upper limit of MLC
      float lMLC = 0; // lower limit of MLC
      float maxFW = usr->MAXFW.toFloat(&ok)/10.0;
      float p1MLC = -0.5; // Closed MLC Position
      float p2MLC =  0.5; // Closed MLC Position
      if (usr->XIOMLC.contains("ElektaBM80leaf")) {
         p1MLC = -maxFW/2.0 - 0.1;
         p2MLC = -maxFW/2.0;
      }

      if (usr->isMLC == "1") {
          nMLChalf = usr->nMLC.toInt(&ok,10)/2;
          iMLChalf = usr->iMLC.toInt(&ok,10)/2;
          iMLCstart = (usr->nMLC.toInt(&ok,10)-usr->iMLC.toInt(&ok,10))/4+1;
          iMLCend = iMLCstart + usr->iMLC.toInt(&ok,10);

          uMLC = usr->SMD.toFloat(&ok) - usr->tMLC.toFloat(&ok)/2; // upper limit of MLC
          lMLC = usr->SMD.toFloat(&ok) + usr->tMLC.toFloat(&ok)/2; // lower limit of MLC
          p1MLC = -0.5; // Closed MLC Position
          p2MLC =  0.5; // Closed MLC Position
         if (usr->XIOMLC.contains("ElektaBM80leaf")) {
            p1MLC = -maxFW/2.0 - 0.1;
            p2MLC = -maxFW/2.0;
         }
      } else {
          vmc->beam[iBeam].nLeaves = "0";
      }
      QString qsUmlc; qsUmlc.sprintf("%7.3f",uMLC);
      vmc->beam[iBeam].mlcUpperLimit = qsUmlc;
      QString qsLmlc; qsLmlc.sprintf("%7.3f",lMLC);
      vmc->beam[iBeam].mlcLowerLimit = qsLmlc;

      vmc->beam[iBeam].collWidthX = FW;
      vmc->beam[iBeam].collWidthY = FL;
      float fwhf = FW.toFloat(&ok)/2.0/10.0;
      float flhf = FL.toFloat(&ok)/2.0/10.0;
      QString FWHF; FWHF.sprintf("%.2f",fwhf);
      QString FLHF; FLHF.sprintf("%.2f",flhf);
      vmc->beam[iBeam].collLeftX = "-" + FWHF;
      vmc->beam[iBeam].collLeftY = "-" + FLHF;
      vmc->beam[iBeam].collRightX = "+" + FWHF;
      vmc->beam[iBeam].collRightY = "+" + FLHF;
      float fs = FW.toFloat(&ok);
      if (vmc->beam[iBeam].mlcOrient.contains("Y")) fs = FL.toFloat(&ok);
      float halfFS = fs/2;
      float halfFScm = halfFS/10.0;
      int halfFSmm = (int)(10*fs/2);
      // std::cout << "getWaterData:: FW = " << FW << "  halfFSmm = " << halfFSmm << endl;

      int iMlc = 0;
      // MLC position unit is cm in XVMC and mm in Verify
      float MLCpos = (usr->nMLC.toInt(&ok,10)-usr->iMLC.toInt(&ok,10))/2
                   * usr->tkMLC.toFloat(&ok)
                   + usr->iMLC.toInt(&ok,10)/2 * usr->thMLC.toFloat(&ok);

      while (iMlc < usr->nMLC.toInt(&ok,10)) {
         float tMLC = usr->tkMLC.toFloat(&ok);
         if (iMlc >= iMLCstart && iMlc < iMLCend) tMLC = usr->thMLC.toFloat(&ok);
         MLCpos = MLCpos - tMLC;
         float tMLCcm = tMLC/10.0;
         int posMLCmm = (int)(10*MLCpos);
         if (posMLCmm >= halfFSmm) {
            vmc->beam[iBeam].leafWidth[iMlc].sprintf("%.2f",tMLCcm);
            vmc->beam[iBeam].leftStart[iMlc].sprintf("%.2f",p1MLC);
            vmc->beam[iBeam].rightStart[iMlc].sprintf("%.2f",p2MLC);
            // oStream << tMLCcm << " " << p1MLC << "  " <<  p2MLC << endl;
         } else {
            float MLCposPlus = MLCpos + tMLC;
            int posPlusMLCmm = (int)(10*MLCposPlus);
            if (posPlusMLCmm <= -halfFSmm) {
               vmc->beam[iBeam].leafWidth[iMlc].sprintf("%.2f",tMLCcm);
               vmc->beam[iBeam].leftStart[iMlc].sprintf("%.2f",p1MLC);
               vmc->beam[iBeam].rightStart[iMlc].sprintf("%.2f",p2MLC);
               //oStream << tMLCcm << "  " << p1MLC << "  " << p2MLC << endl;
            } else {
               vmc->beam[iBeam].leafWidth[iMlc].sprintf("%.2f",tMLCcm);
               vmc->beam[iBeam].leftStart[iMlc].sprintf("%.2f",-halfFScm);
               vmc->beam[iBeam].rightStart[iMlc].sprintf("%.2f",halfFScm);
               // oStream << tMLCcm << "  " << -halfFScm << "  " << halfFScm << endl;
            }
         }
         iMlc++;
      }

      QString SSDcm; SSDcm.sprintf("%d",(int)SSD);

      // get PDD
      // Check pr0 file exists
      QString PR_FILE = TELE_DIR+"/meas/pr"+ssd+"."+FW+"."+FL;
      QFile prFile(PR_FILE);

      planList = planList + EE+"MV."+FW+"x"+FL+"."+SSDcm;
      if (prFile.exists()) {
        QString mzName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".mz0";
        QString mzFile = PT_DIR_MEAS+"/"+mzName;
        float factor = outputFactor[iFS];
        float normPoint = zRef*10;  // Normalization Point (cm)
        // float xOffset = 0.0;
         float xOffset = ui->floatSpinBoxOffset->text().toFloat(&ok);
        int   iOpt = 11;   // Normalize at Y Max and Multiply the Factor
        getPDD(PR_FILE,mzFile,normPoint,factor,xOffset,iOpt);
        QString xCenterString; xCenterString.sprintf("%.3f", xCenter);
        vmc->zProfile[iBeam].x = xCenterString;
        QString yCenterString; yCenterString.sprintf("%.3f", yCenter);
        vmc->zProfile[iBeam].y = yCenterString;
        vmc->nProfiles.z = 1;
        planList = planList + " PDD";
      } else {
        vmc->nProfiles.z = 0;
      }

        // get x Profiles
      // Check ocrr.0.0 file exists
      QStringList xZLIST; xZLIST << "None";
      QString xOCRR = TELE_DIR+"/meas/ocrr."+ssd+".0."+FW+"."+FL;
      QFile xOcrrFile(xOCRR);
      if (xOcrrFile.exists()) {
        QString mxName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".mx";
        QString mxFile = PT_DIR_MEAS+"/"+mxName;
        float factor = outputFactor[iFS];
        float normPoint = zRef*10;  // Normalization Point (cm)
        float xOffset = 0.0;
        int   iOpt = 11;   // Normalize at Y Max and Multiply the Factor
        QString SDDs =
           getOCRR(xOCRR,mxFile,normPoint,factor,xOffset,iOpt);
        QStringList SDDList = SDDs.split("|",QString::SkipEmptyParts);
        xZLIST = SDDList;
        // std::cout << mxName << " xZLIST = " << xZLIST.join("|") << endl;
        if (xZLIST.isEmpty()) xZLIST << "None";
        int ix = 0;
        for (QStringList::Iterator sdd = SDDList.begin();
                                   sdd != SDDList.end(); ++sdd){
           QString yCenterString; yCenterString.sprintf("%.3f", yCenter);
           vmc->xProfile[ix].y = yCenterString;
           vmc->xProfile[ix].z = *sdd;
           ix++;
        }
        vmc->nProfiles.x = ix - 1;
        planList = planList + " XP";
      } else {
        vmc->nProfiles.x = 0;
      }

        // get Y profiles
      // Check ocrr.0.90 file exists
      QStringList yZLIST; yZLIST << "None";
      QString yOCRR = TELE_DIR+"/meas/ocrr."+ssd+".90."+FW+"."+FL;
      QFile yOcrrFile(yOCRR);
      if (yOcrrFile.exists()) {
        QString myName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".my";
        QString myFile = PT_DIR_MEAS+"/"+myName;
        float factor = outputFactor[iFS];
        float normPoint = zRef*10;  // Normalization Point (cm)
        float xOffset = 0.0;
        int   iOpt = 11;   // Normalize at Y Max and Multiply the Factor
        QString SDDs =
           getOCRR(yOCRR,myFile,normPoint,factor,xOffset,iOpt);
        QStringList SDDList = SDDs.split("|",QString::SkipEmptyParts);
        yZLIST = SDDList;
        // std::cout << myName << " yZLIST = " << yZLIST.join("|") << endl;
        if (yZLIST.isEmpty())yZLIST << "None";
        int iy = 0;
        for (QStringList::Iterator sdd = SDDList.begin();
                                   sdd != SDDList.end(); ++sdd){
           QString xCenterString; xCenterString.sprintf("%.3f", xCenter);
           vmc->yProfile[iy].x = xCenterString;
           vmc->yProfile[iy].z = *sdd;
           iy++;
        }
        vmc->nProfiles.y = iy - 1;
        planList = planList + " IP";
      } else {
        vmc->nProfiles.y = 0;
      }
      planList = planList + ":";
      QString planName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".vmc";
      QString vmcFile = PT_DIR_INP+"/"+planName;
      writeVMC(vmcFile);
#ifndef XVMC
      // Newly Added for Verify
      QString planVerify = EE+"MV."+FW+"x"+FL+"."+SSDcm;
      QString CLN_FILE = PT_DIR_INP + "/" + planVerify;
      //float xFScm = FW.toInt(&ok,10)/10;
      //float yFScm = FL.toInt(&ok,10)/10;
      float xFScm = FW.toFloat(&ok)/10.0;
      float yFScm = FL.toFloat(&ok)/10.0;
      if (ui->checkBoxFS->isChecked()) {
         // Change FWcm and FLcm with real field size checked from measured profiles
      float mFW = 0.0;
      float mFL = 0.0;
           if (xOcrrFile.exists()) {
           // std::cout << "FW(" << xFScm << ") = " << getFWHM(xOCRR,0.0) << endl;
             mFW = getFWHM(xOCRR,0.0);
           }
           if (yOcrrFile.exists()) {
           // std::cout << "FL(" << yFScm << ") = " << getFWHM(yOCRR,0.0) << endl;
             mFL = getFWHM(yOCRR,0.0);
           }
           if (mFW > 0.0 && mFL == 0.0) mFL = mFW;
           if (mFL > 0.0 && mFW == 0.0) mFW = mFL;
           if (mFW == 0.0 && mFL == 0.0) {
        mFW = xFScm;
        mFL = yFScm;
     }
     xFScm = mFW;
     yFScm = mFL;
     //std::cout << mFW << " " << xFScm << endl;
     //std::cout << mFL << " " << yFScm << endl;
      }
      QString FWcm; FWcm.sprintf("%.1f", xFScm);
      QString FLcm; FLcm.sprintf("%.1f", yFScm);
  // std::cout << "FW x FL = " << FWcm << " x " << FLcm << endl;
  if (PT_NAME.contains("PB_"))
    writePBCLN(CLN_FILE, FWcm+"x"+FLcm);
  else
    writeCLN(CLN_FILE, FWcm+"x"+FLcm);

  QString VER_FILE = PT_DIR_INP + "/" + planVerify;
  QString MCVariance; MCVariance.sprintf("%f", -0.001* vmc->RSD.toFloat(&ok));
  QString xCenterCM; xCenterCM.sprintf("%.3f", xCenter);
  QString yCenterCM; yCenterCM.sprintf("%.3f", yCenter);
  QString zCenterCM; zCenterCM.sprintf("%.3f", zCenter);
  QString SSDCM; SSDCM.sprintf("%.3f", 100.0 - SSD);
  QString Mode = "Verify";
  if (PT_NAME.contains("PB_")) Mode = "PB_";
  QString OPTIONS = "MCvariance = "+MCVariance
                  + "; voxelSize = " + vmc->voxelSize.x
                  + "; nX = " + vmc->dimension.x
                  + "; nY = " + vmc->dimension.y
                  + "; nZ = " + vmc->dimension.z
                  + "; xCenter = " + xCenterCM
                  + "; yCenter = " + yCenterCM
                  + "; zCenter = " + SSDCM
                  + "; density = 1.0; xDepth = " + xCenterCM
                  + "; yDepth = " + yCenterCM
                  + "; zDepth = " + zCenterCM
                  + "; Mode = " + Mode
                  + ";";

   if ((xFScm < 5.0 && yFScm < 5.0) || PT_NAME.contains("PB_")) {
      QString voxelSmall = ui->comboBoxGridSizeSmall->currentText();
     // QString gridSize = "0.2";
      QString gridSize = "0."+voxelSmall;
     QString xDim = "201";
     QString yDim = "201";
     QString zDim = "200";
      if (voxelSmall == "1") {
          xDim = "401"; yDim = "401"; zDim = "400";
      }
      if (voxelSmall == "2") {
          xDim = "201"; yDim = "201"; zDim = "200";
      }
      if (voxelSmall == "3") {
          xDim = "133"; yDim = "133"; zDim = "133";
      }
      if (voxelSmall == "4") {
          xDim = "101"; yDim = "101"; zDim = "100";
      }
      if (voxelSmall == "5") {
          xDim = "81"; yDim = "81"; zDim = "80";
      }
      if (voxelSmall == "6") {
          xDim = "67"; yDim = "67"; zDim = "50";
      }
      if (voxelSmall == "7") {
          xDim = "57"; yDim = "57"; zDim = "57";
      }
      if (voxelSmall == "8") {
          xDim = "51"; yDim = "51"; zDim = "51";
      }
     // QString MCSTD; MCSTD.sprintf("%f", -0.001* vmc->RSD.toFloat(&ok)/2.0); // MODIFIED
     QString MCSTD; MCSTD.sprintf("%f", -0.001* vmc->RSD.toFloat(&ok));
     QString xCenterS; xCenterS.sprintf("%.3f", gridSize.toFloat(&ok)*xDim.toFloat(&ok)/2.0);
     QString yCenterS; yCenterS.sprintf("%.3f", gridSize.toFloat(&ok)*yDim.toFloat(&ok)/2.0);
     QString zCenterS; zCenterS.sprintf("%.3f", gridSize.toFloat(&ok)*zDim.toFloat(&ok)/2.0);
     OPTIONS = "MCvariance = "+MCSTD
             + "; voxelSize = " + gridSize
             + "; nX = " + xDim
             + "; nY = " + yDim
             + "; nZ = " + zDim
             + "; xCenter = " + xCenterS
             + "; yCenter = " + yCenterS
             + "; zCenter = " + SSDCM
             + "; density = 1.0; xDepth = " + xCenterS
             + "; yDepth = " + yCenterS
             + "; zDepth = " + zCenterS
             + "; Mode = " + Mode
             + ";";
   }

   writeVER(VER_FILE, xZLIST, yZLIST, OPTIONS);
#endif
      iFS++;
    }

 } // if (passed)
 return(planList);
} // End of void MainConsole::getPRLists()
// -----------------------------------------------------------------------------
QString MainConsole::getWaterDataVMC() {
 QString planList = "";
 // Check machine directory exists in Loacl Home like Monaco
 QString mName = ui->comboBoxMachine->currentText();
 QString WORK_DIR = usr->LHOME + mName;
 QString PT_DIR = WORK_DIR + "/" + mName;
 // XiO TEL Directory
 QString TELE_DIR = ui->lineEditTeleDir->text() + mName;
 // Check and Make machine directory
 if (!isThereDir(PT_DIR)) makeDir(PT_DIR);
 // Check measurement directory exists
 QString PT_DIR_MEAS = PT_DIR + "/meas";
 if (!isThereDir(PT_DIR_MEAS)) makeDir(PT_DIR_MEAS);
 // Check calculation directory exists
 QString PT_DIR_CALC = PT_DIR + "/calc";
 if (!isThereDir(PT_DIR_CALC)) makeDir(PT_DIR_CALC);
 // Check VMC Incput directory exists
 QString PT_DIR_INP = PT_DIR + "/INPUT";
 if (!isThereDir(PT_DIR_INP)) makeDir(PT_DIR_INP);

 // Multi-Beam Plan
 int iBeam = 0;

 QString Energy =
         readLocalSetting(mName,"Energy");

 QString mInfoFile = mName + ".info";
 if (isThereFile(WORK_DIR, mInfoFile)) {
    getMachineInfo(WORK_DIR+"/"+mInfoFile);
 }

 bool ok;
 QString EE = Energy.section('.',0,0);

 // File for  Output Factors
 QString OP_FILE = TELE_DIR + "/op";
 QString RECT_OP_FILE = TELE_DIR + "/rect_op";

 bool passed = false;
 // Check Existing
 if (isThereFile(TELE_DIR, "op")
 //   && isThereFile(TELE_DIR, "rect_op")
   ) {
     passed = true;
 } else {
     QMessageBox::warning( this,
     "WARNING: Missing File(s)",
     "In-air output factor file(s) missed..."
     "Check op file\n"
     "in XiO Beam Data Directory.\n" +
     TELE_DIR,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
 }
 if (passed) {
    QString FS = getOutputFactor(OP_FILE,0,0.0);  // Field Size
    QString OF = getOutputFactor(OP_FILE,1,0.0);  // Output Factor
    // FS = FS + getOutputFactor(RECT_OP_FILE,2);
    // OF = OF + getOutputFactor(RECT_OP_FILE,3);
    QStringList FSList = FS.split("|",QString::SkipEmptyParts);
    QStringList OFList = OF.split("|",QString::SkipEmptyParts);
    float maxFS = 0.0; int iFS = 0; int iFSmax = 0;
    QString maxField = "";
    for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it){
      QString FieldSize = *it;
      float xFS = FieldSize.section('x',0,0).toFloat(&ok);
      float yFS = FieldSize.section('x',1,1).toFloat(&ok);
      // std::cout << iFS << "  " << xFS << " x " << yFS << endl;
      if (xFS*yFS >= maxFS) {
         maxFS = xFS*yFS;
         iFSmax = iFS;
         maxField = FieldSize;
      }
      iFS++;
    }

    float *outputFactor = (float *) calloc (iFS, sizeof(float));
    iFS = 0;
    float maxOF = 0.0;
    for (QStringList::Iterator it = OFList.begin(); it != OFList.end(); ++it){
      QString OutputFactor = *it;
      // std::cout << iFS << "  " << xFS << " x " << yFS << endl;
      outputFactor[iFS] = OutputFactor.toFloat(&ok);
      if (iFS == iFSmax) {
         maxOF = outputFactor[iFS];
      }
      iFS++;
    }

    // std::cout << "maxFS = " << maxFS << "  maxOF = " << maxOF << endl;
    iFS = 0;
    // float xOffset = 0.0; float yOffset = 0.0; float zOffset =0.0;
    QString ssd = "1000"; // 1000 mm
     ssd = usr->SSD;

    // Phantom Specifications: Voxel Size
    vmc->voxelSize.x = "0.5";
    vmc->voxelSize.y = "0.5";
    vmc->voxelSize.z = "0.5";
    float xVoxelSize = vmc->voxelSize.x.toFloat(&ok);
    float yVoxelSize = vmc->voxelSize.y.toFloat(&ok);
    float zVoxelSize = vmc->voxelSize.z.toFloat(&ok);
    // Phantom Specifications: Phantom Size
    float xPhantomSize = 50.5;
    float yPhantomSize = 50.5;
    float zPhantomSize = 40.0;
    // Phantom Specifications: Number of Voxels
    int nX = (int)(xPhantomSize/xVoxelSize);
    int nY = (int)(yPhantomSize/yVoxelSize);
    int nZ = (int)(zPhantomSize/zVoxelSize);
    vmc->dimension.x.sprintf("%d", nX);
    vmc->dimension.y.sprintf("%d", nY);
    vmc->dimension.z.sprintf("%d", nZ);
    // Density
    vmc->nDensity = 1;
    vmc->density[iBeam].x1 = "1";
    vmc->density[iBeam].x2 = vmc->dimension.x;
    vmc->density[iBeam].y1 = "1";
    vmc->density[iBeam].y2 = vmc->dimension.y;
    vmc->density[iBeam].z1 = "1";
    vmc->density[iBeam].z2 = vmc->dimension.z;
    vmc->density[iBeam].rho = "1.0";
    //
    vmc->write3dDose = "1";
    vmc->doseType = "4";
    vmc->photoFactor = "1.0";

    float SSD = ssd.toFloat(&ok)/10;  // SSD = 100 cm
    // Phantom Specifications: Phantom Center
    float xCenter = xPhantomSize/2;
    float yCenter = yPhantomSize/2;
    // float zCenter = zPhantomSize/2;
    // Number of Beams
    vmc->nBeams = 1;
    // Isocenter
    vmc->beam[iBeam].isocenter.x.sprintf("%.3f", xCenter);
    vmc->beam[iBeam].isocenter.y.sprintf("%.3f", yCenter);
    float zISO = 100 - SSD;
    vmc->beam[iBeam].isocenter.z.sprintf("%.3f", zISO);
    // Reference Depth
    float zRef = 10.0; // cm
    // Reference Point
    vmc->referencePoint.x.sprintf("%.3f", xCenter);
    vmc->referencePoint.y.sprintf("%.3f", yCenter);
    vmc->referencePoint.z.sprintf("%.3f", zRef);
    // Number of Fractions
    vmc->numFractions = "1";
    vmc->randomSet = "12345678";
    // Event Number
    vmc->beam[iBeam].waterHistory = "-10";
    vmc->beam[iBeam].historyRepeat = "50";
    vmc->beam[iBeam].futherRepeat = "1";
    vmc->beam[iBeam].batch = "20";
    // Gantry
    vmc->beam[iBeam].gantryMode = "F";
    vmc->beam[iBeam].gantryStart = "0";
    vmc->beam[iBeam].gantryStop = "360";
    vmc->beam[iBeam].tableAngle = "0";
    vmc->beam[iBeam].collAngle = "0";
    vmc->beam[iBeam].nominalEnergy = EE;
    // Irregular Field
    vmc->beam[iBeam].irregField = "0";
    // VMC Input Predefinitions
    vmc->beam[iBeam].beamWeight = "100.0";
    vmc->beam[iBeam].deviceKey = mName;
    vmc->beam[iBeam].deviceType = "103";  // Polyenergy Photon
    vmc->beam[iBeam].mlcMaterial = "tungsten";
    vmc->beam[iBeam].mlcOrient = usr->oMLC;
    vmc->beam[iBeam].nLeaves = usr->nMLC;
    vmc->beam[iBeam].mlcRadius = usr->rMLC;
    vmc->beam[iBeam].mlcCenter = usr->cMLC;
    vmc->beam[iBeam].mlcMode = "MLC";
    vmc->beam[iBeam].mlcModel = usr->MLCtype.section("-",0,0);

    int nMLChalf = 0;
    int iMLChalf = 0;
    int iMLCstart = 0;
    int iMLCend = 0;

    float uMLC = 0; // upper limit of MLC
    float lMLC = 0; // lower limit of MLC
    float p1MLC = -0.5; // Closed MLC Position
    float p2MLC =  0.5; // Closed MLC Position
    float maxFW = usr->MAXFW.toFloat(&ok)/10.0;
    if (usr->XIOMLC.contains("ElektaBM80leaf")) {
       p1MLC = -maxFW/2.0 - 0.1;
       p2MLC = -maxFW/2.0;
    }

    if (usr->isMLC == "1") {
       nMLChalf = usr->nMLC.toInt(&ok,10)/2;
       iMLChalf = usr->iMLC.toInt(&ok,10)/2;
       iMLCstart = (usr->nMLC.toInt(&ok,10)-usr->iMLC.toInt(&ok,10))/4+1;
       iMLCend = iMLCstart + usr->iMLC.toInt(&ok,10);

       uMLC = usr->SMD.toFloat(&ok) - usr->tMLC.toFloat(&ok)/2; // upper limit of MLC
       lMLC = usr->SMD.toFloat(&ok) + usr->tMLC.toFloat(&ok)/2; // lower limit of MLC
       p1MLC = -0.5; // Closed MLC Position
       p2MLC =  0.5; // Closed MLC Position
       if (usr->XIOMLC.contains("ElektaBM80leaf")) {
          p1MLC = -maxFW/2.0 - 0.1;
          p2MLC = -maxFW/2.0;
       }
    } else {
       vmc->beam[iBeam].nLeaves = "0";
    }

    QString qsUmlc; qsUmlc.sprintf("%7.3f",uMLC);
    vmc->beam[iBeam].mlcUpperLimit = qsUmlc;
    QString qsLmlc; qsLmlc.sprintf("%7.3f",lMLC);
    vmc->beam[iBeam].mlcLowerLimit = qsLmlc;

    for (QStringList::Iterator it = FSList.begin(); it != FSList.end(); ++it) {
      QString FS = *it;
      QString FW = FS.section('x',0,0).section('.',0,0);
      QString FL = FS.section('x',1,1).section('.',0,0);
      vmc->beam[iBeam].collWidthX = FW;
      vmc->beam[iBeam].collWidthY = FL;
      float fwhf = FW.toFloat(&ok)/2.0/10.0;
      float flhf = FL.toFloat(&ok)/2.0/10.0;
      QString FWHF; FWHF.sprintf("%.2f",fwhf);
      QString FLHF; FLHF.sprintf("%.2f",flhf);
      vmc->beam[iBeam].collLeftX = "-" + FWHF;
      vmc->beam[iBeam].collLeftY = "-" + FLHF;
      vmc->beam[iBeam].collRightX = "+" + FWHF;
      vmc->beam[iBeam].collRightY = "+" + FLHF;
      float fs = FW.toFloat(&ok);
      if (vmc->beam[iBeam].mlcOrient.contains("Y"))
         fs = FL.toFloat(&ok);
      float halfFS = fs/2;
      int halfFSmm = (int)(10*fs/2);

      int iMlc = 0;
      float MLCpos = (usr->nMLC.toInt(&ok,10)-usr->iMLC.toInt(&ok,10))/2
                   * usr->tkMLC.toFloat(&ok)
                   + usr->iMLC.toInt(&ok,10)/2 * usr->thMLC.toFloat(&ok);

      while (iMlc < usr->nMLC.toInt(&ok,10)) {
         float tMLC = usr->tkMLC.toFloat(&ok);
         if (iMlc >= iMLCstart && iMlc < iMLCend)
            tMLC = usr->thMLC.toFloat(&ok);
         MLCpos = MLCpos - tMLC;
         float tMLCcm = tMLC/10.0;
         if (MLCpos >= halfFSmm) {
            vmc->beam[iBeam].leafWidth[iMlc].sprintf("%.2f",tMLCcm);
            vmc->beam[iBeam].leftStart[iMlc].sprintf("%.2f",p1MLC);
            vmc->beam[iBeam].rightStart[iMlc].sprintf("%.2f",p2MLC);
            // oStream << tMLCcm << " " << p1MLC << "  " <<  p2MLC << endl;
         } else {
            float MLCposPlus = MLCpos + tMLC;
            if (MLCposPlus <= -halfFSmm) {
               vmc->beam[iBeam].leafWidth[iMlc].sprintf("%.2f",tMLCcm);
               vmc->beam[iBeam].leftStart[iMlc].sprintf("%.2f",p1MLC);
               vmc->beam[iBeam].rightStart[iMlc].sprintf("%.2f",p2MLC);
               //oStream << tMLCcm << "  " << p1MLC << "  " << p2MLC << endl;
            } else {
               vmc->beam[iBeam].leafWidth[iMlc].sprintf("%.2f",tMLCcm);
               vmc->beam[iBeam].leftStart[iMlc].sprintf("%.2f",-halfFS);
               vmc->beam[iBeam].rightStart[iMlc].sprintf("%.2f",halfFS);
               // oStream << tMLCcm << "  " << -halfFS << "  " << halfFS << endl;
            }
         }
         iMlc++;
      }

      QString SSDcm; SSDcm.sprintf("%d",(int)SSD);

      // Check pr0 file exists
      QString PR_FILE = TELE_DIR+"/meas/pr"+ssd+"."+FW+"."+FL;
      QFile prFile(PR_FILE);

      planList = planList + EE+"MV."+FW+"x"+FL+"."+SSDcm;
      if (prFile.exists()) {
        QString mzName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".mz0";
        QString mzFile = PT_DIR_MEAS+"/"+mzName;
        float factor = outputFactor[iFS];
        float normPoint = zRef*10;  // Normalization Point (cm)
        // float xOffset = 0.0;
          float xOffset = ui->floatSpinBoxOffset->text().toFloat(&ok);
        int   iOpt = 11;   // Normalize at Y Max and Multiply the Factor
        getPDD(PR_FILE,mzFile,normPoint,factor,xOffset,iOpt);
        QString xCenterString; xCenterString.sprintf("%.3f", xCenter);
        vmc->zProfile[iBeam].x = xCenterString;
        QString yCenterString; yCenterString.sprintf("%.3f", yCenter);
        vmc->zProfile[iBeam].y = yCenterString;
        vmc->nProfiles.z = 1;
        planList = planList + " PDD";
      } else {
        vmc->nProfiles.z = 0;
      }

      // Check ocrr.0.0 file exists
      QString xOCRR = TELE_DIR+"/meas/ocrr."+ssd+".0."+FW+"."+FL;
      QFile xOcrrFile(xOCRR);
      if (xOcrrFile.exists()) {
        QString mxName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".mx";
        QString mxFile = PT_DIR_MEAS+"/"+mxName;
        float factor = outputFactor[iFS];
        float normPoint = zRef*10;  // Normalization Point (cm)
        float xOffset = 0.0;
        int   iOpt = 11;   // Normalize at Y Max and Multiply the Factor
        QString SDDs =
           getOCRR(xOCRR,mxFile,normPoint,factor,xOffset,iOpt);
        QStringList SDDList = SDDs.split("|",QString::SkipEmptyParts);
        int ix = 0;
        for (QStringList::Iterator sdd = SDDList.begin();
                                   sdd != SDDList.end(); ++sdd){
           QString yCenterString; yCenterString.sprintf("%.3f", yCenter);
           vmc->xProfile[ix].y = yCenterString;
           vmc->xProfile[ix].z = *sdd;
           ix++;
        }
        vmc->nProfiles.x = ix - 1;
        planList = planList + " XP";
      } else {
        vmc->nProfiles.x = 0;
      }
      // Check ocrr.0.90 file exists
      QString yOCRR = TELE_DIR+"/meas/ocrr."+ssd+".90."+FW+"."+FL;
      QFile yOcrrFile(yOCRR);
      if (yOcrrFile.exists()) {
        QString myName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".my";
        QString myFile = PT_DIR_MEAS+"/"+myName;
        float factor = outputFactor[iFS];
        float normPoint = zRef*10;  // Normalization Point (cm)
        float xOffset = 0.0;
        int   iOpt = 11;   // Normalize at Y Max and Multiply the Factor
        QString SDDs =
           getOCRR(yOCRR,myFile,normPoint,factor,xOffset,iOpt);
        QStringList SDDList = SDDs.split("|",QString::SkipEmptyParts);
        int iy = 0;
        for (QStringList::Iterator sdd = SDDList.begin();
                                   sdd != SDDList.end(); ++sdd){
           QString xCenterString; xCenterString.sprintf("%.3f", xCenter);
           vmc->yProfile[iy].x = xCenterString;
           vmc->yProfile[iy].z = *sdd;
           iy++;
        }
        vmc->nProfiles.y = iy - 1;
        planList = planList + " IP";
      } else {
        vmc->nProfiles.y = 0;
      }
      planList = planList + ":";
      QString planName = EE+"MV."+FW+"x"+FL+"."+SSDcm+".vmc";
      QString vmcFile = PT_DIR_INP+"/"+planName;
      writeVMC(vmcFile);
      iFS++;
    }

 } // if (passed)
 return(planList);
}  // end of getWaterDataVMC()
// -----------------------------------------------------------------------------
QString MainConsole::getOutputFactor(QString inputFile, int iOpt, float SAD) {

   // iOpt = 0  FS for Square Fields
   // iOpt = 1  OF for Square Fields

   // iOpt = 2  FS for Rectanguar Fields
   // iOpt = 3  OF for Rectanguar Fields

   // iOpt = 5 SAD, refFS, and abdDose from op

   // iOpt + 10 for in-air data

   bool ok;
    QString TELE_DIR = ui->lineEditTeleDir->text();
    QString mName = ui->comboBoxMachine->currentText();
    float xOffset = ui->floatSpinBoxOffset->text().toFloat(&ok);
    float dmaxRef = 0.0;

   float CF = 4.0;
   if (iOpt == 2 || iOpt == 3 || iOpt == 12 || iOpt == 13) CF = 1.0;

   bool passed = true;
   // Status of kernel file
   FILE *istrm = fopen(inputFile.toLatin1(),"rt"); /* input stream */
   if(istrm==NULL) {
     QMessageBox::warning( this,
     "ERROR: No File(s) in getOutputFactor...",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }
   rewind(istrm); // Rewind istrm


   char version[MAX_STR_LEN];

   /* Read Format Version */
   if (fscanf(istrm, "%s", version) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: File Format Version in getOutputFactor",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }

    // Read Fields
    int nField = 0;  // Number of Fields
    if (fscanf(istrm, "%d", &nField) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: Number of Fields in getOutputFactor",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
    }
    float *xFS = (float *) calloc (nField+1, sizeof(float));
    float *yFS = (float *) calloc (nField+1, sizeof(float));
    float *yOF = (float *) calloc (nField+1, sizeof(float));
    float aValue = 0.0;
    QString FS = "";
    for (int i=0; i<nField; i++) {
     if (fscanf(istrm, "%f,", &aValue) == FAIL) {
         QMessageBox::warning( this,
         "ERROR: Field Sizes in getOutputFactor",
         "       Please check \n" + inputFile,
         "&Acknowleged", QString::null, QString::null, 1, 1 );
         passed = false;
     }
         xFS[i] = aValue*CF;
         yFS[i] = aValue*CF;
    }

   // In case of oa Read Another Number of Fields For Rectangular Fields
   if (iOpt == 2 || iOpt == 3 || iOpt == 12 || iOpt == 13) {
      if (fscanf(istrm, "%d", &nField) == FAIL) {
         QMessageBox::warning( this,
         "ERROR: Number of Fields in getOutputFactor",
         "       Please check \n" + inputFile,
         "&Acknowleged", QString::null, QString::null, 1, 1 );
         passed = false;
      }
      for (int i=0; i<nField; i++) {
        if (fscanf(istrm, "%f,", &aValue) == FAIL) {
          QMessageBox::warning( this,
          "ERROR: Field Sizes in getOutputFactor",
          "       Please check \n" + inputFile,
          "&Acknowleged", QString::null, QString::null, 1, 1 );
          passed = false;
        }
        yFS[i] = aValue*CF;
      }
   }

   // Read Output Factors
   QString OF = "";
   for (int i=0; i<nField; i++) {
      if (fscanf(istrm, "%f,", &aValue) == FAIL) {
        QMessageBox::warning( this,
        "ERROR: Output Factor in getOutputFactor",
        "       Please check \n" + inputFile,
        "&Acknowleged", QString::null, QString::null, 1, 1 );
        passed = false;
      }
      yOF[i]=aValue;
   }

 float refFS = 0.0;
 float absDose = 0.0;
 QString SSD = "1000";
 SSD = usr->SSD;
 if (iOpt == 0 || iOpt == 1 || iOpt == 5 || iOpt == 10 || iOpt == 11 || iOpt == 15) {
     char comments[MAX_STR_LEN];
     QString VER; VER.sprintf("%s", version);
     if (VER.contains("0001100d")) {
           fgets(comments, MAX_STR_LEN, istrm);  // Unknown 1
     //std::cout << "-1 :" << comments << endl;
           fgets(comments, MAX_STR_LEN, istrm);  // Unknown 2
     //std::cout << "0 :" << comments << endl;
     }
        fgets(comments, MAX_STR_LEN, istrm);   // Description
     //std::cout << "1 :" << comments << endl;
        fgets(comments, MAX_STR_LEN, istrm);   // Dose Unit
     //std::cout << "2 :" << comments << endl;
        fgets(comments, MAX_STR_LEN, istrm);   // Measurement Date
     //std::cout << "3 :" << comments << endl;
        fgets(comments, MAX_STR_LEN, istrm);   // Decay Inverse
     //std::cout << "4 :" << comments << endl;
        fgets(comments, MAX_STR_LEN, istrm); // Timer Error
     //std::cout << "5 :" << comments << endl;

     // Read SCD (called as SAD)
        if (fscanf(istrm, "%f,", &SAD) == FAIL) {
            QMessageBox::warning( this,
            "ERROR: SAD in getOutputFactor",
            "       Please check \n" + inputFile,
            "&Acknowleged", QString::null, QString::null, 1, 1 );
            passed = false;
        }
          usr->SAD.sprintf("%d", (int)SAD);
        // std::cout << "SAD = " << SAD << endl;

     // Read Reference Field Size at SCD
        if (fscanf(istrm, "%f,", &refFS) == FAIL) {
            QMessageBox::warning( this,
            "ERROR: FS(ref) in getOutputFactor",
            "       Please check \n" + inputFile,
            "&Acknowleged", QString::null, QString::null, 1, 1 );
            passed = false;
        }
        // std::cout << "FS(ref)=" << refFS << endl;

     // Read Absoulte Dose at SCD for Reference Field Size
        if (fscanf(istrm, "%f,", &absDose) == FAIL) {
            QMessageBox::warning( this,
            "ERROR: Absolute Dose in getOutputFactor",
            "       Please check \n" + inputFile,
            "&Acknowleged", QString::null, QString::null, 1, 1 );
            passed = false;
        }
        //std::cout << "absDose=" << absDose << endl;

     // Reference Field Size
     QString sFS;
     sFS.sprintf("%.1fx%.1f",refFS*1000.0/SAD,refFS*1000.0/SAD);
     QString refFW = sFS.section('x',0,0).section('.',0,0);
    QString refFL = sFS.section('x',1,1).section('.',0,0);

    // std::cout << " usr->REFDEPTH = " << usr->REFDEPTH << endl;
    // std::cout << " usr->SAD = " << usr->SAD << endl;
    float ssd = usr->SAD.toFloat(&ok) - usr->REFDEPTH.toFloat(&ok)*10;
    SSD.sprintf("%d",(int)ssd);
    usr->SSD = SSD;


     if (iOpt < 10){
     QString PR_FILE_REF = TELE_DIR+mName+"/meas/pr"+SSD+"."+refFW+"."+refFL;
       // std::cout << "PR_FILE_REF=" << PR_FILE_REF << endl;
       dmaxRef = getPDDmax(PR_FILE_REF, xOffset);
     }
 }

 // float refDepth = getParm("refdepth").toFloat(&ok);  // REMOVED

   for (int i=0; i<nField; i++) {
      QString strField;
      strField.sprintf("%.1fx%.1f",xFS[i]*1000.0/SAD,yFS[i]*1000.0/SAD);
      FS = FS + strField + "|";

      QString FW = strField.section('x',0,0).section('.',0,0);
      QString FL = strField.section('x',1,1).section('.',0,0);
      QString PR_FILE = TELE_DIR+mName+"/meas/pr"+SSD+"."+FW+"."+FL;
      QFile prFile(PR_FILE);
      float F = 1.0;
      if (prFile.exists() && iOpt < 10) {
         // std::cout << "strField=" << strField << endl;
         // std::cout << "PR_FILE=" << PR_FILE << endl;
         float dmax = getPDDmax(PR_FILE, xOffset);
         if (dmaxRef == 0.0) dmaxRef = dmax;
         // Mayneord F factor F.M. Kahn, The Physics of Radiation Therapy, 3rd Ed, pp167-168
         // float F1 = pow((100.0+dmax)/(SAD+dmax),2.0)*pow((SAD+refDepth)/(100.0+refDepth),2.0);
         // float F2 = pow((100.0+dmaxRef)/(SAD+dmaxRef),2.0)*pow((SAD+refDepth)/(100.0+refDepth),2.0);
         F = pow((100.0+dmax)/(SAD+dmax),2.0)/pow((100.0+dmaxRef)/(SAD+dmaxRef),2.0); // = F1/F2
         F = 1.0;  // No Correction is applied
      }
      QString strOF; strOF.sprintf("%.3f",yOF[i]*F);
      OF = OF + strOF + "|";
   }
   // std::cout << FS << endl;

   fclose(istrm);

   QString result;
   if (iOpt == 0 || iOpt == 2 || iOpt == 10 || iOpt == 12) result = FS;
   if (iOpt == 1 || iOpt == 3 || iOpt == 11 || iOpt == 13) result = OF;
   // refFS is reference field size at Isocenter in mm
   if (iOpt == 5 || iOpt == 15) result.sprintf("%f|%f|%f", SAD, refFS*1000.0/SAD, absDose);
   // std::cout << OF << endl;
   return (result);
}
// -----------------------------------------------------------------------------
float MainConsole::getPDDmax(QString iFile, float xOffset) {
 float dmax = 0.0;
   bool passed = true;
 // Status of kernel file
   FILE *istrm = fopen(iFile.toLatin1(),"rt"); /* input stream */
   if(istrm==NULL) {
     QMessageBox::warning( this,
     "ERROR: No File(s)... in getPDDmax",
     "       Please check \n" + iFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }
   rewind(istrm); // Rewind istrm

   if (passed) {
    char version[MAX_STR_LEN];

    /* Read Format Version */
    if (fscanf(istrm, "%s", version) == FAIL) {
      QMessageBox::warning( this,
      "ERROR: File Format Version in getPDDmax",
      "       Please check \n" + iFile,
      "&Acknowleged", QString::null, QString::null, 1, 1 );
      passed = false;
    }

    /* Read Fields */
    int nPoints = 0;  // Number of Fields
    if (fscanf(istrm, "%d", &nPoints) == FAIL) {
      QMessageBox::warning( this,
      "ERROR: Number of Fields in getPDDmax",
      "       Please check \n" + iFile,
      "&Acknowleged", QString::null, QString::null, 1, 1 );
      passed = false;
    }
    // Scan Points (sPoint) and Scan Value (sValue) Array
    float *sPoint = (float *) calloc (nPoints+1, sizeof(float));
    float *sValue = (float *) calloc (nPoints+1, sizeof(float));

    float aValue = 0.0;
    for (int i=0; i<nPoints; i++) {
      if (fscanf(istrm, "%f,", &aValue) == FAIL) {
         QMessageBox::warning( this,
         "ERROR: Field Sizes in getPDDmax",
         "       Please check \n" + iFile,
         "&Acknowleged", QString::null, QString::null, 1, 1 );
         passed = false;
      }
      sPoint[i] = aValue;
    }
    for (int i=0; i<nPoints; i++) {
      if (fscanf(istrm, "%f,", &aValue) == FAIL) {
         QMessageBox::warning( this,
         "ERROR: Field Sizes in getPDDmax",
         "       Please check \n" + iFile,
         "&Acknowleged", QString::null, QString::null, 1, 1 );
         passed = false;
      }
      sValue[i] = aValue;
    }

    fclose(istrm);

  float yMax = 0.0;
    for (int i=0; i<nPoints; i++) {
       if (sValue[i] > yMax) {
    dmax = sPoint[i];
    yMax = sValue[i];
   }
    }
 }
  return (dmax+xOffset);
} // float MainConsole::getPDDmax
// -----------------------------------------------------------------------------
QString MainConsole::getPDD(QString inputFile, QString outputFile,
                              float xp,
                              float factor,
                              float xOffset,
                              int   iOpt) {
  // dmax, mSDD (measured SDD)


  // iOpt = 0 Normalization to Max Y
  // iOpt = 10 Normalization to Max Y and multiply factor
  // iOpt = 1 Normalization to Y at xp with liniaer interpolation
  // iOpt = 11  iOpt=1 and then multiply the factor

   bool passed = true;
 // Status of kernel file
   FILE *istrm = fopen(inputFile.toLatin1(),"rt"); /* input stream */
   if(istrm==NULL) {
     QMessageBox::warning( this,
     "ERROR: No File(s)... in getPDD",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }
   rewind(istrm); // Rewind istrm


   char version[MAX_STR_LEN];

   /* Read Format Version */
   if (fscanf(istrm, "%s", version) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: File Format Version in getPDD",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }

   /* Read Fields */
   int nPoints = 0;  // Number of Fields
   if (fscanf(istrm, "%d", &nPoints) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: Number of Fields in getPDD",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }
   // Scan Points (sPoint) and Scan Value (sValue) Array
   float *sPoint = (float *) calloc (nPoints+1, sizeof(float));
   float *sValue = (float *) calloc (nPoints+1, sizeof(float));

   float aValue = 0.0;
   for (int i=0; i<nPoints; i++) {
     if (fscanf(istrm, "%f,", &aValue) == FAIL) {
        QMessageBox::warning( this,
        "ERROR: Field Sizes in getPDD",
        "       Please check \n" + inputFile,
        "&Acknowleged", QString::null, QString::null, 1, 1 );
        passed = false;
     }
     sPoint[i] = aValue;
   }
   for (int i=0; i<nPoints; i++) {
     if (fscanf(istrm, "%f,", &aValue) == FAIL) {
        QMessageBox::warning( this,
        "ERROR: Field Sizes in getPDD",
        "       Please check \n" + inputFile,
        "&Acknowleged", QString::null, QString::null, 1, 1 );
        passed = false;
     }
     sValue[i] = aValue;
   }

   normalize(sPoint,sValue,nPoints,xp,factor,xOffset,iOpt);

   fclose(istrm);

   FILE *ostrm = fopen(outputFile.toLatin1(),"wt");
   for (int i=0; i<nPoints; i++)
     if (sPoint[i] <= 395 || sPoint[i] >= 800)
        fprintf (ostrm, "%e\t%e\n", sPoint[i]/10.0, sValue[i]);

   fclose(ostrm);


  return ("");
} // void MainConsole::getPDD
// -----------------------------------------------------------------------------
QString MainConsole::getOCRR(QString inputFile, QString outputFile,
                              float xp,
                              float factor,
                              float xOffset,
                              int   iOpt) {

    // iOpt = 0 Normalization to Max Y
    // iOpt = 10 Normalization to Max Y and multiply factor
    // iOpt = 1 Normalization to Y at xp with liniae interpolation
    // iOpt = 11  iOpt=1 and then multiply factor
    // iOpt = 100  Reports Average Field Width

   bool passed = true;
   bool water = false;
   if (outputFile.section('.',-1) == "mx" ||
       outputFile.section('.',-1) == "my") water = true;

   QString mName = ui->comboBoxMachine->currentText();
   QString Energy =
         readLocalSetting(mName,"Energy");
   QString EE = Energy.section('.',0,0);

   FILE *istrm = fopen(inputFile.toLatin1(),"rt"); /* input stream */
   if(istrm==NULL) {
     QMessageBox::warning( this,
     "ERROR: No File(s)... in getOCRR",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }
   rewind(istrm); // Rewind istrm

   char version[MAX_STR_LEN];

   /* Read Format Version */
   if (fscanf(istrm, "%s", version) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: File Format Version in getOCRR",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }

   /* Read Source to Detector Distances */
   int nSDDs = 0;  // Number of Fields
   if (fscanf(istrm, "%d", &nSDDs) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: Number of Source to Detector Distances in getOCRR",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }

   float *SDD = (float *) calloc (nSDDs+1, sizeof(float));

   float aValue = 0.0;
   for (int i=0; i<nSDDs; i++) {
     if (fscanf(istrm, "%f,", &aValue) == FAIL) {
        QMessageBox::warning( this,
        "ERROR: Read Source to Detector Distances in getOCRR",
        "       Please check \n" + inputFile,
        "&Acknowleged", QString::null, QString::null, 1, 1 );
        passed = false;
     }
     SDD[i] = aValue;
   }

   QString strSDDList = "";
   for (int k=0; k<nSDDs; k++) {
      QString strSDD;
      float zPoint = SDD[k]/10.0;
      if (water) strSDD.sprintf("%.3f", zPoint);
      else       strSDD.sprintf("%03d",(int)zPoint);

      int nPoints = 0;
      // Scan Points (sPoint) and Scan Value (sValue) Array
      if (fscanf(istrm, "%d", &nPoints) == FAIL) {
        QMessageBox::warning( this,
        "ERROR: Number of Scan Points in getOCRR",
        "       Please check \n" + inputFile,
        "&Acknowleged", QString::null, QString::null, 1, 1 );
        passed = false;
      }

      float *sPoint = (float *) calloc (nPoints+1, sizeof(float));
      float *sValue = (float *) calloc (nPoints+1, sizeof(float));

      float aValue = 0.0;
      for (int i=0; i<nPoints; i++) {
         if (fscanf(istrm, "%f,", &aValue) == FAIL) {
            QMessageBox::warning( this,
            "ERROR: Read Scan Position in getOCRR",
            "       Please check \n" + inputFile,
            "&Acknowleged", QString::null, QString::null, 1, 1 );
            passed = false;
         }
         sPoint[i] = aValue;
      }
      for (int i=0; i<nPoints; i++) {
         if (fscanf(istrm, "%f,", &aValue) == FAIL) {
            QMessageBox::warning( this,
            "ERROR: Read Scan Value in getOCRR",
            "       Please check \n" + inputFile,
            "&Acknowleged", QString::null, QString::null, 1, 1 );
            passed = false;
         }
         sValue[i] = aValue;
      }

      QString zFileName = outputFile.section('/',-1);
      QString zFilePath = outputFile.section(zFileName,0,0);
      if (water) {
         QString fwflssd = zFileName.section("MV.",1,1).section(".m",0,0);
         zFileName = zFilePath + EE+"MV."+fwflssd+".mz0";
      } else {
         QString fwfl = zFileName.mid(1,zFileName.length()-1);
         zFileName = zFilePath + "z" + fwfl + ".000";
      }
      // std::cout << "getOCRR::zFileName = " << zFileName << endl;

      factor = pickOnePoint2Colums(zFileName, zPoint);

        // Find index of x=0 or y=0
        int iZero = 0;
        for (int i=0; i<nPoints; i++) {
       if (sPoint[i] == 0.0) iZero = i;
        }
        if (iZero > 0) {
        for (int i=0; i<iZero; i++) {
            if (iZero+i < nPoints) {
                  if (ui->checkBoxAvg->isChecked()) {
                     float avg = (sValue[iZero-i] + sValue[iZero+i])/2.0;
                        sValue[iZero-i] = avg;
                        sValue[iZero+i] = avg;
                  }

                  if (ui->checkBoxHigh->isChecked()) {
                     float high = (sValue[iZero-i] > sValue[iZero+i]) ? (sValue[iZero-i]) : (sValue[iZero+i]);
                        sValue[iZero-i] = high;
                        sValue[iZero+i] = high;
                  }

                  if (ui->checkBoxLow->isChecked()) {
                     float low = (sValue[iZero-i] < sValue[iZero+i]) ? (sValue[iZero-i]) : (sValue[iZero+i]);
                        sValue[iZero-i] = low;
                        sValue[iZero+i] = low;
                  }
              }
        }
        }
      xp = 0.0;
      normalize(sPoint,sValue,nPoints,xp,factor,xOffset,iOpt);

      QString ocrrOutputFile = outputFile + "." + strSDD;
      if (water) {
        QString iDepth; iDepth.sprintf("%d",k);
        ocrrOutputFile = outputFile + iDepth;
      }
      FILE *ostrm = fopen(ocrrOutputFile.toLatin1(),"wt");
      for (int i=0; i<nPoints; i++)
        fprintf (ostrm, "%e\t%e\n", sPoint[i]/10.0, sValue[i]);

      fclose(ostrm);
      strSDDList = strSDDList + "|" + strSDD;
  }

  fclose(istrm);
  return (strSDDList);
} // void MainConsole::getPDD
// -----------------------------------------------------------------------------
float MainConsole::getFWHM(QString inputFile, float xOffset) {
   bool passed = true;
   bool water = false;
   xOffset = xOffset * 1.0;

   FILE *istrm = fopen(inputFile.toLatin1(),"rt"); /* input stream */
   if(istrm==NULL) {
     QMessageBox::warning( this,
     "ERROR: No File(s)... in getFWHM",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }
   rewind(istrm); // Rewind istrm

   char version[MAX_STR_LEN];

   /* Read Format Version */
   if (fscanf(istrm, "%s", version) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: File Format Version in getFWHM",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }

   /* Read Source to Detector Distances */
   int nSDDs = 0;  // Number of Fields
   if (fscanf(istrm, "%d", &nSDDs) == FAIL) {
     QMessageBox::warning( this,
     "ERROR: Number of Source to Detector Distances in getFWHM",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }

   float *SDD = (float *) calloc (nSDDs+1, sizeof(float));
   vector<float> FW;
   float aValue = 0.0;
   for (int i=0; i<nSDDs; i++) {
     if (fscanf(istrm, "%f,", &aValue) == FAIL) {
        QMessageBox::warning( this,
        "ERROR: Read Source to Detector Distances in getFWHM",
        "       Please check \n" + inputFile,
        "&Acknowleged", QString::null, QString::null, 1, 1 );
        passed = false;
     }
     SDD[i] = aValue;
   }

   float FieldWidth = 0.0;
   for (int k=0; k<nSDDs; k++) {
      QString strSDD;
      float zPoint = SDD[k]/10.0;
      if (water) strSDD.sprintf("%.3f", zPoint);
      else       strSDD.sprintf("%03d",(int)zPoint);

      int nPoints = 0;
      // Scan Points (sPoint) and Scan Value (sValue) Array
      if (fscanf(istrm, "%d", &nPoints) == FAIL) {
        QMessageBox::warning( this,
        "ERROR: Number of Scan Points in getFWHM",
        "       Please check \n" + inputFile,
        "&Acknowleged", QString::null, QString::null, 1, 1 );
        passed = false;
      }

      float *sPoint = (float *) calloc (nPoints+1, sizeof(float));
      float *sValue = (float *) calloc (nPoints+1, sizeof(float));
      float *nValue = (float *) calloc (nPoints+1, sizeof(float));

      float aValue = 0.0;
      for (int i=0; i<nPoints; i++) {
         if (fscanf(istrm, "%f,", &aValue) == FAIL) {
            QMessageBox::warning( this,
            "ERROR: Read Scan Position in getFWHM",
            "       Please check \n" + inputFile,
            "&Acknowleged", QString::null, QString::null, 1, 1 );
            passed = false;
         }
         sPoint[i] = aValue;
      }
      for (int i=0; i<nPoints; i++) {
         if (fscanf(istrm, "%f,", &aValue) == FAIL) {
            QMessageBox::warning( this,
            "ERROR: Read Scan Value in getFWHM",
            "       Please check \n" + inputFile,
            "&Acknowleged", QString::null, QString::null, 1, 1 );
            passed = false;
         }
         sValue[i] = aValue;
      }

      // Find value at x=0.0 or y=0.0
      float centerValue = 0.0;
      for (int i=0; i<nPoints-1; i++) {
         if (sPoint[i]*sPoint[i+1] <= 0.0) {
             centerValue = sValue[i]
               + (sValue[i+1] - sValue[i])
               / (sPoint[i+1] - sPoint[i])
               * (0.0 - sValue[i]);
         }
   if (sPoint[i] == 0.0) centerValue = sValue[i];
      }
      if (centerValue == 0.0) centerValue = 1.0;
      for (int i=0; i<nPoints-1; i++) {
         nValue[i] = sValue[i]/centerValue;
      }
    // Find y50
      vector<float> x50;
      for (int i=0; i<nPoints-1; i++) {
         if ((nValue[i]-0.5)*(nValue[i+1]-0.5) < 0.0){
            //std::cout << sPoint[i] << " " << nValue[i] << endl;
            //std::cout << sPoint[i+1] << " " << nValue[i+1] << endl;
            float x = sPoint[i]
            + (sPoint[i+1] - sPoint[i])
            / (nValue[i+1] - nValue[i])
            * (0.5 - nValue[i]);
            x50.push_back(x);
         }
      }

      if (x50.size() > 1) {
      //  std::cout << outputFile << endl;
    //  std::cout << "Nominal Field Range at " << SDD[k] << " SDD " << x50[0] << " to " << x50[1] << endl;
    //  std::cout << "Field Size = " << (x50[1] - x50[0]) * 1000.0 / (1000.0+SDD[k]) << endl;
    FW.push_back((x50[1] - x50[0]) * 1000.0 / (1000.0+SDD[k]));
     }
  }
  float sumFW = 0.0;
  for (unsigned int i=0; i<FW.size(); i++) sumFW += FW[i];
  if (FW.size() > 0) FieldWidth = sumFW/FW.size()/10.0;

  return (FieldWidth);
} // void MainConsole::getFWHM
// -----------------------------------------------------------------------------
void MainConsole::normalize(float *x, float *y, int nPoints,
                            float xp, float factor, float xOffset,
                            int iOpt) {
  // iOpt = 0 Normalization to Max Y
  // iOpt = 10 Normalization to Max Y and multiply factor
  // iOpt = 1 Normalization to Y at xp with liniaer interpolation
  // iOpt = 11  iOpt=1 and then multiply factor

   /* Find Maximum Y */
   unsigned int iMax= 0;
   float        yMax=y[0];
   for (int i=0; i<nPoints; i++) {
   x[i] += xOffset;  // Apply xOffset before normalize
     if (y[i] >= yMax) {
       yMax = y[i];
       iMax = i;
     }
   }

   float maxValue =yMax;

   if (iOpt == 1 || iOpt == 11) {
     for (int i=0; i<nPoints-1; i++) {
       if ((x[i]-xp) == 0.0) maxValue = y[i];
       if ((x[i+1]-xp) == 0.0) maxValue = y[i+1];
       if ((x[i]-xp)*(x[i+1]-xp) < 0.0) {
          maxValue = (fabs(x[i+1]-xp)*y[i]+fabs(x[i]-xp)*y[i+1])
                        /fabs(x[i+1]-x[i]);
       }
     }
   }


   float FACTOR = 1.0;
   if (iOpt == 11 || iOpt == 10) FACTOR *= factor;
   for (int i=0; i<nPoints;i++) {
      // x[i] += xOffset;

      if (maxValue != 0.0) y[i] = y[i]/maxValue*FACTOR;
   }

}
// -----------------------------------------------------------------------------
float MainConsole::pickOnePoint2Colums(QString inputFile, float &xPoint) {

   float yValue = 0.0;
   bool passed = true;
   FILE *istrm = fopen(inputFile.toLatin1(),"rt"); /* input stream */
   if(istrm==NULL) {
     QMessageBox::warning( this,
     "ERROR: No File(s)... in pickOnePoint2Colums",
     "       Please check \n" + inputFile,
     "&Acknowleged", QString::null, QString::null, 1, 1 );
     passed = false;
   }
   if (passed) {
   rewind(istrm); // Rewind istrm

 // Check Number of Data Lines
 int nLines = 0;
 int iCol = 0;
 int iComment = -1;
 char iCh;
 while (feof(istrm) == 0) {
  iCh = (char)fgetc(istrm);

  switch (iCh) {
   case ('\n'): {
    nLines++;
    if (iComment == 0) nLines--;
    iCol = 0;  // next character will be at 1st column
    iComment = -1; // Not Comment
    break;
   }
   case ('#') : {
    iComment = iCol;
    iCol ++;
    // printf ("nLines = %d iCol = %d\n", nLines, iCol);
    break;
   }
   default: {
    iCol++;
    break;
   }
  }
 }
 // printf ("Lines = %d\n", nLines);
 int nLine = nLines;

 int iLine = 0;
 float *x = (float *) calloc (nLine+1, sizeof(float));
 float *y = (float *) calloc (nLine+1, sizeof(float));

 rewind(istrm); // Rewind istrm
 fpos_t iPos;
 fgetpos(istrm, &iPos);
 iCol = 0; iComment = -1;
 // char aLine[256];
 while (feof(istrm) == 0) {
  iCh = (char)fgetc(istrm);

  switch (iCh) {
   case ('\n'): {
    iLine++;
    if (iComment == 0) iLine--;

    char *aLine = (char *) malloc((iCol*2)*sizeof(char));

    fsetpos(istrm, &iPos); // Step back to column 1

    for (int i=0; i <= iCol; i++) {
     aLine[i] = fgetc(istrm);
     if (aLine[i] == '#') {
      aLine[i] = '\0';
     }
    }

    if (strlen(aLine) > 0) {
     float xp, yp;
     sscanf(aLine, "%f %f", &xp, &yp);
     x[iLine-1] = xp;
     y[iLine-1] = yp;
               // std::cout << x[iLine-1] << "  " << y[iLine-1] << endl;
    }

    free (aLine);

    iCol = 0;  // next character will be at 1st column
    iComment = -1; // Not Comment
    if (iLine < nLine) fgetpos(istrm, &iPos);

    break;
   }
   case ('#') : {
    iComment = iCol;
    iCol ++;
    // printf ("iLine = %d iCol = %d\n", iLine, iCol);
    break;
   }
   default: {
    iCol++;
    break;
   }
  }
 }
 // printf ("Lines = %d\n", iLine);
 fclose(istrm);

 if (xPoint < x[0]) {
  yValue = y[1]-(y[1]-y[0])/(x[1]-x[0])*(x[1]-xPoint);
 }
 if (xPoint > x[iLine-1]) {
  yValue = y[iLine-1]+(y[iLine-1]-y[iLine-2])
             /(x[iLine-1]-x[iLine-2])*(xPoint-x[iLine-2]);
 }
 for (int i = 0; i < iLine-1; i++) {
    if ((x[i]-xPoint)*(x[i+1]-xPoint) <= 0.0) {
    yValue = y[i]+(y[i+1]-y[i])/(x[i+1]-x[i])*(xPoint-x[i]);
    break;
  }
   }
 }
   return(yValue);
}
// -----------------------------------------------------------------------------
void MainConsole::writeVMC(QString FNAME) {
   QFile oFile( FNAME );
   oFile.remove();
   oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
   QTextStream oStream( &oFile );

   oStream << "*PHANTOM        | " << endl;
   oStream << "-VOXELSIZE      | " << vmc->voxelSize.x
                           << "  " << vmc->voxelSize.y
                           << "  " << vmc->voxelSize.z << endl;
   oStream << "-DIMENSION      | " << vmc->dimension.x
                           << "  " << vmc->dimension.y
                           << "  " << vmc->dimension.z << endl;
   for (int i=0; i<vmc->nDensity; i++) {
      if (i == 0) oStream << "-CHANGE-DENSITY | ";
      else        oStream << "                | ";
         oStream << vmc->density[i].x1 << "  " << vmc->density[i].x2
         << "  " << vmc->density[i].y1 << "  " << vmc->density[i].y2
         << "  " << vmc->density[i].z1 << "  " << vmc->density[i].z2
         << "  " << vmc->density[i].rho << endl;
   }
   oStream << "!" << endl;

   // Write GLOBAL Data
   oStream << "*GLOBAL-DATA    | " << endl;
   oStream << "-WRITE-3D-DOSE  | " << vmc->write3dDose << endl;
   // oStream << "-E-CUTOFF       | " << "0.25" << endl;
   for (int i=0; i<vmc->nPlanes.z; i++)
      oStream << "-XY-PLANE       | " << vmc->xyPlane[i] << endl;
   for (int i=0; i<vmc->nPlanes.y; i++)
      oStream << "-XZ-PLANE       | " << vmc->xzPlane[i] << endl;
   for (int i=0; i<vmc->nPlanes.x; i++)
      oStream << "-YZ-PLANE       | " << vmc->yzPlane[i] << endl;
   for (int i=0; i<vmc->nProfiles.x; i++)
      oStream << "-X-PROFILE      | " << vmc->xProfile[i].y
                                 << "  " << vmc->xProfile[i].z << endl;
   for (int i=0; i<vmc->nProfiles.y; i++)
      oStream << "-Y-PROFILE      | " << vmc->yProfile[i].x
                                 << "  " << vmc->yProfile[i].z << endl;
   for (int i=0; i<vmc->nProfiles.z; i++)
      oStream << "-Z-PROFILE      | " << vmc->zProfile[i].x
                                 << "  " << vmc->zProfile[i].y << endl;
   oStream << "-DOSE-TYPE      | " << vmc->doseType << endl;
   vmc->k0cut = "0.25";
   vmc->k1cut = "2.0";
   oStream << "-P0-CUTOFF-KERMA| " << vmc->k0cut << endl;
   oStream << "-P1-CUTOFF-KERMA| " << vmc->k1cut << endl;
   oStream << "-NUM-FRACTIONS  | " << vmc->numFractions
                           << "  " << vmc->photoFactor << endl;
   oStream << "-REFERENCE-POINT| " << vmc->referencePoint.x
                           << "  " << vmc->referencePoint.y
                           << "  " << vmc->referencePoint.z << endl;
   oStream << "-RANDOM-SET     | " << vmc->randomSet.mid(0,2)
                           << "  " << vmc->randomSet.mid(2,2)
                           << "  " << vmc->randomSet.mid(4,2)
                           << "  " << vmc->randomSet.mid(6,2) << endl;
   oStream << "!" << endl;
   // BEAM-PARAMETERS
   for (int iBeam=0; iBeam<vmc->nBeams; iBeam++) {
      oStream << "*BEAM-PARAMETERS| " << endl;
      oStream << "-BEAM-WEIGHT    | " << vmc->beam[iBeam].beamWeight << endl;
      oStream << "-DEVICE-TYPE    | " << vmc->beam[iBeam].deviceType << endl;
      oStream << "-DEVICE-KEY     | " << vmc->beam[iBeam].deviceKey << endl;
      if (vmc->beam[iBeam].deviceType.length() < 3)
         oStream << "-APPLICATOR     | " << vmc->beam[iBeam].applicator << endl;
      oStream << "-NOMINAL-ENERGY | " << vmc->beam[iBeam].nominalEnergy << endl;
      if (vmc->beam[iBeam].waterHistory == "")
         vmc->beam[iBeam].waterHistory = "-" + vmc->RSD + "0";
      if (vmc->beam[iBeam].historyRepeat == "")
         vmc->beam[iBeam].historyRepeat = "50";
      if (vmc->beam[iBeam].futherRepeat == "")
         vmc->beam[iBeam].futherRepeat = "1";
      if (vmc->beam[iBeam].batch == "")
         vmc->beam[iBeam].batch = "20";
      if (vmc->RSD != "") vmc->beam[iBeam].waterHistory = vmc->RSD;
      oStream << "-EVENT-NUMBER   | "
                       << vmc->beam[iBeam].waterHistory.section('.',0,0)
               << "  " << vmc->beam[iBeam].historyRepeat
               << "  " << vmc->beam[iBeam].futherRepeat
               << "  " << vmc->beam[iBeam].batch << endl;
      oStream << "-ISOCENTER      | "
                       << vmc->beam[iBeam].isocenter.x
               << "  " << vmc->beam[iBeam].isocenter.y
               << "  " << vmc->beam[iBeam].isocenter.z << endl;
      oStream << "-GANTRY-ANGLE   | "
                       << vmc->beam[iBeam].gantryMode
               << "  " << vmc->beam[iBeam].gantryStart
               << "  " << vmc->beam[iBeam].gantryStop << endl;
      oStream << "-TABLE-ANGLE    | "
                       << vmc->beam[iBeam].tableAngle << endl;
      oStream << "-COLL-ANGLE     | " << vmc->beam[iBeam].collAngle << endl;
      if (vmc->beam[iBeam].deviceType.length() > 2) {
         oStream << "-COLL-LEFT-X    | " << vmc->beam[iBeam].collLeftX  << endl;
         oStream << "-COLL-RIGHT-X   | " << vmc->beam[iBeam].collRightX << endl;
         oStream << "-COLL-LEFT-Y    | " << vmc->beam[iBeam].collLeftY  << endl;
         oStream << "-COLL-RIGHT-Y   | " << vmc->beam[iBeam].collRightY << endl;
      } else {
         oStream << "-COLL-WIDTH-X   | " << vmc->beam[iBeam].collWidthX << endl;
         oStream << "-COLL-WIDTH-Y   | " << vmc->beam[iBeam].collWidthY << endl;
      }
      QString nPointsStr = vmc->beam[iBeam].irregField.simplified();
      bool ok;
      int nPoints = nPointsStr.toInt(&ok,10);
      if (nPoints > 0) {
         oStream << "-IRREGULAR-FIELD| " << vmc->beam[iBeam].irregField << endl;
         for (int i=0; i<nPoints; i++)
            oStream << "  " << vmc->beam[iBeam].irregX[i]
                    << "  " << vmc->beam[iBeam].irregY[i] << endl;
      }

      QString nLeavesStr = vmc->beam[iBeam].nLeaves.simplified();
      int nLeaves = nLeavesStr.toInt(&ok,10);
      if (nLeaves > 0) {
         QString mlcModel = "";
         if (vmc->beam[iBeam].mlcMode == "MLC") {
            if (vmc->beam[iBeam].mlcModel == "SIMPLE")
                  mlcModel = "-SIMPLE-MLC     | ";
            if (vmc->beam[iBeam].mlcModel == "DBLFOCUS")
                  mlcModel = "-DBFOCUS-MLC    | ";
            if (vmc->beam[iBeam].mlcModel == "RNDFOCUS")
                  mlcModel = "-RNDFOCUS-MLC   | ";
            if (vmc->beam[iBeam].mlcModel == "ELEKTA")
                  mlcModel = "-ELEKTA-MLC     | ";
            if (vmc->beam[iBeam].mlcModel == "VARIAN")
                  mlcModel = "-VARIAN-MLC     | ";

            oStream << mlcModel
                    << vmc->beam[iBeam].nLeaves << "  "
                    << vmc->beam[iBeam].mlcOrient << "  "
                    << vmc->beam[iBeam].mlcMaterial << "  "
                    << vmc->beam[iBeam].mlcUpperLimit << "  "
                    << vmc->beam[iBeam].mlcLowerLimit << "  "
                    << vmc->beam[iBeam].mlcCenter << "  "
                    << vmc->beam[iBeam].mlcRadius << endl;
            for (int i=0; i<nLeaves; i++)
               oStream << "  " << vmc->beam[iBeam].leafWidth[i]
                       << "  " << vmc->beam[iBeam].leftStart[i]
                       << "  " << vmc->beam[iBeam].rightStart[i]
                       << endl;
         }
         if (vmc->beam[iBeam].mlcMode == "DMLC") {
            if (vmc->beam[iBeam].mlcModel == "SIMPLE")
                  mlcModel = "-SIMPLE-DMLC    | ";
            if (vmc->beam[iBeam].mlcModel == "DBLFOCUS")
                  mlcModel = "-DBFOCUS-DMLC   | ";
            if (vmc->beam[iBeam].mlcModel == "RNDFOCUS")
                  mlcModel = "-RNDFOCUS-DMLC  | ";
            if (vmc->beam[iBeam].mlcModel == "ELEKTA")
                  mlcModel = "-ELEKTA-DMLC    | ";
            if (vmc->beam[iBeam].mlcModel == "VARIAN")
                  mlcModel = "-VARIAN-DMLC    | ";
            oStream << mlcModel
                    << vmc->beam[iBeam].nLeaves << "  "
                    << vmc->beam[iBeam].mlcOrient << "  "
                    << vmc->beam[iBeam].mlcMaterial << "  "
                    << vmc->beam[iBeam].mlcUpperLimit << "  "
                    << vmc->beam[iBeam].mlcLowerLimit << "  "
                    << vmc->beam[iBeam].mlcCenter << "  "
                    << vmc->beam[iBeam].mlcRadius << endl;
            for (int i=0; i<nLeaves; i++) {
               if (vmc->beam[iBeam].leftStop[i] == "")
                  vmc->beam[iBeam].leftStop[i] = vmc->beam[iBeam].leftStart[i];
               if (vmc->beam[iBeam].rightStop[i] == "")
                  vmc->beam[iBeam].rightStop[i] = vmc->beam[iBeam].rightStart[i];
               oStream << "  " << vmc->beam[iBeam].leafWidth[i]
                       << "  " << vmc->beam[iBeam].leftStart[i]
                       << "  " << vmc->beam[iBeam].rightStart[i]
                       << "  " << vmc->beam[iBeam].leftStop[i]
                       << "  " << vmc->beam[iBeam].rightStop[i] << endl;
            }
         }
      }
      oStream << "!" << endl;
   }
   oStream << "*END-INPUT      |" << endl;

   oFile.close();
}
// -----------------------------------------------------------------------------
void MainConsole::writeCLN(QString FNAME, QString pList) {
    QString mName = ui->comboBoxMachine->currentText();
    QString mInfoFile = usr->LHOME + mName + "/" + mName + ".info" ;
    getMachineInfo(mInfoFile);
    QString fname = FNAME+".CLN";
    QFile oFile( fname );
    oFile.remove();
    oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
    QTextStream oStream( &oFile );

    QString maxDoseVariance = "0.02";
    oStream << "!MAXDOSEVARIANCE " << maxDoseVariance << endl;
    oStream << "!DOSE_ENGINES" << endl;
    // oStream << "#  PB=e0 machinename=" << upd->PBmachine << endl;
    oStream << "  MC=e1 machinename=" << mName + ".bdt" << endl;
    oStream << "!END" << endl;
    oStream << "!MOD_EQUIPMENT" << endl;

    QString MLCtype = "";
     // std::cout << "writeCLN::usr->XIOMLC = " << usr->XIOMLC << endl;
    if (usr->XIOMLC.contains("ElektaBM80leaf")) MLCtype = "elektaModuleaf";
    if (usr->XIOMLC.contains("MILLENNIUM120")) MLCtype = "varian60";
    if (usr->XIOMLC.contains("PHILIPSMLC")) MLCtype = "elekta40";
    if (usr->XIOMLC.contains("SIEMENSV50")) MLCtype = "siemens27";
    if (usr->XIOMLC.contains("VARIAN26")) MLCtype = "varian52";
    if (usr->XIOMLC.contains("VARIAN40")) MLCtype = "varian40";
    if (usr->XIOMLC.contains("OPTIFOCUS82")) MLCtype = "siemens41";
    /* Monaco Supports these MLC types ONLY
       "staticMLC_Elekta"  "smartMLC"  "elekta40"
       "elektaSplit"  "elektaModuleaf"  "elektaIModuleaf"
       "siemens27" "siemens41" "varian60"  "varian52"
       "varian40";
    */
    QString segmentSize = "10x2"; // Modified on Jan 19, 2007 from 10x10
     if (MLCtype == "varian60") segmentSize = "5x2"; // Added on Jan 19, 2007
     if (MLCtype == "elektaModuleaf") segmentSize = "4x2"; // Added on Jan 19, 2007
    oStream << "   type=" << MLCtype << " size=" << segmentSize << endl;
    oStream << "!END" << endl;

    QStringList planList;
    if (pList !="") {
        planList << pList;
        // planList = pList.split("|",QString::SkipEmptyParts);
     }
     else if (MLCtype != "elektaModuleaf") {
#ifdef PHANTOM_40x40x30
//        planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
        planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#elif PHANTOM_40x40x40
//		  planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
          planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#else
        planList << "2" << "4" << "6" << "8" << "10" << "15" << "20";
#endif
     }
     else {
        // planList << "1.6" << "2.4" << "3.2" << "4.0" << "7.2" << "10.4" << "16";
        planList << "1.6" << "2.4" << "3.2" << "4.0" << "7.2" << "10.4" << "16.0" << "21.0";
     }

    QString couch = "0.0";
    QString gantry = "0.0";
    QString collimator = "0.0";
    QString primary = "e1";
    QString portalMU = "100.0";
    for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
       bool ok;
       QString plan = *it;
       QString FW = plan;
       QString FL = plan;
       if (plan.contains('x')) { // for NOT square field
         FW = plan.section('x',0,0);
         FL = plan.section('x',1,1);
       }

       if (MLCtype == "elektaModuleaf") {
          //std::cout << FW << " x " << FL << " ---> ";
          float FWF = FW.toFloat(&ok);
          float BMW = FWF - (int)(FWF/0.8)*0.8;  // for Beam Modulator
         if (BMW < 1.0E-3 || BMW >= 0.8-1.0E-3) BMW = 0.0;
         //std::cout << "BMW = " << BMW << "  ";
          if (BMW > 0.0 && BMW <= 0.2) FWF = FWF - 0.2;
          if (BMW > 0.2 && BMW <= 0.4) FWF = FWF + 0.4;
          if (BMW > 0.4) FWF = FWF + 0.2;
             if (FWF > 21.0) FWF = 21.0;
          FW.sprintf("%.1f",FWF);

          float FLF = FL.toFloat(&ok);
          float BML = FLF - (int)(FLF/0.8)*0.8;  // for Beam Modulator
          if (BML < 1.0E-3 || BML >= 0.8-1.0E-3) BML = 0.0;
          //std::cout << "BML = " << BML << "  ";
          if (BML > 0.0 && BML <= 0.2) FLF = FLF - 0.2;
          if (BML > 0.2 && BML <= 0.4) FLF = FLF + 0.4;
          if (BML > 0.4) FLF = FLF + 0.2;
             if (FLF > 16.0) FLF = 16.0;
          FL.sprintf("%.1f",FLF);
          //std::cout << FW << " x " << FL << endl;
       }

       oStream << "!BEAMDEF" << endl;
       oStream << "   couch=" << couch << endl;
       oStream << "   gantry=" << gantry << endl;
       oStream << "   collimator=" << collimator << endl;
       oStream << "   primary=" << primary << endl;
       oStream << "   portalMU=" << portalMU << endl;
       float X1 = FW.toFloat(&ok)/2.0 * -10.0;
       float X2 = FW.toFloat(&ok)/2.0 *  10.0;
       float Y1 = FL.toFloat(&ok)/2.0 * -10.0;
       float Y2 = FL.toFloat(&ok)/2.0 *  10.0;
       oStream << "   portalMinPJaw=" << X1 << endl;
       oStream << "   portalMaxPJaw=" << X2 << endl;
       oStream << "   portalMinTJaw=" << Y1 << endl;
       oStream << "   portalMaxTJaw=" << Y2 << endl;
       oStream << "!END" << endl;
    } // foreach planList
    QString fraction = "1";
    oStream << "!FRACTIONS " << fraction << endl;
    oFile.close();
} // End of writeCLN()
// -----------------------------------------------------------------------------
// This function must be merged into writeCLN with option.
// but this is temporary use.
void MainConsole::writePBCLN(QString FNAME, QString pList) {
    QString mName = ui->comboBoxMachine->currentText();
    QString mInfoFile = usr->LHOME + mName + "/" + mName + ".info" ;
    getMachineInfo(mInfoFile);
    QString fname = FNAME+".CLN";
    QFile oFile( fname );
    oFile.remove();
    oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
    QTextStream oStream( &oFile );

    QString maxDoseVariance = "0.02";
    oStream << "!MAXDOSEVARIANCE " << maxDoseVariance << endl;
    oStream << "!DOSE_ENGINES" << endl;
    oStream << "  PB=e1 machinename=" << mName + ".RPB" << endl;
    // oStream << "  MC=e1 machinename=" << mName + ".bdt" << endl;
    oStream << "!END" << endl;
    oStream << "!MOD_EQUIPMENT" << endl;

    QString MLCtype = "";
    if (usr->XIOMLC.contains("ElektaBM80leaf")) MLCtype = "elektaModuleaf";
    if (usr->XIOMLC.contains("MILLENNIUM120")) MLCtype = "varian60";
    if (usr->XIOMLC.contains("PHILIPSMLC")) MLCtype = "elekta40";
    if (usr->XIOMLC.contains("SIEMENSV50")) MLCtype = "siemens27";
    if (usr->XIOMLC.contains("VARIAN26")) MLCtype = "varian52";
    if (usr->XIOMLC.contains("VARIAN40")) MLCtype = "varian40";
    if (usr->XIOMLC.contains("OPTIFOCUS82")) MLCtype = "siemens41";
    /* Monaco Supports these MLC types ONLY
       "staticMLC_Elekta"  "smartMLC"  "elekta40"
       "elektaSplit"  "elektaModuleaf"  "elektaIModuleaf"
       "siemens27" "siemens41" "varian60"  "varian52"
       "varian40";
    */
    QString segmentSize = "10x2"; // Modified on Jan 19, 2007 from 10x10
     if (MLCtype == "varian60") segmentSize = "5x2"; // Added on Jan 19, 2007
     if (MLCtype == "elektaModuleaf") segmentSize = "4x2"; // Added on Jan 19, 2007
    oStream << "   type=" << MLCtype << " size=" << segmentSize << endl;
    oStream << "!END" << endl;

    QStringList planList;
    if (pList !="") {
        planList << pList;
        // planList = pList.split("|",QString::SkipEmptyParts);
     }
     else if (MLCtype != "elektaModuleaf") {
#ifdef PHANTOM_40x40x30
//          planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
          planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#elif PHANTOM_40x40x40
//          planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "25";
          planList << "2" << "4" << "6" << "8" << "10" << "15" << "20" << "26";
#else
          planList << "2" << "4" << "6" << "8" << "10" << "15" << "20";
#endif
     }
     else {
        // planList << "1.6" << "2.4" << "3.2" << "4.0" << "7.2" << "10.4" << "16";
        planList << "1.6" << "2.4" << "3.2" << "4.0" << "7.2" << "10.4" << "16.0" << "21.0";
     }

    QString couch = "0.0";
    QString gantry = "0.0";
    QString collimator = "0.0";
    QString primary = "e1";
    QString portalMU = "100.0";
    for ( QStringList::Iterator it = planList.begin(); it != planList.end(); ++it ) {
       bool ok;
       QString plan = *it;
       QString FW = plan;
       QString FL = plan;
       if (plan.contains('x')) { // for NOT square field
         FW = plan.section('x',0,0);
         FL = plan.section('x',1,1);
       }

       if (MLCtype == "elektaModuleaf") {
             //std::cout << FW << " x " << FL << " ---> ";
          float FWF = FW.toFloat(&ok);
          float BMW = FWF - (int)(FWF/0.8)*0.8;  // for Beam Modulator
             if (BMW < 1.0E-3 || BMW >= 0.8-1.0E-3) BMW = 0.0;
             //std::cout << "BMW = " << BMW << "  ";
          if (BMW > 0.0 && BMW <= 0.2) FWF = FWF - 0.2;
          if (BMW > 0.2 && BMW <= 0.4) FWF = FWF + 0.4;
          if (BMW > 0.4) FWF = FWF + 0.2;
          FW.sprintf("%.1f",FWF);

          float FLF = FL.toFloat(&ok);
          float BML = FLF - (int)(FLF/0.8)*0.8;  // for Beam Modulator
             if (BML < 1.0E-3 || BML >= 0.8-1.0E-3) BML = 0.0;
             //std::cout << "BML = " << BML << "  ";
          if (BML > 0.0 && BML <= 0.2) FLF = FLF - 0.2;
          if (BML > 0.2 && BML <= 0.4) FLF = FLF + 0.4;
          if (BML > 0.4) FLF = FLF + 0.2;
          FL.sprintf("%.1f",FLF);
             //std::cout << FW << " x " << FL << endl;
       }

       oStream << "!BEAMDEF" << endl;
       oStream << "   couch=" << couch << endl;
       oStream << "   gantry=" << gantry << endl;
       oStream << "   collimator=" << collimator << endl;
       oStream << "   primary=" << primary << endl;
       oStream << "   portalMU=" << portalMU << endl;
       float X1 = FW.toFloat(&ok)/2.0 * -10.0;
       float X2 = FW.toFloat(&ok)/2.0 *  10.0;
       float Y1 = FL.toFloat(&ok)/2.0 * -10.0;
       float Y2 = FL.toFloat(&ok)/2.0 *  10.0;
       oStream << "   portalMinPJaw=" << X1 << endl;
       oStream << "   portalMaxPJaw=" << X2 << endl;
       oStream << "   portalMinTJaw=" << Y1 << endl;
       oStream << "   portalMaxTJaw=" << Y2 << endl;
       oStream << "!END" << endl;
    } // foreach planList
    QString fraction = "1";
    oStream << "!FRACTIONS " << fraction << endl;
    oFile.close();
} // End of writeCLN()
// -----------------------------------------------------------------------------
void MainConsole::writeVER(QString FNAME, QStringList xZLIST, QStringList yZLIST, QString OPTIONS) {
    // OPTIONS should be a string including "keyword = value;"
    // OPTIONS = "MCvariance = 0.01; voxelSize = 0.2; nX = 150; nY = 150; nZ = 150; xCenter = 15.0;
    //            yCenter = 15.0; zCenter = 0.0; density = 1.0; xDepth = 15.0; yDepth = 15.0; zDepth = 15.0"
     // std::cout << "OPTIONS=" << OPTIONS << endl;
    bool ok;

    QString fname = FNAME+".VER";
    QFile oFile( fname );
    oFile.remove();
    oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
    QTextStream oStream( &oFile );

    QString MCvariance = "0.005";
    QString voxelSize = "0.2"; // cm
    QString nX = "150";
    QString nY = "150";
    QString nZ = "150";
    QString xCenter = "15.0"; // cm
    QString yCenter = "15.0"; // cm
    QString zCenter = "10.0";  // cm  for SSD=900 mm
    QString density = "1.0";

    QString xDepth = "15.0";  // For Plane Dose
    QString yDepth = "15.0";  // For Plane Dose
    QString zDepth = "15.0";  // For Plane Dose
#ifdef PHANTOM_40x40x40
     nX = "200";
     nY = "200";
     nZ = "200";
     xCenter = "20.0";
     yCenter = "20.0";
     xDepth = "20.0";
     yDepth = "20.0";
#endif
#ifdef PHANTOM_40x40x30
     nX = "200";
     nY = "200";
     nZ = "150";
     xCenter = "20.0";
     yCenter = "20.0";
     xDepth = "20.0";
     yDepth = "20.0";
#endif

     QString voxelSmall = ui->comboBoxGridSizeSmall->currentText();
     QString voxelLarge = ui->comboBoxGridSizeLarge->currentText();

    QString Mode = "PBComm"; // MODE = PBComm/Mono/Verify/PB_/PBMC_

    if (OPTIONS.length() > 0) {
     if (OPTIONS.toLower().contains("mcvariance"))
     MCvariance = OPTIONS.toLower().section("mcvariance",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("voxelsize"))
     voxelSize = OPTIONS.toLower().section("voxelsize",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("nx"))
     nX = OPTIONS.toLower().section("nx",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("ny"))
     nY = OPTIONS.toLower().section("ny",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("nz"))
     nZ = OPTIONS.toLower().section("nz",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("xcenter"))
     xCenter = OPTIONS.toLower().section("xcenter",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("ycenter"))
     yCenter = OPTIONS.toLower().section("ycenter",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("zcenter"))
     zCenter = OPTIONS.toLower().section("zcenter",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("density"))
     density = OPTIONS.toLower().section("density",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("xdepth"))
     xDepth = OPTIONS.toLower().section("xdepth",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("ydepth"))
     yDepth = OPTIONS.toLower().section("ydepth",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("zdepth"))
     zDepth = OPTIONS.toLower().section("zdepth",1,1).section("=",1,1).section(";",0,0);
     if (OPTIONS.toLower().contains("mode"))
     Mode = OPTIONS.toLower().section("mode",1,1).section("=",1,1).section(";",0,0);
    }

    oStream << "!MCVARIANCE " << MCvariance << endl;
    if (Mode.contains("pb_") || Mode.contains("pbmc_")) oStream << "#  ";
    oStream << "!NODOSE      0.001" << endl;
    // oStream << "!E-CUTOFF    0.25" << endl;
    // oStream << "!HISTPERGYSQMM 999" << endl; // Removed  by U Jelen's instruction
    oStream << "!WATERPHANTOM" << endl;

    // int gridSize = (int) (voxelSize.toFloat(&ok)*10);
    float gridSize = voxelSize.toFloat(&ok)*10;
    oStream << "        dosegridsize=" << gridSize << endl;
    oStream << "        dimension=" << nX << ", "
                                    << nZ << ", "
                                    << nY << endl;
    //int xc = (int) (xCenter.toFloat(&ok)*10);
    //int yc = (int) (yCenter.toFloat(&ok)*10);
    //int zc = (int) (zCenter.toFloat(&ok)*10);
    float xc = xCenter.toFloat(&ok)*10;
    float yc = yCenter.toFloat(&ok)*10;
    float zc = zCenter.toFloat(&ok)*10;
    oStream << "        isocentre=" << xc << ", "
                                    << zc << ", "
                                    << yc << endl;
    oStream << "        density="   << density << endl;
    oStream << "!END" << endl;
    oStream << "!SLAB" << endl;
    oStream << "        density="   << density << endl;
    oStream << "        xlimits= 0,"   << nX << endl;
    oStream << "        ylimits= 0,"   << nZ << endl;
    oStream << "        zlimits= 0,"   << nY << endl;
    oStream << "!END" << endl;
    oStream << "!SINGLEBEAM" << endl;

    //int x = (int) (xDepth.toFloat(&ok)*10);
    //int y = (int) (yDepth.toFloat(&ok)*10);
    //int z = (int) (zDepth.toFloat(&ok)*10);
    float x = xDepth.toFloat(&ok)*10;
    float y = yDepth.toFloat(&ok)*10;
    float z = zDepth.toFloat(&ok)*10;


    if (xZLIST.isEmpty() && yZLIST.isEmpty() ){
      QStringList zList;
        if (Mode == "PBComm" || Mode.contains("pbmc_")) {
     // For PBComm
     zList << "0.2" << "0.5" << "1.0" << "2.0" << "3.0" << "4.0" << "5.0" << "6.0"
     << "6.5" << "7.0" << "7.5" << "8.0" << "8.5" << "9.0" << "9.5" << "10.0"
     << "10.5" << "11.0" << "11.5" << "12.0" << "12.5" << "13.0" << "13.5" << "14.0"
     << "14.5" << "15.0" << "15.5" << "16.0" << "16.5" << "17.0" << "17.5" << "18.0"
     << "18.5" << "19.0" << "19.5" << "20.0" << "20.5" << "21.0" << "21.5" << "22.0"
     << "22.5" << "23.0" << "23.5" << "24.0" << "24.5" << "25.0" << "25.5" << "26.0"
     << "27.0" << "28.0" << "29.0"
     #ifdef PHANTOM_40x40x40
         << "30.0" << "31.0" << "32.0" << "33.0" << "34.0"
     << "35.0" << "36.0" << "37.0" << "38.0" << "39.0"
     #endif
     ;  // Do not remove this line

     oStream << "        profile y : " << x << "," << y << endl;
     for ( QStringList::Iterator it = zList.begin(); it != zList.end(); ++it ) {
         QString depth = *it;
         // int z = (int) (depth.toFloat(&ok)*10);
         float  z = depth.toFloat(&ok)*10;
         oStream << "        profile z : " << y << "," << z << endl;
     }
     for ( QStringList::Iterator it = zList.begin(); it != zList.end(); ++it ) {
         QString depth = *it;
         // int z = (int) (depth.toFloat(&ok)*10);
         float  z = depth.toFloat(&ok)*10;
         oStream << "        profile x : " << z << "," << x << endl;
     }
     oStream << "        slice x : " << x << endl;
     oStream << "        slice y : " << z << endl;
     oStream << "        slice z : " << y << endl;
     }
     else {
         // For MonoVerify
        if (ui->comboBoxVoxels->currentText() == "1") {
         oStream << "        profile y : " << x << "," << y << endl;
         }
         else {
         float halfGrid = 0.5*gridSize;
         oStream << "        profile y : " << x-halfGrid << "," << y-halfGrid << endl;
         oStream << "        profile y : " << x+halfGrid << "," << y-halfGrid << endl;
         oStream << "        profile y : " << x+halfGrid << "," << y+halfGrid << endl;
         oStream << "        profile y : " << x-halfGrid << "," << y+halfGrid << endl;
             if (ui->comboBoxVoxels->currentText() == "16") {
             oStream << "        profile y : " << x-halfGrid*3 << "," << y-halfGrid*3 << endl;
             oStream << "        profile y : " << x+halfGrid*3 << "," << y-halfGrid*3 << endl;
             oStream << "        profile y : " << x+halfGrid*3 << "," << y+halfGrid*3 << endl;
             oStream << "        profile y : " << x-halfGrid*3 << "," << y+halfGrid*3 << endl;

             oStream << "        profile y : " << x-halfGrid*3 << "," << y-halfGrid*1 << endl;
             oStream << "        profile y : " << x+halfGrid*3 << "," << y-halfGrid*1 << endl;
             oStream << "        profile y : " << x+halfGrid*3 << "," << y+halfGrid*1 << endl;
             oStream << "        profile y : " << x-halfGrid*3 << "," << y+halfGrid*1 << endl;

             oStream << "        profile y : " << x-halfGrid*1 << "," << y-halfGrid*3 << endl;
             oStream << "        profile y : " << x+halfGrid*1 << "," << y-halfGrid*3 << endl;
             oStream << "        profile y : " << x+halfGrid*1 << "," << y+halfGrid*3 << endl;
             oStream << "        profile y : " << x-halfGrid*1 << "," << y+halfGrid*3 << endl;
             }
         }
      }
    } else {
      oStream << "        profile y : " << x << "," << y << endl;
      QStringList xZList = xZLIST;
      for ( QStringList::Iterator it = xZList.begin(); it != xZList.end(); ++it ) {
         QString depth = *it;
         if (!depth.contains("None")){
         // int z = (int) (depth.toFloat(&ok)*10);
         float  z = depth.toFloat(&ok)*10;
            oStream << "        profile x : " << z << "," << x << endl;
         }
      }
      QStringList yZList = yZLIST;
      for ( QStringList::Iterator it = yZList.begin(); it != yZList.end(); ++it ) {
         QString depth = *it;
         if (!depth.contains("None")){
         // int z = (int) (depth.toFloat(&ok)*10);
         float  z = depth.toFloat(&ok)*10;
            oStream << "        profile z : " << y << "," << z << endl;
         }
      }
    }

    oStream << "!END" << endl;

    oFile.close();

}
// -----------------------------------------------------------------------------
void MainConsole::readSMOOTH(QString FNAME) {
    QString u1s = "0.04";
    QString u2s = "0.5";
    QString w1s = "0.008";
    QString u1d = "1.0";
    QString u2d = "1.0";
    QString w1d = "0.2";

    QFile mFile(FNAME);
    if (mFile.exists()) {
       QTextStream stream( &mFile );
       QString sLine;
       mFile.open( QIODevice::ReadOnly);
       int iPara = 0;
       while ( !stream.atEnd() && iPara < 3) {
          sLine = stream.readLine();
          QString strLine = sLine.simplified();
          if (strLine.left(1) != "#") {
             if (iPara == 0) {
                u2s = strLine.section(' ', 0,0);
                u2d = strLine.section(' ', 1,1);
                iPara++;
                continue;
             }
             if (iPara == 1) {
                u1s = strLine.section(' ', 0,0);
                u1d = strLine.section(' ', 1,1);
                iPara++;
                continue;
             }
             if (iPara == 2) {
                w1s = strLine.section(' ', 0,0);
                w1d = strLine.section(' ', 1,1);
                iPara++;
                continue;
             }
          }
       }
       mFile.close();
    }
    ui->lineEditU1s->setText(u1s);
    ui->lineEditU2s->setText(u2s);
    ui->lineEditWs->setText(w1s);
    ui->lineEditU1d->setText(u1d);
    ui->lineEditU2d->setText(u2d);
    ui->lineEditWd->setText(w1d);
}
// -----------------------------------------------------------------------------
void MainConsole::writeSMOOTH(QString FNAME) {

    QString fname = FNAME;
    QFile oFile(fname);
    oFile.remove();
    oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
    QTextStream oStream( &oFile );

    QString u1s = ui->lineEditU1s->text();
    QString u2s = ui->lineEditU2s->text();
    QString w1s = ui->lineEditWs->text();
    QString u1d = ui->lineEditU1d->text();
    QString u2d = ui->lineEditU2d->text();
    QString w1d = ui->lineEditWd->text();


    oStream << u2s << "  " << u2d << endl;
    oStream << u1s << "  " << u1d << endl;
    oStream << w1s << "  " << w1d << endl;

    oFile.close();

}
// -----------------------------------------------------------------------------
void MainConsole::readFIT(QString FNAME) {
    QString w11 = "2";
    QString w21 = "0.99";
    QString u11 = "0.8";
    QString u21 = "0.25";
    QString A1 = "0.03";

    QString w12 = "0.99";
    QString w22 = "0.8";
    QString u12 = "0.25";
    QString u22 = "0.03";
    QString A2 = "-1";

    QString w1 = "0.6";
    QString w2 = "0.4";
    QString u1 = "0.8";
    QString u2 = "0.1";
    QString A = "0.3";

    int iPara = 0;
    QFile mFile(FNAME);
    if (mFile.exists()) {
       QTextStream stream(&mFile);
       QString sLine;
       mFile.open( QIODevice::ReadOnly );
       while (!stream.atEnd() && iPara < 5) {
          sLine = stream.readLine();
          QString strLine = sLine.simplified();
          if (strLine.left(1) != "#") {
             if (iPara == 0) {
                w11 = strLine.section(' ', 0,0);
                w12 = strLine.section(' ', 1,1);
                w1  = strLine.section(' ', 2,2);
                iPara++;
                continue;
             }
             if (iPara == 1) {
                w21 = strLine.section(' ', 0,0);
                w22 = strLine.section(' ', 1,1);
                w2  = strLine.section(' ', 2,2);
                iPara++;
                continue;
             }
             if (iPara == 2) {
                u11 = strLine.section(' ', 0,0);
                u12 = strLine.section(' ', 1,1);
                u1  = strLine.section(' ', 2,2);
                iPara++;
                continue;
             }
             if (iPara == 3) {
                u21 = strLine.section(' ', 0,0);
                u22 = strLine.section(' ', 1,1);
                u2  = strLine.section(' ', 2,2);
                iPara++;
                continue;
             }
             if (iPara == 4) {
                A1 = strLine.section(' ', 0,0);
                A2 = strLine.section(' ', 1,1);
                A  = strLine.section(' ', 2,2);
                iPara++;
                continue;
             }
          }
       }
       mFile.close();
    }

    ui->lineEditW11->setText(w11);
    ui->lineEditW12->setText(w12);
    ui->lineEditW1->setText(w1);

    ui->lineEditW21->setText(w21);
    ui->lineEditW22->setText(w22);
    ui->lineEditW2->setText(w2);

    ui->lineEditU11->setText(u11);
    ui->lineEditU12->setText(u12);
    ui->lineEditU1->setText(u1);

    ui->lineEditU21->setText(u21);
    ui->lineEditU22->setText(u22);
    ui->lineEditU2->setText(u2);

    ui->lineEditA1->setText(A1);
    ui->lineEditA2->setText(A2);
    ui->lineEditA->setText(A);
}
// -----------------------------------------------------------------------------
void MainConsole::writeFIT(QString FNAME) {

    QString fname = FNAME;
    QFile oFile(fname);
    oFile.remove();
    oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite);
    QTextStream oStream( &oFile );

    QString w11 = ui->lineEditW11->text();
    QString w21 = ui->lineEditW21->text();
    QString u11 = ui->lineEditU11->text();
    QString u21 = ui->lineEditU21->text();
    QString A1 = ui->lineEditA1->text();

    QString w12 = ui->lineEditW12->text();
    QString w22 = ui->lineEditW22->text();
    QString u12 = ui->lineEditU12->text();
    QString u22 = ui->lineEditU22->text();
    QString A2 = ui->lineEditA2->text();

    QString w1 = ui->lineEditW1->text();
    QString w2 = ui->lineEditW2->text();
    QString u1 = ui->lineEditU1->text();
    QString u2 = ui->lineEditU2->text();
    QString A = ui->lineEditA->text();

    oStream << w11 << "  " << w12 << "  " << w1 << endl;
    oStream << w21 << "  " << w22 << "  " << w2 << endl;
    oStream << u11 << "  " << u12 << "  " << u1 << endl;
    oStream << u21 << "  " << u22 << "  " << u2 << endl;
    oStream << A1 << "  " << A2 << "  " << A << endl;

    oFile.close();

}
// -----------------------------------------------------------------------------
void MainConsole::resetSmooth(){
   // Current Machine Name
   QString mName = ui->comboBoxMachine->currentText();
   // Current Machine Directory
   QString mDir = usr->LHOME+mName;

   QString SMTH_FILE = mDir + "/smooth.inp";
   if (isThereFile(mDir, "smooth.inp")) readSMOOTH(SMTH_FILE);
}
// -----------------------------------------------------------------------------
void MainConsole::resetFit(){
   // Current Machine Name
   QString mName = ui->comboBoxMachine->currentText();
   // Current Machine Directory
   QString mDir = usr->LHOME+mName;

   QString FIT_FILE = mDir + "/fit.inp";
   if (isThereFile(mDir, "fit.inp")) readFIT(FIT_FILE);
}
// -----------------------------------------------------------------------------
void MainConsole::initSmooth(){
    QString u1s = "0.04";
    QString u2s = "0.5";
    QString w1s = "0.008";
    QString u1d = "1.0";
    QString u2d = "1.0";
    QString w1d = "0.2";

    QString w11 = "2";
    QString w21 = "0.99";
    QString u11 = "0.8";
    QString u21 = "0.25";
    QString A1 = "0.03";

    QString w12 = "0.99";
    QString w22 = "0.8";
    QString u12 = "0.25";
    QString u22 = "0.03";
    QString A2 = "-1";

    QString w1 = "0.6";
    QString w2 = "0.4";
    QString u1 = "0.8";
    QString u2 = "0.1";
    QString A = "0.3";

    ui->lineEditU1s->setText(u1s);
    ui->lineEditU2s->setText(u2s);
    ui->lineEditWs->setText(w1s);
    ui->lineEditU1d->setText(u1d);
    ui->lineEditU2d->setText(u2d);
    ui->lineEditWd->setText(w1d);


    ui->lineEditW11->setText(w11);
    ui->lineEditW12->setText(w12);
    ui->lineEditW1->setText(w1);

    ui->lineEditW21->setText(w21);
    ui->lineEditW22->setText(w22);
    ui->lineEditW2->setText(w2);

    ui->lineEditU11->setText(u11);
    ui->lineEditU12->setText(u12);
    ui->lineEditU1->setText(u1);

    ui->lineEditU21->setText(u21);
    ui->lineEditU22->setText(u22);
    ui->lineEditU2->setText(u2);

    ui->lineEditA1->setText(A1);
    ui->lineEditA2->setText(A2);
    ui->lineEditA->setText(A);
}
// -----------------------------------------------------------------------------
void MainConsole::initFit(){
    QString w11 = "2";
    QString w21 = "0.99";
    QString u11 = "0.8";
    QString u21 = "0.25";
    QString A1 = "0.03";

    QString w12 = "0.99";
    QString w22 = "0.8";
    QString u12 = "0.25";
    QString u22 = "0.03";
    QString A2 = "-1";

    QString w1 = "0.6";
    QString w2 = "0.4";
    QString u1 = "0.8";
    QString u2 = "0.1";
    QString A = "0.3";

    ui->lineEditW11->setText(w11);
    ui->lineEditW12->setText(w12);
    ui->lineEditW1->setText(w1);

    ui->lineEditW21->setText(w21);
    ui->lineEditW22->setText(w22);
    ui->lineEditW2->setText(w2);

    ui->lineEditU11->setText(u11);
    ui->lineEditU12->setText(u12);
    ui->lineEditU1->setText(u1);

    ui->lineEditU21->setText(u21);
    ui->lineEditU22->setText(u22);
    ui->lineEditU2->setText(u2);

    ui->lineEditA1->setText(A1);
    ui->lineEditA2->setText(A2);
    ui->lineEditA->setText(A);
}
// -----------------------------------------------------------------------------
void MainConsole::writeRSD(){
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName,"RSD", ui->floatSpinBoxRSD->text());
}
// -----------------------------------------------------------------------------
void MainConsole::readRSD(){
   QString mName = ui->comboBoxMachine->currentText();
   QString RSD = readLocalSetting(mName,"RSD");
   bool ok;
   ui->floatSpinBoxRSD->setValue(RSD.toFloat(&ok));
}
// -----------------------------------------------------------------------------
void MainConsole::writeNU(){
   QString mName = ui->comboBoxMachine->currentText();
    QString NUVALUE = ui->lineEditNUval->text();
    bool ok;
    if (NUVALUE.toFloat(&ok) < 0.45) NUVALUE = "0.45";
   writeLocalSetting(mName,"NUVALUE", NUVALUE);
}
// -----------------------------------------------------------------------------
void MainConsole::readNU(){
   QString mName = ui->comboBoxMachine->currentText();
   QString NUVALUE = readLocalSetting(mName,"NUVALUE");
   bool ok;
   if (NUVALUE.toFloat(&ok) < 0.45) NUVALUE = "0.45";
   ui->lineEditNUval->setText(NUVALUE);
}
void MainConsole::writeMFS(){
   QString mName = ui->comboBoxMachine->currentText();
   if (ui->checkBoxFS->isChecked())
      writeLocalSetting(mName,"RealFS", "ON");
   else
      writeLocalSetting(mName,"RealFS", "OFF");
}
// -----------------------------------------------------------------------------
void MainConsole::readMFS(){
   QString mName = ui->comboBoxMachine->currentText();
   QString SW = readLocalSetting(mName,"RealFS");
   if (SW == "ON") ui->checkBoxFS->setChecked(true);
   if (SW == "OFF") ui->checkBoxFS->setChecked(false);
}
// -----------------------------------------------------------------------------
void MainConsole::writeOffset(){
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName,"Offset", ui->floatSpinBoxOffset->text());
}
// -----------------------------------------------------------------------------
void MainConsole::readOffset(){
   QString mName = ui->comboBoxMachine->currentText();
   QString Offset = readLocalSetting(mName,"Offset");
   bool ok;
   ui->floatSpinBoxOffset->setValue(Offset.toFloat(&ok));
}
// -----------------------------------------------------------------------------
void MainConsole::writeOFAdjust(){
   QString mName = ui->comboBoxMachine->currentText();
   if (ui->checkBoxOFAdjust->isChecked())
      writeLocalSetting(mName,"OFAdjust", "ON|"+ui->lineEditOFAdjust->text());
   else
      writeLocalSetting(mName,"OFAdjust", "OFF|"+ui->lineEditOFAdjust->text());
}
// -----------------------------------------------------------------------------
void MainConsole::readOFAdjust(){
   QString mName = ui->comboBoxMachine->currentText();
   QString qsOFAdjust = readLocalSetting(mName,"OFAdjust");
    ui->checkBoxOFAdjust->setChecked(false);
    if(qsOFAdjust.section("|",0,0) == "ON") ui->checkBoxOFAdjust->setChecked(true);
   ui->lineEditOFAdjust->setText(qsOFAdjust.section("|",1,1));
   ui->pushButtonAdjustNorm->setEnabled(true);
}
// -----------------------------------------------------------------------------
void MainConsole::writeAirOpt(){
    bool ok;
 QString mName = ui->comboBoxMachine->currentText();
 int iHigh=0;
 int iAvg = 0;
 int iLow = 0;
 int iAsIs = 0;
 QString STD = ui->lineEditSTDev->text();
 int eOnly = 0;
 int eCF = 0;
 int iGridSize = ui->comboBoxGridSize->currentData().toInt(&ok);
 int iVoxels = ui->comboBoxVoxels->currentData().toInt(&ok);
 if (ui->checkBoxHigh->isChecked()) iHigh = 1;
 if (ui->checkBoxAvg->isChecked()) iAvg = 1;
 if (ui->checkBoxLow->isChecked()) iLow = 1;
 if (ui->checkBoxAsIs->isChecked()) iAsIs = 1;
 if (ui->checkBoxElectronOnly->isChecked()) eOnly = 1;
 if (ui->checkBoxElectronCorrect->isChecked()) eCF = 1;
 QString AIROPT="";
 AIROPT.sprintf("%d|%d|%d|%d|%d|%d|%d|%d",iHigh,iAvg,iLow,iAsIs,eOnly,eCF,iGridSize,iVoxels);
 AIROPT = AIROPT + "|" + STD;
 writeLocalSetting(mName,"AIROPT",AIROPT);
}
// -----------------------------------------------------------------------------
void MainConsole::readAirOpt() {
 QString mName = ui->comboBoxMachine->currentText();
 QString AIROPT=readLocalSetting(mName,"AIROPT");
 if (AIROPT != "") {
  bool ok;
  int iHigh= AIROPT.section("|",0,0).toInt(&ok,10);
  int iAvg = AIROPT.section("|",1,1).toInt(&ok,10);
  int iLow = AIROPT.section("|",2,2).toInt(&ok,10);
  int iAsIs = AIROPT.section("|",3,3).toInt(&ok,10);
  int eOnly = AIROPT.section("|",4,4).toInt(&ok,10);
  int eCF = AIROPT.section("|",5,5).toInt(&ok,10);
  int iGridSize = AIROPT.section("|",6,6).toInt(&ok,10);
  int iVoxels = AIROPT.section("|",7,7).toInt(&ok,10);
  QString STD = AIROPT.section("|",8,8);
  if (iHigh == 1) ui->checkBoxHigh->setChecked(true);
     if (iAvg == 1) ui->checkBoxAvg->setChecked(true);
     if (iLow == 1) ui->checkBoxLow->setChecked(true);
     if (iAsIs == 1) ui->checkBoxAsIs->setChecked(true);
     ui->lineEditSTDev->setText(STD);
     if (eOnly == 1) ui->checkBoxElectronOnly->setChecked(true);
     else ui->checkBoxElectronOnly->setChecked(false);
     if (eCF == 1) ui->checkBoxElectronCorrect->setChecked(true);
     else ui->checkBoxElectronCorrect->setChecked(false);
    ui->comboBoxGridSize->setCurrentIndex(iGridSize);
    ui->comboBoxVoxels->setCurrentIndex(iVoxels);
 }
}
// -----------------------------------------------------------------------------
void MainConsole::writeWaterOpt(){
   QString mName = ui->comboBoxMachine->currentText();
   QString qMBSF="0";
   if (ui->checkBoxMBSF->isChecked()) qMBSF = "1";

   writeLocalSetting(mName,"WATEROPT",qMBSF);
}
// -----------------------------------------------------------------------------
void MainConsole::readWaterOpt() {
 QString mName = ui->comboBoxMachine->currentText();
 QString WATEROPT=readLocalSetting(mName,"WATEROPT");
 if (WATEROPT != "") {
  bool ok;
  int iMBSF= WATEROPT.toInt(&ok,10);
  if (iMBSF == 1) ui->checkBoxMBSF->setChecked(true);
  else ui->checkBoxMBSF->setChecked(false);
 }
}
// -----------------------------------------------------------------------------
void MainConsole::writeOffAxisOpt(){
   QString mName = ui->comboBoxMachine->currentText();
   QString qOffAxis="0";
   if (ui->checkBoxOffAxis->isChecked()) qOffAxis = "1";

   writeLocalSetting(mName,"OffAxis",qOffAxis);
}
// -----------------------------------------------------------------------------
void MainConsole::readOffAxisOpt() {
 QString mName = ui->comboBoxMachine->currentText();
 QString OffAxisOPT=readLocalSetting(mName,"OffAxis");
 if (OffAxisOPT != "") {
  bool ok;
  int iOffAxis= OffAxisOPT.toInt(&ok,10);
  if (iOffAxis == 1) ui->checkBoxOffAxis->setChecked(true);
  else ui->checkBoxOffAxis->setChecked(false);
 }
}
// -----------------------------------------------------------------------------
void MainConsole::mySystem(QString CMD) {
   int iAns = 2;
   int sysAns = system(CMD.toLatin1());
   if ( sysAns != 0 && sysAns != 256) {
      // QTextStream (stdout) << "sysAnd = " << sysAns << endl;
      iAns = QMessageBox::critical(this,
             "ERROR: Fail to run system command",
             CMD,
             QMessageBox::Abort,
             QMessageBox::Retry,
             QMessageBox::Ignore);
   }
   switch (iAns) {
      case 0: exit(-1); break;
      case 1: mySystem(CMD); break;
      case 2: break;
      default: break;
   }
}
/*
// -----------------------------------------------------------------------------
// Work in Progress for displaying console output
void MainConsole::myProcess(QString WDIR, QString CMD, QString LOGFILE) {
   //int currentPage = tabWidget->currentIndex();
    QDir *PWD = new QDir;
    QString CWD = PWD->currentDirPath();
    PWD->setCurrent(WDIR);

   tabWidget->setCurrentPage(7);
   QStringList CMDList = QStringList::split(" ",CMD);
    std::cout << "WDIR = " << WDIR << endl;
    std::cout << "CMD = " << CMD << endl;
    std::cout << "LOGFILE = " << LOGFILE << endl;
    proc = new QProcess( this );
   proc->setArguments( CMDList );
    // proc->addArgument( "ls" );
    // proc->addArgument( "-l" );

   logFile = new QFile( WDIR+"/"+LOGFILE );
   if (!logFile->open( QIODevice::ReadOnly ) ){
       perror("Error was ");
       qFatal("Error creating log "+LOGFILE+" file ");
       QMessageBox::critical(this,"Error creating log "+LOGFILE+" file ",
                  "Check whether you have write permissions "
                  "in : \n" + QDir::currentDirPath () +
                  "\n and start again!",
                  "&OK",
                  0,0,0,-1);
   }
   logStream.setDevice ( logFile );

   connect( proc, SIGNAL(readyReadStdout()),this, SLOT(myDumpStdOut()) );

    std::cout << "Before ---" << endl;
   if ( !proc->start() ) {
        // error handling
        QMessageBox::critical( 0,
                tr("Fatal error"),
                tr("Could not start the command."),
                tr("Quit") );
        exit( -1 );
   }
    connect( proc, SIGNAL(processExited()),this, SLOT(PWD->setCurrent(CWD)));

    std::cout << "After ---" << endl;
}
// -----------------------------------------------------------------------------
void MainConsole::myDumpStdOut(){
    // Read and process the data.
    // Bear in mind that the data might be output in chunks.

    textEditMonitor->append( proc->readStdout() );
     QString message = proc->readLineStdout();
     LOGSTREAM << message.toLatin1();
     LOGFILE->flush();
}
*/
// -----------------------------------------------------------------------------
void MainConsole::mergePDDs(QString DIR, QString PLAN, int I) {
     vector<vector<double> > vPos; // Scan Point Position
     vector<vector<double> > vVal; // Scan Value
     vector<double> xPos;
     vector<double> yPos;
     vector<double> zPos;

     for (int i=0; i<I; i++) {
          stringstream iStrStream;
          iStrStream << DIR.toStdString() + "/" << PLAN.toStdString() << "_BEAM1_PROFILE" << i+1 << ".txt";
          string FileName = iStrStream.str();
          // std::cout << FileName << endl;
          fstream fin;
          fin.open(FileName.c_str(), std::ios::in);
          if (!fin.is_open()) {
            cerr << "ERROR: Failed to open " << FileName << endl;
            exit(1);
          }

          string sToken("");
          int iToken = 0;
          // X Pos
          fin >> sToken; iToken++;
          if (sToken != "#") {
            cerr << "ERROR: sToken (" << sToken << ") is not # (" << iToken << ")" << endl;
            exit(1);
          }

          fin >> sToken;
          if (sToken != "X:") {
            cerr << "ERROR: sToken (" << sToken << ") is not X: (" << iToken << ")" << endl;
            exit(1);
          }

          fin >> sToken;
          xPos.push_back(atof(sToken.c_str()));

          // Y Pos
          fin >> sToken; iToken++;
          if (sToken != "#") {
            cerr << "ERROR: sToken (" << sToken << ") is not # (" << iToken << ")" << endl;
            exit(1);
          }

          fin >> sToken;
          if (sToken != "Y:") {
            cerr << "ERROR: sToken (" << sToken << ") is not Y: (" << iToken << ")" << endl;
            exit(1);
          }

          fin >> sToken;
          yPos.push_back(atof(sToken.c_str()));

          // Z Pos
          fin >> sToken; iToken++;
          if (sToken != "#") {
            cerr << "ERROR: sToken (" << sToken << ") is not # (" << iToken << ")" << endl;
            exit(1);
          }

          fin >> sToken;
          if (sToken != "Z") {
            cerr << "ERROR: sToken (" << sToken << ") is not Z (" << iToken << ")" << endl;
            exit(1);
          }

          vPos.push_back(vector<double>());
          vVal.push_back(vector<double>());
          while (!fin.eof()){
            fin >> sToken; iToken++;
            vPos[i].push_back(atof(sToken.c_str()));
            fin >> sToken;
            vVal[i].push_back(atof(sToken.c_str()));
          }
          fin.close();
     } // for (int i=0; i<I; i++)

     double xSum = 0.0;
     double ySum = 0.0;
     for (int i=0; i<I; i++) {
          xSum += xPos[i];
          ySum += yPos[i];
     }
     stringstream oStrStream;
     oStrStream << DIR.toStdString() + "/" << PLAN.toUpper().toStdString() << "_BEAM1_PROFILE1.txt";
     string oFileName = oStrStream.str();
     fstream fout;
     fout.open(oFileName.c_str(), std::ios::out);
     if (!fout.is_open()){
          cerr << "ERROR: Failed to open " << oFileName << endl;
          exit(1);
     }

     fout << "#  X:     " << xSum/(I*1.0) << endl;
     fout << "#  Y:     " << ySum/(I*1.0) << endl;
     fout << "#  Z      " << endl;
     int nData = vPos[0].size() - 1;
     for (int i=0; i<nData; i++) {
          fout << vPos[0][i];
          double sum = 0.0;
          for (int k=0; k<I; k++) {
            sum += vVal[k][i];
          }
          fout << "  " << sum/(I*1.0) << endl;
     }
     fout.close();
}
// -----------------------------------------------------------------------------
void MainConsole::updateMachineInfo() {
   QString mName = ui->comboBoxMachine->currentText();
   setMDIR(ui->lineEditTeleDir->text());
   setLHOME(usr->LHOME);
   setTMPDIR(usr->LHOME + mName);
   setMODEL(mName);
   setModelFile(mName + ".info");
   getMachineInfo();
   updateAll();
   writeLocalSetting(mName,"AVALUE",usr->AVAL);
   writeLocalSetting(mName,"ZVALUE",usr->ZVAL);
}
// -----------------------------------------------------------------------------
QString MainConsole::readVER(QString FNAME, QString OPT) {
 QString mDir = FNAME;

 QString doseGridSize;
 QString density;
 QString defaultValue;
 QString MCVariance;
 QString NoDose;
 QString ECutOff;
 QString algorithm;
 QString machine;
 QString MLCtype;
 QString HistPerGysqmm;
 QString maxDoseVariance;
 QString PBmachine;
 QString MCmachine;
 QString segmentSize;
 QString fraction;
 QString dimensionX;
 QString dimensionY;
 QString dimensionZ;
 QString isocenterX;
 QString isocenterY;
 QString isocenterZ;
 QString dummyX;
 QString dummyY;
 QString dummyZ;
 int nProfiles;
 int nProfileX;
 int nProfileY;
 int nProfileZ;

 QFile mFile( mDir );
 if (mFile.exists()) {
    QTextStream stream( &mFile );
    QString sLine;
    mFile.open( QIODevice::ReadOnly );

      while ( !stream.atEnd() ) {
        sLine = stream.readLine();
        QString strLine = sLine.toLatin1();
        sLine = strLine.simplified();

        if (sLine.left(1) != "#") {
          if (sLine.contains("!MCVARIANCE")) {
             MCVariance = sLine.section(' ',1,1);
          }
          if (sLine.contains("!NODOSE")) {
             NoDose = sLine.section(' ',1,1);
          }
          if (sLine.contains("!E-CUTOFF")) {
             ECutOff = sLine.section(' ',1,1);
         }
          if (sLine.contains("!HISTPERGYSQMM")) {
             HistPerGysqmm = sLine.section(' ',1,1);
          }
          if (sLine.contains("!WATERPHANTOM")) {
             while(!sLine.contains("!END")){
               QString rLine = stream.readLine();
               QString lLine = rLine.toLatin1();
               sLine = lLine.simplified();
               if (sLine.left(1) != "#") {
                 if (sLine.contains("default")) {
                   defaultValue = "default";
                 }
                 if (sLine.contains("dosegridsize=")) {
                   doseGridSize = sLine.section('=',1,1);
                 }
                 if (sLine.contains("density=")) {
                   density = sLine.section('=',1,1);
                 }
                 if (sLine.contains("dimension=")) {
                   dimensionX =
                     sLine.section('=',1,1).section(',',0,0).simplified();
                   dimensionZ =
                     sLine.section('=',1,1).section(',',1,1).simplified();
                   dimensionY =
                     sLine.section('=',1,1).section(',',2,2).simplified();
                 }
                 if (sLine.contains("isocentre=")) {
                   isocenterX =
                     sLine.section('=',1,1).section(',',0,0).simplified();
                   isocenterZ =
                     sLine.section('=',1,1).section(',',1,1).simplified();
                   isocenterY =
                     sLine.section('=',1,1).section(',',2,2).simplified();
                 }
               } // End of if (sLine.left(1) != "#")
             } // while(!sLine.contains("!END"))
          } // if (sLine.contains("!WATERPHANTOM"))

          if (sLine.contains("!SINGLEBEAM")) {
             while(!sLine.contains("!END")){
               QString rLine = stream.readLine();
               QString lLine = rLine.toLatin1();
               sLine = lLine.simplified();
               if (sLine.contains("profile z :")) {
                 dummyX =
                    sLine.section(':',1,1).section(',',0,0).simplified();
                 dummyZ =
                    sLine.section(':',1,1).section(',',1,1).simplified();
                 nProfileY++;
                 nProfiles++;
               }
               if (sLine.contains("profile x :")) {
                 dummyZ =
                    sLine.section(':',1,1).section(',',0,0).simplified();
                 dummyY =
                    sLine.section(':',1,1).section(',',1,1).simplified();
                 nProfileX++;
                 nProfiles++;
               }
               if (sLine.contains("profile y :")) {
                 dummyX =
                    sLine.section(':',1,1).section(',',0,0).simplified();
                 dummyY =
                    sLine.section(':',1,1).section(',',1,1).simplified();
                 nProfileZ++;
                 nProfiles++;
               }
               if (sLine.contains("slice x :")) {
                 dummyX =
                    sLine.section(':',1,1).section(',',0,0).simplified();
                 nProfileX++;
                 nProfiles++;
               }
               if (sLine.contains("slice y :")) {
                 dummyZ =
                    sLine.section(':',1,1).section(',',0,0).simplified();
                 nProfileZ++;
                 nProfiles++;
               }
               if (sLine.contains("slice z :")) {
                 dummyY =
                    sLine.section(':',1,1).section(',',0,0).simplified();
                 nProfileY++;
                 nProfiles++;
               }
             } // while(!sLine.contains("!END"))
          } // if (sLine.contains("!SINGLEBEAM"))

        } // if (sLine.left(1) != "#")
      } // while ( !stream.atEnd() )
    mFile.close();
 }
 QString VALUE = "N/A";
 if (OPT == "xISO") VALUE = isocenterX;
 if (OPT == "yISO") VALUE = isocenterY;
 if (OPT == "zISO") VALUE = isocenterZ;
 if (OPT == "GRIDSIZE") VALUE = doseGridSize;
 bool ok;
 if (OPT == "xPHANTOM") {
   int xSize = doseGridSize.toInt(&ok,10) * dimensionX.toInt(&ok,10);
   VALUE.sprintf("%d", xSize);
 }
 if (OPT == "yPHANTOM") {
   int ySize = doseGridSize.toInt(&ok,10) * dimensionZ.toInt(&ok,10);
   VALUE.sprintf("%d", ySize);
 }
 if (OPT == "zPHANTOM") {
   int zSize = doseGridSize.toInt(&ok,10) * dimensionY.toInt(&ok,10);
   VALUE.sprintf("%d", zSize);
 }
 // std::cout << "FNAME = " << FNAME << endl;
 // std::cout << "OPT = " << OPT << " VALUE = " << VALUE << endl;
 return (VALUE);
} // End of Verify::readVER()
// -----------------------------------------------------------------------------
void MainConsole::readBDT(QString fName) {
 // define bool variables
 QString version          = "";      bool version_found       = false;
 QString entry            = "";      bool entry_found         = false;
 QString ptl_type         = "";      bool ptl_type_found      = false;
 QString energy           = "";      bool energy_found        = false;
 QString model_id         = "";      bool model_id_found      = false;
 QString begin_parameter  = "";      bool begin_para_found    = false;
 QString end_parameter    = "";      bool end_para_found      = false;

 // initialization
 QString p_pri            = "";      bool p_pri_found         = false;
 QString p_sct            = "";
 QString pri_distance     = "";      bool pri_distance_found  = false;
 QString sigma_pri        = "";      bool sigma_pri_found     = false;
 QString horn_0           = "";      bool horn_0_found        = false;
 QString horn_1           = "";      bool horn_1_found        = false;
 QString horn_2           = "";      bool horn_2_found        = false;
 QString horn_3           = "";      bool horn_3_found        = false;
 QString horn_4           = "";      bool horn_4_found        = false;
 QString sct_distance     = "";      bool sct_distance_found  = false;
 QString sigma_sct        = "";      bool sigma_sct_found     = false;
 QString col_mdistance    = "";      bool col_mdistance_found = false;
 QString col_cdistance    = "";      bool col_cdistance_found = false;
 QString col_xdistance    = "";      bool col_xdistance_found = false;
 QString col_ydistance    = "";      bool col_ydistance_found = false;
 QString norm_value       = "";      bool norm_value_found    = false;
 QString gray_mu_dmax     = "";      bool gray_mu_dmax_found  = false;
 QString energy_min       = "";      bool energy_min_found    = false;
 QString energy_max       = "";      bool energy_max_found    = false;
 QString l_value          = "";      bool l_value_found       = false;
 QString b_value          = "";      bool b_value_found       = false;
 QString a_value          = "";      bool a_value_found       = false;
 QString z_value          = "";      bool z_value_found       = false;
 QString p_con            = "";      bool p_con_found         = false;
 QString distance_con     = "";      bool distance_con_found  = false;
 QString radius_con       = "";      bool radius_con_found    = false;
 QString e_mean_con       = "";      bool e_mean_con_found    = false;
 QString nu_value         = "0.45";  bool nu_value_found      = false;

 // bool ok;
 // read input file
 QFile iFile( fName );
 if ( iFile.open( QIODevice::ReadOnly ) ) {
  QTextStream stream( &iFile );
  QString line;
  while ( !stream.atEnd() ) {
   line = stream.readLine(); // line of text excluding '\n'
   QString strLine = line.toLatin1();
   if (strLine.contains("BASE-DATA-FILE-VERSION:") != 0) {
    version = getKeyValue(strLine); version_found = true;
    usr->version = version;
   }
   if (strLine.contains("BASE-DATA-ENTRY") != 0) {
    entry_found = true;
   }
   if (strLine.contains("PARTICLE-TYPE:") != 0) {
    ptl_type = getKeyValue(strLine); ptl_type_found = true;
    usr->particle = ptl_type;
   }
   if (strLine.contains("NOMINAL-ENERGY:") != 0) {
    energy = getKeyValue(strLine); energy_found = true;
    usr->E = energy;
   }
   if (strLine.contains("BEAM-MODEL-ID:") != 0) {
    model_id = getKeyValue(strLine); model_id_found = true;
    usr->ID = model_id;
   }
   if (strLine.contains("BEGIN-PARAMETERS") != 0) {
    begin_para_found = true;
   }
   if (strLine.contains("PRIMARY-PHOTONS:") != 0) {
    p_pri = getKeyValue(strLine); p_pri_found = true;
    usr->p0 = p_pri;
   }
   if (strLine.contains("PRIMARY-DIST:") != 0) {
    pri_distance = getKeyValue(strLine); pri_distance_found = true;
    usr->Z0 = pri_distance;
   }
   if (strLine.contains("PRIMARY-SIGMA:") != 0) {
    sigma_pri = getKeyValue(strLine); sigma_pri_found = true;
    usr->s0 = sigma_pri;
   }
   if (strLine.contains("SCATTER-DIST:") != 0) {
    sct_distance = getKeyValue(strLine); sct_distance_found = true;
    usr->ZS = sct_distance;
   }
   if (strLine.contains("SCATTER-SIGMA:") != 0) {
    sigma_sct = getKeyValue(strLine); sigma_sct_found = true;
    usr->ss = sigma_sct;
   }
   if (strLine.contains("PRIMARY-HORN0:") != 0) {
    horn_0 = getKeyValue(strLine); horn_0_found = true;
    usr->h0 = horn_0;
   }
   if (strLine.contains("PRIMARY-HORN1:") != 0) {
    horn_1 = getKeyValue(strLine); horn_1_found = true;
    usr->h1 = horn_1;
   }
   if (strLine.contains("PRIMARY-HORN2:") != 0) {
    horn_2 = getKeyValue(strLine); horn_2_found = true;
    usr->h2 = horn_2;
   }
   if (strLine.contains("PRIMARY-HORN3:") != 0) {
    horn_3 = getKeyValue(strLine); horn_3_found = true;
    usr->h3 = horn_3;
   }
   if (strLine.contains("PRIMARY-HORN4:") != 0) {
    horn_4 = getKeyValue(strLine); horn_4_found = true;
    usr->h4 = horn_4;
   }
   if (strLine.contains("COLM-DIST:") != 0) {
    col_mdistance = getKeyValue(strLine); col_mdistance_found = true;
    usr->ZM = col_mdistance;
   }
   if (strLine.contains("COLC-DIST:") != 0) {
    col_cdistance = getKeyValue(strLine); col_cdistance_found = true;
    usr->ZC = col_cdistance;
   }
   if (strLine.contains("COLX-DIST:") != 0) {
    col_xdistance = getKeyValue(strLine); col_xdistance_found = true;
    usr->ZX = col_xdistance;
   }
   if (strLine.contains("COLY-DIST:") != 0) {
    col_ydistance = getKeyValue(strLine); col_ydistance_found = true;
    usr->ZY = col_ydistance;
   }
   if (strLine.contains("NORM-VALUE:") != 0) {
    norm_value = getKeyValue(strLine); norm_value_found = true;
    usr->normValue = norm_value;
   }
   if (strLine.contains("GY/MU-DMAX:") != 0) {
    gray_mu_dmax = getKeyValue(strLine); gray_mu_dmax_found = true;
    usr->gy_mu_dmax = gray_mu_dmax;
   }
   if (strLine.contains("ENERGY-MAX:") != 0) {
    energy_max = getKeyValue(strLine); energy_max_found = true;
    usr->Emax = energy_max;
   }
   if (strLine.contains("ENERGY-MIN:") != 0) {
    energy_min = getKeyValue(strLine); energy_min_found = true;
    usr->Emin = energy_min;
   }
   if (strLine.contains("L-VALUE:") != 0) {
    l_value = getKeyValue(strLine); l_value_found = true;
    usr->lval = l_value;
   }
   if (strLine.contains("B-VALUE:") != 0) {
    b_value = getKeyValue(strLine); b_value_found = true;
    usr->bval = b_value;
   }
   if (strLine.contains("A-VALUE:") != 0) {
    a_value = getKeyValue(strLine); a_value_found = true;
    // usr->aval = a_value;
     usr->aval = usr->AVAL;
   }
   if (strLine.contains("Z-VALUE:") != 0) {
    z_value = getKeyValue(strLine); z_value_found = true;
    // usr->zval = z_value;
    usr->zval = usr->ZVAL;
   }
   if (strLine.contains("CHARGED-PARTICLES:") != 0) {
    p_con = getKeyValue(strLine); p_con_found = true;
    usr->pcon = p_con;
   }
   if (strLine.contains("CHARGED-DIST:") != 0) {
    distance_con = getKeyValue(strLine); distance_con_found = true;
    usr->ZE = distance_con;
   }
   if (strLine.contains("CHARGED-RADIUS:") != 0) {
    radius_con = getKeyValue(strLine); radius_con_found = true;
     if (radius_con != "") usr->FFRad = radius_con;
   }
   if (strLine.contains("CHARGED-E-MEAN:") != 0) {
    e_mean_con = getKeyValue(strLine); e_mean_con_found = true;
    usr->eEnergy = e_mean_con;
   }
   if (strLine.contains("NU-VALUE:") != 0) {
    nu_value = getKeyValue(strLine); nu_value_found = true;
    usr->nu = nu_value;
   }
   if (strLine.contains("END-PARAMETERS") != 0) {
    end_para_found = true;
   }
  }
 }
 iFile.close();
} //  End of void MainConsole::readBDT(QString fName)
// -----------------------------------------------------------------------------
void MainConsole::writeBDT(QString bdtFile) {
// bool ok;

 // usr->aval = ui->lineEditAval->text(); // Takes A-Value from WaterFit GUI

 QFile oFile(bdtFile);
 oFile.remove();
 oFile.open( QIODevice::Unbuffered | QIODevice::ReadWrite );
 QTextStream oStream( &oFile );

// oStream << "BASE-DATA-FILE-VERSION:  1.3" << endl;
#ifdef XVMC
 oStream << "BASE-DATA-FILE-VERSION:  1.4" << endl;
#else
 oStream << "BASE-DATA-FILE-VERSION:  1.5" << endl;
#endif
 oStream << "BASE-DATA-ENTRY" << endl;
 oStream << "   PARTICLE-TYPE:     " << ui->lineEditPType->text() << endl;
 oStream << "   NOMINAL-ENERGY:    " << ui->lineEditE->text() << endl;
 oStream << "   BEAM-MODEL-ID:     " << usr->ID << endl;
 oStream << "   BEGIN-PARAMETERS" << endl;
 oStream << "      PRIMARY-PHOTONS:    " << ui->lineEditP0->text() << endl;
#ifdef XVMC
 // oStream << "      PRIMARY-DIST:       " << ui->lineEditZ0->text() << endl;
 oStream << "      PRIMARY-DIST:       " << usr->Z0 << endl;
#endif
 oStream << "      PRIMARY-SIGMA:      " << ui->lineEditS0->text() << endl;
 oStream << "      PRIMARY-HORN0:      " << ui->lineEditH0->text() << endl;
 oStream << "      PRIMARY-HORN1:      " << ui->lineEditH1->text() << endl;
 oStream << "      PRIMARY-HORN2:      " << ui->lineEditH2->text() << endl;
 oStream << "      PRIMARY-HORN3:      " << ui->lineEditH3->text() << endl;
 oStream << "      PRIMARY-HORN4:      " << ui->lineEditH4->text() << endl;
 oStream << "      SCATTER-DIST:       " << ui->lineEditZS->text() << endl;
 oStream << "      SCATTER-SIGMA:      " << ui->lineEditSS->text() << endl;
#ifdef XVMC
 // oStream << "      COLM-DIST:          " << ui->lineEditZM->text() << endl;
 // oStream << "      COLC-DIST:          " << ui->lineEditZC->text() << endl;
 // oStream << "      COLX-DIST:          " << ui->lineEditZX->text() << endl;
 // oStream << "      COLY-DIST:          " << ui->lineEditZY->text() << endl;
   oStream << "      COLM-DIST:          " << usr->ZM << endl;
   oStream << "      COLC-DIST:          " << usr->ZC << endl;
   oStream << "      COLX-DIST:          " << usr->ZX << endl;
   oStream << "      COLY-DIST:          " << usr->ZY << endl;
#endif
 oStream << "      NORM-VALUE:         " << ui->lineEditNorm->text() << endl;
 oStream << "      GY/MU-DMAX:         " << ui->lineEditGyMU->text() << endl;
 oStream << "      ENERGY-MIN:         " << ui->lineEditEmin->text() << endl;
 oStream << "      ENERGY-MAX:         " << ui->lineEditEmax->text() << endl;
 oStream << "      L-VALUE:            " << ui->lineEditLval->text() << endl;
 oStream << "      B-VALUE:            " << ui->lineEditBval->text() << endl;
 oStream << "      A-VALUE:            " << ui->lineEditAval->text() << endl;
 oStream << "      Z-VALUE:            " << ui->lineEditZval->text() << endl;
 // if (usr->nu.toFloat(&ok) == 0.0) usr->nu = "0.45";
 // oStream << "      NU-VALUE:           " << ui->lineEditNUval->text() << endl;
 oStream << "      NU-VALUE: 0.45" << endl;
 oStream << "      CHARGED-PARTICLES:  " << ui->lineEditPcon->text() << endl;
 oStream << "      CHARGED-DIST:       " << ui->lineEditZS->text() << endl;
 oStream << "      CHARGED-RADIUS:     " << ui->lineEditFFRad->text() << endl;
 oStream << "      CHARGED-E-MEAN:     " << ui->lineEditEmean->text() << endl;
 oStream << "   END-PARAMETERS" << endl;

 oFile.close();
} // End of writeBDT
// -----------------------------------------------------------------------------
void MainConsole::readAirFitInp(QString fname) {
  QFile mFile(fname);
  if (mFile.exists()) {
    QTextStream stream( &mFile );
    QString sLine;
    mFile.open( QIODevice::ReadOnly );
    while ( !stream.atEnd() ) {
      sLine = stream.readLine();
      if (sLine.contains("version:")) {
         QString version = sLine.section(':',1,1);
         usr->version = version.simplified();
      }
      if (sLine.contains("energy:")) {
         QString energy = sLine.section(':',1,1);
         usr->E = energy.simplified();
      }
      if (sLine.contains("p0:")) {
         QString p0 = sLine.section(':',1,1).simplified();
         usr->p0 = p0.section(' ',0,0);
      }
      if (sLine.contains("s0:")) {
         QString s0 = sLine.section(':',1,1).simplified();
         usr->s0 = s0.section(' ',0,0);
      }
      if (sLine.contains("h0:")) {
         QString h0 = sLine.section(':',1,1).simplified();
         usr->h0 = h0.section(' ',0,0);
      }
      if (sLine.contains("h1:")) {
         QString h1 = sLine.section(':',1,1).simplified();
         usr->h1 = h1.section(' ',0,0);
      }
      if (sLine.contains("h2:")) {
         QString h2 = sLine.section(':',1,1).simplified();
         usr->h2 = h2.section(' ',0,0);
      }
      if (sLine.contains("h3:")) {
         QString h3 = sLine.section(':',1,1).simplified();
         usr->h3 = h3.section(' ',0,0);
      }
      if (sLine.contains("h4:")) {
         QString h4 = sLine.section(':',1,1).simplified();
         usr->h4 = h4.section(' ',0,0);
      }
      if (sLine.contains("ss:")) {
         QString ss = sLine.section(':',1,1).simplified();
         usr->ss = ss.section(' ',0,0);
      }
      if (sLine.contains("ZS:")) {
         QString ZS = sLine.section(':',1,1).simplified();
         usr->ZS = ZS;
      }
      if (sLine.contains("Z0:")) {
         QString Z0 = sLine.section(':',1,1).simplified();
         usr->Z0 = Z0;
      }
      if (sLine.contains("ZM:")) {
         QString ZM = sLine.section(':',1,1).simplified();
         usr->ZM = ZM.section(' ',0,0);
         usr->MX = ZM.section(' ',1,1);
         usr->MY = ZM.section(' ',2,2);
      }
      if (sLine.contains("ZX:")) {
         QString ZX = sLine.section(':',1,1).simplified();
         usr->ZX = ZX;
      }
      if (sLine.contains("ZY:")) {
         QString ZY = sLine.section(':',1,1).simplified();
         usr->ZY = ZY;
      }
      if (sLine.contains("XN:")) {
         QString XN = sLine.section(':',1,1).simplified();
         usr->XN = XN;
      }
      if (sLine.contains("YN:")) {
         QString YN = sLine.section(':',1,1).simplified();
         usr->YN = YN;
      }
      if (sLine.contains("ZN:")) {
         QString ZN = sLine.section(':',1,1).simplified();
         usr->ZN = ZN;
      }
      if (sLine.contains("WXN:")) {
         QString WXN = sLine.section(':',1,1).simplified();
         usr->WXN = WXN;
      }
      if (sLine.contains("WYN:")) {
         QString WYN = sLine.section(':',1,1).simplified();
         usr->WYN = WYN;
      }
    } // End of While
    mFile.close();
   }
} // End of readAirFitInp(QString fname)
// -----------------------------------------------------------------------------
void MainConsole::readWaterFitOut(QString fname) {
  // bool ok;
  QString CMDERROR = "";
  QFile mFile(fname);
  if (mFile.exists()) {
    QTextStream stream( &mFile );
    QString sLine;
    mFile.open( QIODevice::ReadOnly );
    while ( !stream.atEnd() ) {
      sLine = stream.readLine();
      if (!sLine.contains(',')) {
        if (sLine.contains("norm:")) {
           QString norm = sLine.section(':',1,1).section("+/-",0,0);
           usr->norm = norm.simplified();
        }
        if (sLine.contains("l_value:")) {
           QString lval = sLine.section(':',1,1);
           usr->lval = lval.simplified();
        }
        if (sLine.contains("b_value:")) {
           QString bval = sLine.section(':',1,1);
           usr->bval = bval.simplified();
        }
//        if (sLine.contains("a_value:")) {
//           QString aval = sLine.section(':',1,1);
//           usr->aval = aval.simplified();
//        }
        if (sLine.contains("energy_min:")) {
           QString Emin = sLine.section(':',1,1);
           usr->Emin = Emin.simplified();
        }
        if (sLine.contains("energy_max:")) {
           QString Emax = sLine.section(':',1,1);
           usr->Emax = Emax.simplified();
        }
        if (sLine.contains("p_con:")) {
           QString pcon = sLine.section(':',1,1);
           usr->pcon = pcon.simplified();
        }
        if (sLine.contains("gy_mu_dmax:")) {
           QString gymu = sLine.section(':',1,1).section('(',0,0);
           usr->gy_mu_dmax = gymu.simplified();
        }
        if (sLine.contains("Emean:")) {
           QString Emean = sLine.section(':',1,1);
           usr->Emean = Emean.simplified();
        }
        if (sLine.contains("Eprob:")) {
           QString Eprob = sLine.section(':',1,1);
           usr->Eprob = Eprob.simplified();
        }
        if (sLine.contains("norm_value:")) {
           QString normValue = sLine.section(':',1,1);
           usr->normValue = normValue.simplified();
        }
        //if (sLine.contains("Writing file:")) {
        //   QString writeFile = sLine.section(':',1,1);
        //   usr->writeFile = writeFile.simplified();
        //}
        if (sLine.contains("Numerical Recipes run-time error...")) {
           CMDERROR = "Numerical Recipes run-time error...";
        }
      } else {
        if (sLine.contains("number of total points:")) {
           QString DEV = sLine.section("average deviation:",1,1)
                              .section("%",0,0);
           usr->avgdev = DEV.simplified();
        }
        if (sLine.contains("number of fit points:")) {
           QString CHI = sLine.section("chi:",1,1).section("%",1,1);
           usr->chi = CHI.simplified();
        }
      }
    } // End of While
    mFile.close();
   }
}
// End of readWaterFitOut
// -----------------------------------------------------------------------------
void MainConsole::updatePBCommInfo() {
   QString mName = ui->comboBoxMachine->currentText();
   QString XVMC_WORK = usr->LHOME + mName;
   QDir *dirPBComm = new QDir;
   dirPBComm->setPath(XVMC_WORK);
   dirPBComm->setNameFilters(QStringList("PBComm_*"));
   QStringList dList = dirPBComm->entryList(QDir::Dirs, QDir::Name);
   ui->comboBoxSSD->clear();
   ui->lineEditGridSize->setText("");
   ui->lineEditPSizeX->setText("");
   ui->lineEditPSizeY->setText("");
   ui->lineEditPSizeZ->setText("");

   for (QStringList::Iterator it = dList.begin(); it != dList.end(); ++it){
      QString pName = *it;
      if (!pName.startsWith(".")) {
         QString D6MV_VER = XVMC_WORK + "/" + *it + "/D6MV.VER";
         // std::cout << "D6MV_VER = " << D6MV_VER << endl;
         QString D6MV_CLN = XVMC_WORK + "/" + *it + "/D6MV.CLN";
         QFile VER(D6MV_VER);
         QFile CLN(D6MV_VER);
         if (VER.exists() && CLN.exists()) {
           QString SSDcm = pName.section('_',-2,-1);
           ui->comboBoxSSD->addItem(SSDcm);
         }
      }
   }
   updatePBCommValue();
}
// -----------------------------------------------------------------------------
void MainConsole::updatePBCommValue() {
 QString SSDcm = ui->comboBoxSSD->currentText();
 QString mName = ui->comboBoxMachine->currentText();
 QString XVMC_WORK = usr->LHOME + mName;
 QString VER = XVMC_WORK + "/PBComm_"+SSDcm+"/D6MV.VER";
 QString GridSize = readVER(VER, "GRIDSIZE");
 ui->lineEditGridSize->setText(GridSize);
 QString xPhantom = readVER(VER, "xPHANTOM");
 ui->lineEditPSizeX->setText(xPhantom);
 QString yPhantom = readVER(VER, "yPHANTOM");
 ui->lineEditPSizeZ->setText(yPhantom);
 QString zPhantom = readVER(VER, "zPHANTOM");
 ui->lineEditPSizeY->setText(zPhantom);
}
// -----------------------------------------------------------------------------
void MainConsole::updatePBInfo() {
   QString mName = ui->comboBoxMachine->currentText();
   QString XVMC_WORK = usr->LHOME + mName;
   QDir *dirPB = new QDir;
   dirPB->setPath(XVMC_WORK);
   dirPB->setNameFilters(QStringList("PB_*"));
   QStringList dList = dirPB->entryList(QDir::Dirs, QDir::Name);
   ui->comboBoxPBSSD->clear();

   for (QStringList::Iterator it = dList.begin(); it != dList.end(); ++it){
      QString pName = *it;
      if (!pName.startsWith(".")) {
         QString SSDcm = pName.section('_',-2,-1);
         ui->comboBoxPBSSD->addItem(SSDcm);
      }
   }
}
// -----------------------------------------------------------------------------
void MainConsole::updateBdtValues() {
   updateBdtValuesAir();
   updateBdtValuesWater();

    ui->lineEditE->setText(usr->E);
    ui->lineEditP0->setText(usr->p0);
    ui->lineEditS0->setText(usr->s0);
    ui->lineEditZS->setText(usr->ZS);
    ui->lineEditSS->setText(usr->ss);
    ui->lineEditFFRad->setText(usr->FFRad);
    ui->lineEditZE->setText(usr->ZE);
    ui->lineEditEmean->setText(usr->eEnergy);
    ui->lineEditH0->setText(usr->h0);
    ui->lineEditH1->setText(usr->h1);
    ui->lineEditH2->setText(usr->h2);
    ui->lineEditH3->setText(usr->h3);
    ui->lineEditH4->setText(usr->h4);

    ui->lineEditPType->setText(usr->particle);
    ui->lineEditLval->setText(usr->lval);
    ui->lineEditBval->setText(usr->bval);
    ui->lineEditAval->setText(usr->aval);
    ui->lineEditZval->setText(usr->zval);
    ui->lineEditNUval->setText(usr->nu);
    ui->lineEditNorm->setText(usr->normValue);
    ui->lineEditGyMU->setText(usr->gy_mu_dmax);
    ui->lineEditEmin->setText(usr->Emin);
    ui->lineEditEmax->setText(usr->Emax);
    ui->lineEditPcon->setText(usr->pcon);
}
// -----------------------------------------------------------------------------
void MainConsole::clearBdtValues() {
 usr->particle="";
 usr->E="";
 usr->p0="";
 usr->s0="";
 usr->ZS="";
 usr->ss="";
 usr->FFRad="";
 usr->ZE="";
 usr->eEnergy="";
 usr->h0="";
 usr->h1="";
 usr->h2="";
 usr->h3="";
 usr->h4="";
 usr->lval="";
 usr->bval="";
 usr->aval="";
 usr->zval="";
 usr->nu="";
 usr->normValue="";
 usr->gy_mu_dmax="";
 usr->Emin="";
 usr->Emax="";
 usr->pcon="";
}
// -----------------------------------------------------------------------------
void MainConsole::updateBdtValuesAir() {
 // Parameters in afit.inp
 ui->lineEditPTypeAir->setText(usr->particle);
 ui->lineEditEAir->setText(usr->E);
 ui->lineEditP0Air->setText(usr->p0);
 ui->lineEditS0Air->setText(usr->s0);
 ui->lineEditZSAir->setText(usr->ZS);
 ui->lineEditSSAir->setText(usr->ss);
 ui->lineEditFFRadAir->setText(usr->FFRad);
 ui->lineEditZEAir->setText(usr->ZE);
 ui->lineEditEmeanAir->setText(usr->eEnergy);
 ui->lineEditH0Air->setText(usr->h0);
 ui->lineEditH1Air->setText(usr->h1);
 ui->lineEditH2Air->setText(usr->h2);
 ui->lineEditH3Air->setText(usr->h3);
 ui->lineEditH4Air->setText(usr->h4);
}
// -----------------------------------------------------------------------------
void MainConsole::updateBdtValuesWater() {
 // Parameters in wfit.inp
 ui->lineEditEWater->setText(usr->E);
 ui->lineEditPTypeWater->setText(usr->particle);
 ui->lineEditLvalWater->setText(usr->lval);
 ui->lineEditBvalWater->setText(usr->bval);
 ui->lineEditAvalWater->setText(usr->aval);
 ui->lineEditZvalWater->setText(usr->zval);
 ui->lineEditNUvalWater->setText(usr->nu);
 ui->lineEditNormWater->setText(usr->normValue);
 ui->lineEditGyMUWater->setText(usr->gy_mu_dmax);
 ui->lineEditEminWater->setText(usr->Emin);
 ui->lineEditEmaxWater->setText(usr->Emax);
 ui->lineEditPconWater->setText(usr->pcon);
}
// -----------------------------------------------------------------------------
void MainConsole::resetBDT() {
   QString mName = ui->comboBoxMachine->currentText();
   QString XVMC_WORK = usr->LHOME + mName;
    QString WFIT_DIR = XVMC_WORK + "/wfit";
    QString WFIT_OUT = WFIT_DIR + "/wfit.out";
    QString AFIT_DIR = XVMC_WORK + "/afit";
    QString AFIT_INP = AFIT_DIR + "/afit.inp";
    QFile wFile(WFIT_OUT);
    QFile aFile(AFIT_INP);
    if (wFile.exists() && aFile.exists()) {
      readWaterFitOut(WFIT_OUT);
      readAirFitInp(AFIT_INP);
      updateBdtValues();
    }
}
// -----------------------------------------------------------------------------
void MainConsole::updateAirFitValues() {
   QString mName = ui->comboBoxMachine->currentText();
   QString XVMC_WORK = usr->LHOME + mName;
   QString AFIT_DIR = XVMC_WORK + "/afit";
   QString AFIT_INP = AFIT_DIR + "/afit.inp";
   QFile aFile(AFIT_INP);
   if (aFile.exists()) {
     readAirFitInp(AFIT_INP); // AFIT_INP must be afit.inp because fitting is done
     ui->lineEditPTypeAir->setText(usr->particle);
     ui->lineEditEAir->setText(usr->E);
     ui->lineEditP0Air->setText(usr->p0);
     ui->lineEditS0Air->setText(usr->s0);
     ui->lineEditZSAir->setText(usr->ZS);
     ui->lineEditSSAir->setText(usr->ss);
     ui->lineEditFFRadAir->setText(usr->FFRad);
     ui->lineEditZEAir->setText(usr->ZE);
      if(usr->eEnergy == "") {
          bool ok;
          float eE = 0.13 * usr->E.toFloat(&ok) + 0.55;
        usr->eEnergy.sprintf("%.3f",eE);
      }
     ui->lineEditEmeanAir->setText(usr->eEnergy);
     ui->lineEditH0Air->setText(usr->h0);
     ui->lineEditH1Air->setText(usr->h1);
     ui->lineEditH2Air->setText(usr->h2);
     ui->lineEditH3Air->setText(usr->h3);
     ui->lineEditH4Air->setText(usr->h4);
  }
}
// -----------------------------------------------------------------------------
void MainConsole::updateWaterFitValues() {
   QString mName = ui->comboBoxMachine->currentText();
   QString XVMC_WORK = usr->LHOME + mName;
    QString WFIT_DIR = XVMC_WORK + "/wfit";
    QString WFIT_OUT = WFIT_DIR + "/wfit.out";
    QString AFIT_DIR = XVMC_WORK + "/afit";
    QString AFIT_INP = AFIT_DIR + "/afit.inp";
    QFile wFile(WFIT_OUT);
    QFile aFile(AFIT_INP);
    if (wFile.exists() && aFile.exists()) {
        readAirFitInp(AFIT_INP);
    readWaterFitOut(WFIT_OUT);
        ui->lineEditPTypeWater->setText(usr->particle);
        ui->lineEditEWater->setText(usr->E);
        ui->lineEditLvalWater->setText(usr->lval);
        ui->lineEditBvalWater->setText(usr->bval);
        ui->lineEditAvalWater->setText(usr->aval);
        ui->lineEditZvalWater->setText(usr->zval);
        ui->lineEditNUvalWater->setText(usr->nu);
        ui->lineEditNormWater->setText(usr->normValue);
        ui->lineEditGyMUWater->setText(usr->gy_mu_dmax);
        ui->lineEditEminWater->setText(usr->Emin);
        ui->lineEditEmaxWater->setText(usr->Emax);
        ui->lineEditPconWater->setText(usr->pcon);
    ui->lineEditEmeanAir->setText(usr->eEnergy);
   }
}
// -----------------------------------------------------------------------------
QString MainConsole::readLogfile(QString logFile, QString keyWord) {
  QString rValue = "";
  QFile file( logFile );
  if ( file.open( QIODevice::ReadOnly ) ) {
    QTextStream stream( &file );
    QString line;
  while ( !stream.atEnd() ) {
         line = stream.readLine(); // line of text excluding '\n'
         QString strLine = line.toLatin1();
         if (strLine.contains(keyWord)) {
            rValue = strLine.section(keyWord,1,1);
         }
    }
    file.close();
  }
  return rValue.simplified();
}
// -----------------------------------------------------------------------------
void MainConsole::scalePDD(QString DIR, QString PLAN, float FACTOR) {
 vector<vector<double> > vPos; // Scan Point Position
 vector<vector<double> > vVal; // Scan Value
 vector<double> xPos;
 vector<double> yPos;
 vector<double> zPos;

 for (int i=0; i<1; i++) {
  stringstream iStrStream;
  iStrStream << DIR.toStdString() + "/" << PLAN.toStdString() << "_BEAM1_PROFILE" << i+1 << ".txt";
  string FileName = iStrStream.str();
  // std::cout << FileName << endl;
  fstream fin;
  fin.open(FileName.c_str(), std::ios::in);
  if (!fin.is_open()) {
   cerr << "ERROR: Failed to open " << FileName << endl;
   exit(1);
  }

  string sToken("");
  int iToken = 0;
  // X Pos
  fin >> sToken; iToken++;
  if (sToken != "#") {
   cerr << "ERROR: sToken (" << sToken << ") is not # (" << iToken << ")" << endl;
   exit(1);
  }

  fin >> sToken;
  if (sToken != "X:") {
   cerr << "ERROR: sToken (" << sToken << ") is not X: (" << iToken << ")" << endl;
   exit(1);
  }

  fin >> sToken;
  xPos.push_back(atof(sToken.c_str()));

  // Y Pos
  fin >> sToken; iToken++;
  if (sToken != "#") {
   cerr << "ERROR: sToken (" << sToken << ") is not # (" << iToken << ")" << endl;
   exit(1);
  }

  fin >> sToken;
  if (sToken != "Y:") {
   cerr << "ERROR: sToken (" << sToken << ") is not Y: (" << iToken << ")" << endl;
   exit(1);
  }

  fin >> sToken;
  yPos.push_back(atof(sToken.c_str()));

  // Z Pos
  fin >> sToken; iToken++;
  if (sToken != "#") {
   cerr << "ERROR: sToken (" << sToken << ") is not # (" << iToken << ")" << endl;
   exit(1);
  }

  fin >> sToken;
  if (sToken != "Z") {
   cerr << "ERROR: sToken (" << sToken << ") is not Z (" << iToken << ")" << endl;
   exit(1);
  }

  vPos.push_back(vector<double>());
  vVal.push_back(vector<double>());
  while (!fin.eof()){
   fin >> sToken; iToken++;
   vPos[i].push_back(atof(sToken.c_str()));
   fin >> sToken;
   vVal[i].push_back(atof(sToken.c_str()));
  }
  fin.close();
 } // for (int i=0; i<1; i++)

 double xSum = 0.0;
 double ySum = 0.0;
 for (int i=0; i<1; i++) {
  xSum += xPos[i];
  ySum += yPos[i];
 }
 stringstream oStrStream;
 oStrStream << DIR.toStdString() + "/" << PLAN.toLower().toStdString() << "_BEAM1_PROFILE1.txt";
 string oFileName = oStrStream.str();
 fstream fout;
 fout.open(oFileName.c_str(), std::ios::out);
 if (!fout.is_open()){
  cerr << "ERROR: Failed to open " << oFileName << endl;
  exit(1);
 }

 fout << "#  X:     " << xSum/4.0 << endl;
 fout << "#  Y:     " << ySum/4.0 << endl;
 fout << "#  Z      " << endl;
 int nData = vPos[0].size() - 1;
 for (int i=0; i<nData; i++) {
  fout << vPos[0][i];
  double sum = 0.0;
  for (int k=0; k<1; k++) {
   sum += vVal[k][i];
  }
  fout << "  " << sum*FACTOR << endl;
 }
 fout.close();
}
// -----------------------------------------------------------------------------
void MainConsole::readGridSizeVer(){
   QString mName = ui->comboBoxMachine->currentText();
    bool ok;
   QString smallGridSizeItem = readLocalSetting(mName,"smallGridSize");
   QString largeGridSizeItem = readLocalSetting(mName,"largeGridSize");
    int iSmallGridSize = smallGridSizeItem.toInt(&ok,10);
    int iLargeGridSize = largeGridSizeItem.toInt(&ok,10);
    ui->comboBoxGridSizeSmall->setCurrentIndex(iSmallGridSize);
    ui->comboBoxGridSizeLarge->setCurrentIndex(iLargeGridSize);
}
// -----------------------------------------------------------------------------
void MainConsole::resetPBComm() {
    ui->pushButtonPBComm->setEnabled(true);
    ui->groupBoxMCPB->setEnabled(true);
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName, "PBComm", "NotDone");
}
// -----------------------------------------------------------------------------
void MainConsole::resetValidate() {
    ui->pushButtonPBPack->setEnabled(true);
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName, "Validate", "NotDone");
}
// -----------------------------------------------------------------------------
void MainConsole::resetAFIT() {
    ui->pushButtonAirFit->setEnabled(true);
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName, "AFIT", "NotDone");
}
// -----------------------------------------------------------------------------
void MainConsole::resetMonoMC() {
    ui->pushButtonMonoStart->setEnabled(true);
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName, "MonoMC", "NotDone");
}
// -----------------------------------------------------------------------------
void MainConsole::resetWFIT() {
    ui->pushButtonWfit->setEnabled(true);
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName, "WFIT", "NotDone");
}
// -----------------------------------------------------------------------------
void MainConsole::resetVerification() {
    ui->pushButtonVerification->setEnabled(true);
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName, "Verification", "NotDone");
}
// -----------------------------------------------------------------------------
void MainConsole::resetPBMC() {
    ui->pushButtonPBMC->setEnabled(true);
   QString mName = ui->comboBoxMachine->currentText();
   writeLocalSetting(mName, "PBMC", "NotDone");
}
// -----------------------------------------------------------------------------
void MainConsole::setCopyMachineName() {
    QString qsMachineName = ui->comboBoxOrigMachine->currentText();
    QString qsNewMachineName = qsMachineName+"_copy";
    ui->lineEditCopy->setText(qsNewMachineName);
}
// -----------------------------------------------------------------------------
void MainConsole::copyMachine() {
    QString oName = ui->comboBoxOrigMachine->currentText();
    QString tName = ui->lineEditCopy->text();
    int iAns = iCopyMachine();
    QString  MESSAGE = "Normal Termination";
   switch (iAns) {
      case 0:
            MESSAGE = "Normal Terminated";
            break;
      case 1:
            MESSAGE = "ERROR: No Source Machine (" + oName + ") found";
            break;
      case 2:
            MESSAGE = "ERROR: Target Machine (" + tName + ") ALREADY exists";
            break;
      case 3:
            MESSAGE = "ERROR: Failed to copy " + oName + " to " + tName;
            break;
      case 4:
            MESSAGE = "ERROR: No " + oName.toLower() + "rc exists";
            break;
      default:
            break;
   }
    if (iAns > 0) {
        QMessageBox::critical(this,
                            "Error Message in copyMachine",
                            MESSAGE,
                            QMessageBox::Abort,
                            QMessageBox::NoButton,
                            QMessageBox::NoButton);
   }
    getMCMachine();
}
// -----------------------------------------------------------------------------
int MainConsole::iCopyMachine() {
    QString oName = ui->comboBoxOrigMachine->currentText();
    QString tName = ui->lineEditCopy->text();
    QString LHOME = usr->LHOME;
    QString TELE_DIR = ui->lineEditTeleDir->text();

   // std::cout << "oName = " << oName << endl;
   // std::cout << "tName = " << tName << endl;
   // std::cout << "LHOME = " << LHOME << endl;
   // std::cout << "TELE_DIR = " << TELE_DIR << endl;

    // std::cout << "... Please wait ... Copying in progress ..." << endl;

    QString SRC = LHOME + oName;
    QString TGT = LHOME + tName;

    if (!isThereDir(SRC)) return(1);
    if (isThereDir(TGT)) return(2);

    QString CMD = "cp -R " + SRC + " " + TGT;
    mySystem(CMD);

    if (!isThereDir(TGT)) return(3);

    // Link Tele Directory/File
    QString TELE_SRC = TELE_DIR + oName;
    QString TELE_TGT = TELE_DIR + tName;
    if (isThereFile(TELE_DIR, oName)) {
        CMD = "ln -sf " + TELE_SRC + " " + TELE_TGT;
        mySystem(CMD);
    }

    // Rename Working Directory if exists
    if (isThereDir(TGT+"/"+oName)) {
        CMD = "mv " + TGT + "/" + oName + " " + TGT + "/" + tName;
        mySystem(CMD);
    } else {
     QMessageBox::warning( this,  "Warning Meaagse in copyMachine",
                                "WARNING: No previous calculation directory " + oName,
                                "&Acknowleged", QString::null, QString::null, 1, 1 );
   }

    // Rename MONO Directory if exists
    if (isThereDir(TGT+"/"+oName+"_MONO")) {
        CMD = "mv " + TGT + "/" + oName + "_MONO " + TGT + "/" + tName + "_MONO";
        mySystem(CMD);
    } else {
     QMessageBox::warning( this,  "Warning Meaagse in copyMachine",
                                "WARNING: No previous MONO directory " + oName,
                                "&Acknowleged", QString::null, QString::null, 1, 1 );
   }

    QString SRC_RC = TGT + "/" + oName.toLower() + "rc";
    QString TGT_RC = TGT + "/" + tName.toLower() + "rc";
    // std::cout << "SRC_RC = " << SRC_RC << endl;
    // std::cout << "TGT_RC = " << TGT_RC << endl;
    // Rename rc file
    if (isThereFile(TGT, oName.toLower() + "rc")) {
        CMD = "sed s/"+oName+"/"+ tName+"/g "+SRC_RC+" > "+TGT_RC;
        // std::cout << "CMD = " << CMD << endl;
        mySystem(CMD);
        CMD = "rm -f "+SRC_RC;
        mySystem(CMD);
    } else {
        return(4);
    }

    // Rename info file
    if (isThereFile(TGT,oName+".info")) {
        CMD = "mv " + TGT + "/" + oName + ".info " + TGT + "/" + tName + ".info";
        mySystem(CMD);
    } else {
     QMessageBox::warning( this, "Warning Meaagse in copyMachine",
                                "WARNING: No previous "+ oName + ".info file",
                                "&Acknowleged", QString::null, QString::null, 1, 1 );
   }

    // Rename BDT file
    if (isThereFile(TGT+"/dat/",oName+".bdt")) {
        CMD = "mv " + TGT + "/dat/" + oName + ".bdt " + TGT + "/dat/" + tName + ".bdt";
        mySystem(CMD);
    } else {
     QMessageBox::warning( this,  "Warning Meaagse in copyMachine",
                                "WARNING: No previous "+ oName + ".bdt in dat directory",
                                "&Acknowleged", QString::null, QString::null, 1, 1 );
   }

    // Rename RPB file
   QDir *qdRPB = new QDir;
   qdRPB->setPath(TGT+"/dat/");
    // std::cout << TGT+"/dat/" << endl;
   qdRPB->setNameFilters(QStringList(oName+"_*"));
   QStringList qslRPB = qdRPB->entryList(QDir::Files, QDir::Name);
   for (QStringList::Iterator it = qslRPB.begin(); it != qslRPB.end(); ++it){
      QString qsName = *it;
        QString qsSSD = qsName.section("_",1,1);
        QString qsGridSize = qsName.section("_",2,2).section(".RPB",0,0);
        // std::cout << oName+"_" + qsSSD + "_" + qsGridSize + ".RPB" << endl;
        if (isThereFile(TGT+"/dat/",oName+"_" + qsSSD + "_" + qsGridSize + ".RPB")) {
            CMD = "mv " + TGT + "/dat/" + qsName + " " + TGT + "/dat/" + tName + "_" + qsSSD + "_" + qsGridSize + ".RPB";
            // std::cout << CMD << endl;
            mySystem(CMD);
        } else {
      QMessageBox::warning( this,  "Warning Meaagse in copyMachine",
                                    "WARNING: No previous "+ oName + ".bdt in dat directory",
                                    "&Acknowleged", QString::null, QString::null, 1, 1 );
    }
    }

    // Rename BDT file
    if (isThereFile(TGT+"/dat/basedata/",oName+".bdt")) {
        CMD = "mv " + TGT + "/dat/basedata/" + oName + ".bdt " + TGT + "/dat/basedata/" + tName + ".bdt";
        mySystem(CMD);
    } else {
     QMessageBox::warning( this,  "Warning Meaagse in copyMachine",
                                "WARNING: No previous "+ oName + ".bdt in dat/basedata directory",
                                "&Acknowleged", QString::null, QString::null, 1, 1 );
   }

    return(0);
} // End of int MainConsole::iCopyMachine()
// -----------------------------------------------------------------------------
void MainConsole::adjustNorm() {
     QString LBIN = usr->LHOME + "bin";
     QString mName = ui->comboBoxMachine->currentText();
     bool ok;
     QString qsFactor = ui->lineEditOFAdjust->text();
     QString qsOldNorm = ui->lineEditNorm->text();
     QString qsNewNorm = qsOldNorm;
     float fFactor=qsFactor.toFloat(&ok);
     float fNewNorm = qsOldNorm.toFloat(&ok)*qsFactor.toFloat(&ok);
     if (ui->checkBoxOFAdjust->isChecked()) {
         qsFactor.sprintf("%8.5f", fFactor);
         qsNewNorm.sprintf("%10.6f",fNewNorm);
         ui->lineEditNorm->setText(qsNewNorm);
         ui->lineEditOFAdjust->setText("1.000");
         ui->pushButtonAdjustNorm->setEnabled(false);
         writeOFAdjust();
     }
} // End of adjustNorm()
// -----------------------------------------------------------------------------



void MainConsole::initMachineInfo() {
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

void MainConsole::clearModifiedAll()
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

void MainConsole::updateAll()
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

void MainConsole::setMDIR(QString usrText) {
 mInfoUsr->MDIR = usrText;
 // QTextOStream(&mInfoUsr->MDIR) << usrText;
}

void MainConsole::setLHOME(QString usrText) {
 mInfoUsr->LHOME = usrText;
}

void MainConsole::setMODEL(QString usrText) {
 mInfoUsr->MODEL = usrText;
 mInfoUsr->CurrentMachine = usrText;
}

void MainConsole::setModelFile(QString usrText) {
 mInfoUsr->CurrentModelFile = usrText;
}

void MainConsole::setVendor(QString usrText) {
 mInfoUsr->VENDOR = usrText;
}

void MainConsole::setTMPDIR(QString usrText) {
 mInfoUsr->TMPDIR = usrText;
}
/*
QString MainConsole::mm2cm(QString mm) {
  bool ok;
  float value = mm.toFloat(&ok)/10.0;
  mm.sprintf("%7.2f",value);
  mm.simplified();
  return(mm);
}
*/
void MainConsole::clearMachineInfo() {
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

void MainConsole::readParm(QString mDir) {
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
void MainConsole::readParm(QString mDir) {
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
void MainConsole::readCollim(QString mDir) {
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

void MainConsole::readMlc(QString mDir) {
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

void MainConsole::getMachineInfo() {
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

void MainConsole::resetMachine() {
   getXiOInfo();
   mInfoUsr->CurrentModelFile = "XiO Machine Files";
   updateAll();
}

void MainConsole::getXiOInfo() {
  clearMachineInfo();
  QString mDir;

  mDir = mInfoUsr->MDIR + "/" + mInfoUsr->CurrentMachine + "/parm" ;
  readParm(mDir);

  mDir = mInfoUsr->MDIR + "/" + mInfoUsr->CurrentMachine + "/collim" ;
  readCollim(mDir);

  mDir = mInfoUsr->MDIR + "/" + mInfoUsr->CurrentMachine + "/mlc" ;
  readMlc(mDir);
}

void MainConsole::getLocalInfo(QString modelFile) {
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

void MainConsole::writeModified (QString group){
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

void MainConsole::writeNotModified (QString group){
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

void MainConsole::writeMachineInfo() {
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

void MainConsole::checkModified(){

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

void MainConsole::Cancel(){
  getMachineInfo();
  updateAll();
  writeNotModified(mInfoUsr->CurrentMachine);
  // close();
}

void MainConsole::on_xMLCcheckBox_clicked()
{
    if(ui->xMLCcheckBox->isChecked())
        ui->xMLCcheckBox->setChecked(false);
    else
        ui->xMLCcheckBox->setChecked(true);
}


void MainConsole::on_yMLCcheckBox_clicked()
{
    if(ui->yMLCcheckBox->isChecked())
        ui->yMLCcheckBox->setChecked(false);
    else
        ui->yMLCcheckBox->setChecked(true);
}


void MainConsole::on_saveButton_clicked()
{
    this->checkModified();
}

void MainConsole::on_cancelButton_clicked()
{
    this->Cancel();
}

void MainConsole::on_resetButton_clicked()
{
    this->resetMachine();
}

void MainConsole::on_oMLCcomboBox_activated(const QString &arg1)
{
    this->setFocus();
}

void MainConsole::on_pushButtonDone_clicked()
{
    this->done();
}

void MainConsole::on_pushButtonCancel_clicked()
{
    this->cancel();
}

void MainConsole::on_comboBoxModel_activated(const QString &arg1)
{
    this->setModelFile();
}

void MainConsole::on_comboBoxVendor_activated(const QString &arg1)
{
    this->getModels();
}

void MainConsole::on_comboBoxMachine_activated(const QString &arg1)
{
    this->machineChange();
}

void MainConsole::on_pushButtonPBReset_clicked()
{
    this->resetSmooth();
}

void MainConsole::on_pushButtonTele_clicked()
{
    this->getTeleDir();
}

void MainConsole::on_pushButtonLocal_clicked()
{
    this->getLocalDir();
}

void MainConsole::on_pushButtonPBPack_clicked()
{
    this->PBPack();
}

void MainConsole::on_pushButtonVerificationReview2_clicked()
{
    this->plotVerification();
}

void MainConsole::on_pushButtonWfitReview2_clicked()
{
    this->runWaterFit();
}

void MainConsole::on_pushButtonPBReviewPBMC2_clicked()
{
    this->plotPBMCComparison();
}

void MainConsole::on_pushButtonPBReviewPBVer2_clicked()
{
    this->plotPBVerification();
}

void MainConsole::on_pushButtonAirReview2_clicked()
{
    this->runAirFitNew();
}

void MainConsole::on_pushButtonPBMCReview_clicked()
{
    this->plotPBvsPBMC();
}

void MainConsole::on_pushButtonPBinit_clicked()
{
    this->initSmooth();
}

void MainConsole::on_pushButtonPBVerify_clicked()
{
    this->plotVerification();
}

void MainConsole::on_pushButtonPBReview_clicked()
{
    this->plotPBComm();
}

void MainConsole::on_pushButtonPBReviewPBVer_clicked()
{
    this->plotPBVerification();
}

void MainConsole::on_pushButtonPBReviewPBMC_clicked()
{
    this->plotPBMCComparison();
}

void MainConsole::on_pushButtonPBComm_clicked()
{
    this->runPBComm();
}

void MainConsole::on_pushButtonPBReviewPara_clicked()
{
    this->plotPBCommPara();
}

void MainConsole::on_pushButtonPBMCver_clicked()
{
    this->runPBMCverification();
}

void MainConsole::on_pushButtonFITReset_clicked()
{
    this->resetFit();
}

void MainConsole::on_pushButtonFITinit_clicked()
{
    this->initFit();
}

void MainConsole::on_comboBoxSSD_activated(const QString &arg1)
{
    this->updatePBCommValue();
}

void MainConsole::on_pushButtonPBMC_clicked()
{
    this->runPBMC();
}

void MainConsole::on_pushButtonBDTreset_clicked()
{
    this->resetBDT();
}

void MainConsole::on_pushButtonVerification_clicked()
{
    this->runVerification();
}

void MainConsole::on_pushButtonVerificationReview_clicked()
{
    this->plotVerification();
}

void MainConsole::on_pushButtonWfit_clicked()
{
    this->runWaterFit();
}

void MainConsole::on_pushButtonWfitReview_clicked()
{
    this->runWaterFit();
}

void MainConsole::on_pushButtonMonoStart_clicked()
{
    this->runMonoMC();
}

void MainConsole::on_pushButtonAirFit_clicked()
{
    this->runAirFitNew();
}

void MainConsole::on_pushButtonAirReview_clicked()
{
    this->runAirFitNew();
}

void MainConsole::on_pushButtonCopyNew_clicked()
{
    this->copyMachine();
}

void MainConsole::on_pushButtonAdjustNorm_clicked()
{
    this->adjustNorm();
}

void MainConsole::on_pushButtonResetAFIT_clicked()
{
    this->resetAFIT();
}

void MainConsole::on_pushButtonResetWFIT_clicked()
{
    this->resetWFIT();
}

void MainConsole::on_pushButtonResetMono_clicked()
{
    this->resetMonoMC();
}

void MainConsole::on_pushButtonResetValidation_clicked()
{
    this->resetValidate();
}

void MainConsole::on_pushButtonResetMC_clicked()
{
    this->resetVerification();
}

void MainConsole::on_pushButtonResetPBMC_clicked()
{
    this->resetPBMC();
}

void MainConsole::on_comboBoxOrigMachine_activated(const QString &arg1)
{
    this->setCopyMachineName();
}

void MainConsole::on_checkBoxOFAdjust_clicked()
{
    this->writeOFAdjust();
}
