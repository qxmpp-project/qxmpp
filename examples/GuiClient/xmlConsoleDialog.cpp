#include "xmlConsoleDialog.h"
#include "ui_xmlConsoleDialog.h"

xmlConsoleDialog::xmlConsoleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::xmlConsoleDialog)
{
    ui->setupUi(this);
}

xmlConsoleDialog::~xmlConsoleDialog()
{
    delete ui;
}

void xmlConsoleDialog::message(QXmppLogger::MessageType type, const QString& text)
{
    ui->textBrowser->append(text);
    ui->textBrowser->append("\n");
}
