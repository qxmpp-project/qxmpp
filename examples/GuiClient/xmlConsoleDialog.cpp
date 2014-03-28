/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


#include "xmlConsoleDialog.h"
#include "ui_xmlConsoleDialog.h"

#include <QDomDocument>
#include <QTextStream>

static QString s_colorHexSent("#02aa3f");
static QString s_colorHexReceived("#aa0000");

xmlConsoleDialog::xmlConsoleDialog(QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::xmlConsoleDialog)
{
    ui->setupUi(this);
    setWindowTitle("Debugging Console");

    ui->label_legend->setText(
            QString("<html><body><p><span style=\"color:%1\">Sent</span><span> | </span><span style=\"color:%2\">Received</span></p></body></html>").arg(s_colorHexSent).arg(s_colorHexReceived));
}

xmlConsoleDialog::~xmlConsoleDialog()
{
    delete ui;
}

void xmlConsoleDialog::message(QXmppLogger::MessageType type, const QString& text)
{
    if(!ui->checkBox_enable->isChecked())
        return;

    QColor color;
    switch(type)
    {
    case QXmppLogger::ReceivedMessage:
        color = QColor(s_colorHexReceived);
        break;
    case QXmppLogger::SentMessage:
        color = QColor(s_colorHexSent);
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
