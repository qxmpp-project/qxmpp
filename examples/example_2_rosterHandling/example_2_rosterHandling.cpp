// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "example_2_rosterHandling.h"

#include "QXmppMessage.h"
#include "QXmppRosterManager.h"

#include <QCoreApplication>

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent),
      m_rosterManager(findExtension<QXmppRosterManager>())
{
    connect(this, &QXmppClient::connected,
            this, &xmppClient::clientConnected);

    connect(m_rosterManager, &QXmppRosterManager::rosterReceived,
            this, &xmppClient::rosterReceived);

    /// Then QXmppRoster::presenceChanged() is emitted whenever presence of
    /// someone in roster changes
    connect(m_rosterManager, &QXmppRosterManager::presenceChanged,
            this, &xmppClient::presenceChanged);
}

xmppClient::~xmppClient() = default;

void xmppClient::clientConnected()
{
    qDebug("example_2_rosterHandling:: CONNECTED");
}

void xmppClient::rosterReceived()
{
    qDebug("example_2_rosterHandling:: Roster received");
    const QStringList jids = m_rosterManager->getRosterBareJids();
    for (const QString &bareJid : jids) {
        QString name = m_rosterManager->getRosterEntry(bareJid).name();
        if (name.isEmpty())
            name = "-";
        qDebug("example_2_rosterHandling:: Roster received: %s [%s]", qPrintable(bareJid), qPrintable(name));
    }
}

void xmppClient::presenceChanged(const QString &bareJid,
                                 const QString &resource)
{
    qDebug("example_2_rosterHandling:: Presence changed %s/%s", qPrintable(bareJid), qPrintable(resource));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    xmppClient client;
    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return app.exec();
}
