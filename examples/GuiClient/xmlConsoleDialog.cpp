#include "xmlConsoleDialog.h"
#include "ui_xmlConsoleDialog.h"

#include <QDomDocument>
#include <QTextStream>

xmlConsoleDialog::xmlConsoleDialog(QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::xmlConsoleDialog)
{
    ui->setupUi(this);
    setWindowTitle("Debugging Console");
}

xmlConsoleDialog::~xmlConsoleDialog()
{
    delete ui;
}

void xmlConsoleDialog::message(QXmppLogger::MessageType type, const QString& text)
{
    QColor color;
    switch(type)
    {
    case QXmppLogger::ReceivedMessage:
        color = QColor("#aa0000");
        break;
    case QXmppLogger::SentMessage:
        color = QColor("#02aa3f");
        break;
    default:
        return;
    }

    QDomDocument doc;

// Indent XML string
    bool isXml = doc.setContent(text);
    QString formattedText;
    QTextStream stream(&formattedText);
    doc.save(stream, 2);

    ui->textBrowser->setTextColor(color);
    if(isXml)
        ui->textBrowser->append(formattedText);
    else
        ui->textBrowser->append(text);
}
