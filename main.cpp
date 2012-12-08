#include <QApplication>
#include "FenPrincipale.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow fenetre;
    fenetre.show();

    return app.exec();
}

