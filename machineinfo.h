#ifndef MACHINEINFO_H
#define MACHINEINFO_H

#include <QDialog>

#include "mInfo.h"

namespace Ui {
class MachineInfo;
}

class MachineInfo : public QDialog
{
    Q_OBJECT

public:
    explicit MachineInfo(QWidget *parent = 0);
    ~MachineInfo();

    void init();

    QString mm2cm(QString mm);

public slots:
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

private:
    Ui::MachineInfo *ui;
};


#endif // MACHINEINFO_H
