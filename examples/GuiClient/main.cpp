#include <QtGui/QApplication>
#include <QDir>
#include "chatDialog.h"
#include "chatGraphicsView.h"
#include "chatGraphicsScene.h"
#include "mainDialog.h"
#include "statusTextWidget.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    QDir dir;
    if(!dir.exists(getSettingsDir()))
        dir.mkpath(getSettingsDir());

    QApplication a(argc, argv);
    mainDialog cw;
    cw.show();
    return a.exec();
}
