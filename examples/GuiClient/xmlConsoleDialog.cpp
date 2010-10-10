#include "xmlConsoleDialog.h"
#include "ui_xmlConsoleDialog.h"

#include <QDomDocument>
#include <QTextStream>

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
    QDomDocument doc;

// Indent XML string
    bool isXml = doc.setContent(text);
    QString formattedText;
    QTextStream stream(&formattedText);
    doc.save(stream, 2);

    if(isXml)
        ui->textBrowser->append(formattedText);
    else
        ui->textBrowser->append(text);
}
