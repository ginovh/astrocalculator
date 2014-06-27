#include <QApplication>
#include <QDialog>

#include "astrocalculator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AstroCalculator *dialog = new AstroCalculator;
    dialog->show();

    return app.exec();
}
