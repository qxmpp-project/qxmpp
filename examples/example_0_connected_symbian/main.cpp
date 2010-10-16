/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Authors:
 *	Manjeet Dahiya
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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


#include <QApplication>
#include <QDialogButtonBox>
#include <QLayout>
#include <QTextBrowser>

#include "QXmppClient.h"
#include "QXmppLogger.h"
#include "main.h"

LogViewer::LogViewer()
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);

    m_browser = new QTextBrowser;
    vbox->addWidget(m_browser);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttons, SIGNAL(accepted()), qApp, SLOT(quit()));
    vbox->addWidget(buttons);

    setLayout(vbox);
}

void LogViewer::log(QXmppLogger::MessageType type, const QString& msg)
{
    Q_UNUSED(type);
    m_browser->append(msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
  
    // Set up logging 
    QXmppLogger *logger = QXmppLogger::getLogger();
    logger->setLoggingType(QXmppLogger::SignalLogging);

    LogViewer viewer;
    QObject::connect(logger, SIGNAL(message(QXmppLogger::MessageType,QString)),
            &viewer, SLOT(log(QXmppLogger::MessageType,QString)));
    viewer.show();

    // Connect to server
    QXmppClient client;
    // client.connectToServer("username@jabber.org", "passwd");
    client.connectToServer("qxmpp.test1@gmail.com", "qxmpp123");

    return a.exec();
}
