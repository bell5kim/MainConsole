#include "mainconsole.h"
#include <QApplication>

#include <cmath>
#include <iostream>
#include <cstdio>
using namespace std;

void usage() {
  printf("\nUSAGE: %s -i gname\n", "mainconsole.exe");
  printf("\n");
}

int main(int argc, char *argv[])
{
    cout << "Test" << endl;
    char *gName="";

    for(int iArg=0; iArg < argc; iArg++){
        if(iArg < argc-1){
        if( strcmp(argv[iArg],"-i") == 0 || strcmp(argv[iArg],"-gname") == 0) {
                iArg++;
                gName = argv[iArg];
        }
        }
        if(strcmp("-help", argv[iArg]) == 0 || strcmp("-h", argv[iArg]) == 0 ) {
         usage();
         return(0);
        }
    }


    QApplication app(argc, argv);
    MainConsole *mConsole = new MainConsole;

    mConsole->show();

    // app->setCentralWidget(mConsole);

    return app.exec();
}
