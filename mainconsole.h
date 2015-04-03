#ifndef MAINCONSOLE_H
#define MAINCONSOLE_H

#include <QMainWindow>

#include "mInfo.h"

namespace Ui {
class MainConsole;
}

// class MachinInfo;

class MainConsole : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainConsole(QWidget *parent = 0);
    ~MainConsole();

    // QString mm2cm(QString mm);

    virtual QString readLocalSetting( QString group, QString keyWord );
    virtual QString pickDir( QString dirFrom, QString messageText );
    virtual QString mm2cm( QString mm );
    virtual void readParm();
    virtual bool isThereFile( QString dirStr, QString fileStr );
    virtual bool backupDir( QString dirName );
    virtual bool checkMonoMC();
    virtual bool checkPBMC();
    virtual QString getKeyValue( QString strLine );
    virtual void getPRLists();
    virtual QString getWaterData( QString PT_NAME );
    virtual QString getWaterDataVMC();
    virtual QString getOutputFactor( QString inputFile, int iOpt, float SAD );
    virtual float getPDDmax( QString iFile, float xOffset );
    virtual QString getPDD( QString inputFile, QString outputFile, float xp, float factor, float xOffset, int iOpt );
    virtual QString getOCRR( QString inputFile, QString outputFile, float xp, float factor, float xOffset, int iOpt );
    virtual float getFWHM( QString inputFile, float xOffset );
    virtual float pickOnePoint2Colums( QString inputFile, float & xPoint );
    virtual QString readVER( QString FNAME, QString OPT );
    virtual QString readLogfile( QString logFile, QString keyWord );
    virtual int iCopyMachine();

public slots:
    virtual void initLocalSettings( QString group );
    virtual void writeLocalSetting( QString group, QString keyWord, QString keyValue );
    virtual void updateLocalSettingRemoved( QString group );
    virtual void writeSettings( QString group );
    virtual void readSettings( QString group );
    virtual void getTeleDir();
    virtual void getLocalDir();
    virtual void getMCMachine();
    virtual void machineChange();
    virtual void getVendors();
    virtual void getModels();
    virtual void setModelFile();
    virtual void done();
    virtual void cancel();
    virtual void machineInfoView();
    virtual void updateButtons();
    virtual void checkMachineModifiedRemoved();
    virtual bool isThereDir( QString dirStr );
    virtual void backupAfitDir();
    virtual void makeDirOld( QString dirStr );
    virtual void makeDir( QString dirStr );
    virtual void runAirFitNew();
    virtual void plotAirFit();
    virtual void makeMonoVMC( QString plan );
    virtual void writeRefFS();
    virtual void runMonoMC();
    virtual void runMonoXVMC();
    virtual void runMonoVerify();
    virtual void runMonoVerify( QString FS, QString FScm );
    virtual void getMachineInfo( QString modelFile );
    virtual void get10x10PDD();
    virtual void runWaterFit();
    virtual void runVerification();
    virtual void plotVerification();
    virtual void runPBVerification();
    virtual void plotPBVerification();
    virtual void plotPBMCComparison();
    virtual void plotPBvsPBMC();
    virtual void runPBMCxvmc();
    virtual void runPBMC();
    virtual void runPBMCverification();
    virtual void makePBVMC();
    virtual void runPBCommOld();
    virtual void runPBComm();
    virtual void plotPBComm();
    virtual void plotPBCommPara();
    virtual void updateBDT14();
    virtual void updateBDT( QString MODEL_ID, QString ENERGY_MAX );
    virtual void runVerify();
    virtual void PBPack();
    virtual void getAirDataNew();
    virtual void normalize( float * x, float * y, int nPoints, float xp, float factor, float xOffset, int iOpt );
    virtual void writeVMC( QString FNAME );
    virtual void writeCLN( QString FNAME, QString pList );
    virtual void writePBCLN( QString FNAME, QString pList );
    virtual void writeVER( QString FNAME, QStringList xZLIST, QStringList yZLIST, QString OPTIONS );
    virtual void readSMOOTH( QString FNAME );
    virtual void writeSMOOTH( QString FNAME );
    virtual void readFIT( QString FNAME );
    virtual void writeFIT( QString FNAME );
    virtual void resetSmooth();
    virtual void resetFit();
    virtual void initSmooth();
    virtual void initFit();
    virtual void writeRSD();
    virtual void readRSD();
    virtual void writeNU();
    virtual void readNU();
    virtual void writeMFS();
    virtual void readMFS();
    virtual void writeOffset();
    virtual void readOffset();
    virtual void writeOFAdjust();
    virtual void readOFAdjust();
    virtual void writeAirOpt();
    virtual void readAirOpt();
    virtual void writeWaterOpt();
    virtual void readWaterOpt();
    virtual void writeOffAxisOpt();
    virtual void readOffAxisOpt();
    virtual void mySystem( QString CMD );
    virtual void mergePDDs( QString DIR, QString PLAN, int I );
    virtual void updateMachineInfo();
    virtual void readBDT( QString fName );
    virtual void writeBDT( QString bdtFile );
    virtual void readAirFitInp( QString fname );
    virtual void readWaterFitOut( QString fname );
    virtual void updatePBCommInfo();
    virtual void updatePBCommValue();
    virtual void updatePBInfo();
    virtual void updateBdtValues();
    virtual void clearBdtValues();
    virtual void updateBdtValuesAir();
    virtual void updateBdtValuesWater();
    virtual void resetBDT();
    virtual void updateAirFitValues();
    virtual void updateWaterFitValues();
    virtual void scalePDD( QString DIR, QString PLAN, float FACTOR );
    virtual void readGridSizeVer();
    virtual void resetPBComm();
    virtual void resetValidate();
    virtual void resetAFIT();
    virtual void resetMonoMC();
    virtual void resetWFIT();
    virtual void resetVerification();
    virtual void resetPBMC();
    virtual void setCopyMachineName();
    virtual void copyMachine();
    virtual void adjustNorm();

    virtual void clearModifiedAll();
    virtual void updateAll();
    virtual void setMDIR( QString usrText );
    virtual void setLHOME( QString usrText );
    virtual void setMODEL( QString usrText );
    virtual void setModelFile( QString usrText );
    virtual void setVendor( QString usrText );
    virtual void setTMPDIR( QString usrText );
    virtual void clearMachineInfo();
    virtual void readParm( QString mDir );
    virtual void readCollim( QString mDir );
    virtual void readMlc( QString mDir );
    virtual void getMachineInfo();
    virtual void resetMachine();
    virtual void getXiOInfo();
    virtual void getLocalInfo( QString modelFile );
    virtual void writeModified( QString group );
    virtual void writeNotModified( QString group );
    virtual void writeMachineInfo();
    virtual void checkModified();
    virtual void Cancel();

private slots:
    void on_xMLCcheckBox_clicked();

    void on_yMLCcheckBox_clicked();

    void on_saveButton_clicked();

    void on_cancelButton_clicked();

    void on_resetButton_clicked();

    void on_oMLCcomboBox_activated(const QString &arg1);

    void on_pushButtonDone_clicked();

    void on_pushButtonCancel_clicked();

    void on_comboBoxModel_activated(const QString &arg1);

    void on_comboBoxVendor_activated(const QString &arg1);

    void on_comboBoxMachine_activated(const QString &arg1);

    void on_pushButtonPBReset_clicked();

    void on_pushButtonTele_clicked();

    void on_pushButtonLocal_clicked();

    void on_pushButtonPBPack_clicked();

    void on_pushButtonVerificationReview2_clicked();

    void on_pushButtonWfitReview2_clicked();

    void on_pushButtonPBReviewPBMC2_clicked();

    void on_pushButtonPBReviewPBVer2_clicked();

    void on_pushButtonAirReview2_clicked();

    void on_pushButtonPBMCReview_clicked();

    void on_pushButtonPBinit_clicked();

    void on_pushButtonPBVerify_clicked();

    void on_pushButtonPBReview_clicked();

    void on_pushButtonPBReviewPBVer_clicked();

    void on_pushButtonPBReviewPBMC_clicked();

    void on_pushButtonPBComm_clicked();

    void on_pushButtonPBReviewPara_clicked();

    void on_pushButtonPBMCver_clicked();

    void on_pushButtonFITReset_clicked();

    void on_pushButtonFITinit_clicked();

    void on_comboBoxSSD_activated(const QString &arg1);

    void on_pushButtonPBMC_clicked();

    void on_pushButtonBDTreset_clicked();

    void on_pushButtonVerification_clicked();

    void on_pushButtonVerificationReview_clicked();

    void on_pushButtonWfit_clicked();

    void on_pushButtonWfitReview_clicked();

    void on_pushButtonMonoStart_clicked();

    void on_pushButtonAirFit_clicked();

    void on_pushButtonAirReview_clicked();

    void on_pushButtonCopyNew_clicked();

    void on_pushButtonAdjustNorm_clicked();

    void on_pushButtonResetAFIT_clicked();

    void on_pushButtonResetWFIT_clicked();

    void on_pushButtonResetMono_clicked();

    void on_pushButtonResetValidation_clicked();

    void on_pushButtonResetMC_clicked();

    void on_pushButtonResetPBMC_clicked();

    void on_comboBoxOrigMachine_activated(const QString &arg1);

    void on_checkBoxOFAdjust_clicked();

private:
    Ui::MainConsole *ui;

    QPixmap image0;
    QPixmap image1;

    void init();
    void initMachineInfo();
};

#endif // MAINCONSOLE_H
