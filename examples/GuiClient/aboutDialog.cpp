#include "aboutDialog.h"
#include "ui_aboutDialog.h"

#include "QXmppGlobal.h"

aboutDialog::aboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("About %1").arg(qApp->applicationName()));

    ui->textEdit->append(QString("Copyright © 2009-2010, Manjeet Dahiya"));
    ui->textEdit->append(qApp->applicationName() + " " + qApp->applicationVersion());
    ui->textEdit->append(QString("Based on QXmpp v %1").arg(QXmppVersion()));
}

aboutDialog::~aboutDialog()
{
    delete ui;
}
