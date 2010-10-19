#include "aboutDialog.h"
#include "ui_aboutDialog.h"

#include "QXmppGlobal.h"

#include <QtGlobal>

aboutDialog::aboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("About %1").arg(qApp->applicationName()));

    ui->textEdit->append(QString("Copyright © 2009-2010, Manjeet Dahiya\n"));
    ui->textEdit->append(qApp->applicationName() + " " + qApp->applicationVersion());
    ui->textEdit->append(QString("\nBased on:"));
    ui->textEdit->append(QString("QXmpp %1").arg(QXmppVersion()));
    ui->textEdit->append(QString("Qt %1 [built-with]").arg(qVersion()));
    ui->textEdit->append(QString("Qt %1 [running-with]").arg(QT_VERSION_STR));
}

aboutDialog::~aboutDialog()
{
    delete ui;
}
