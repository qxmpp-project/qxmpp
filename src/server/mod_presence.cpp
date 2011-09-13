/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppPresence.h"
#include "QXmppServer.h"
#include "QXmppServerPlugin.h"
#include "QXmppStream.h"
#include "QXmppUtils.h"

#include "mod_presence.h"

class QXmppServerPresencePrivate
{
public:
    QXmppServerPresencePrivate(QXmppServerPresence *qq);

    QSet<QString> collectSubscribers(const QString &jid);
    QSet<QString> collectSubscriptions(const QString &jid);

    QHash<QString, QHash<QString, QXmppPresence> > presences;
    QHash<QString, QSet<QString> > subscribers;

private:
    QXmppServerPresence *q;
};

QXmppServerPresencePrivate::QXmppServerPresencePrivate(QXmppServerPresence *qq)
    : q(qq)
{
}

/// Collect subscribers from the extensions.
///
/// \param jid

QSet<QString> QXmppServerPresencePrivate::collectSubscribers(const QString &jid)
{
    QSet<QString> recipients;
    foreach (QXmppServerExtension *extension, q->server()->extensions())
        recipients += extension->presenceSubscribers(jid);
    return recipients;
}

/// Collect subscriptions from the extensions.
///
/// \param jid

QSet<QString> QXmppServerPresencePrivate::collectSubscriptions(const QString &jid)
{
    QSet<QString> recipients;
    foreach (QXmppServerExtension *extension, q->server()->extensions())
        recipients += extension->presenceSubscriptions(jid);
    return recipients;
}

QXmppServerPresence::QXmppServerPresence()
{
    d = new QXmppServerPresencePrivate(this);
}

QXmppServerPresence::~QXmppServerPresence()
{
    delete d;
}

/// Returns the list of available resources for the given local JID.
///
/// \param bareJid

QList<QXmppPresence> QXmppServerPresence::availablePresences(const QString &bareJid) const
{
    return d->presences.value(bareJid).values();
}

bool QXmppServerPresence::handleStanza(const QDomElement &element)
{
    if (element.tagName() != QLatin1String("presence"))
        return false;

    const QString domain = server()->domain();
    const QString from = element.attribute("from");
    const QString type = element.attribute("type");
    const QString to = element.attribute("to");

    if (to == domain) {
        // presence to the local domain

        // we only want available or unavailable presences from local users
        if ((!type.isEmpty() && type != QLatin1String("unavailable"))
            || (jidToDomain(from) != domain))
            return true;

        const QString bareFrom = jidToBareJid(from);
        bool isInitial = false;

        if (type.isEmpty()) {
            QXmppPresence presence;
            presence.parse(element);

            // record the presence for future use
            isInitial = !d->presences.value(bareFrom).contains(from);
            d->presences[bareFrom][from] = presence;
        } else {
            d->presences[bareFrom].remove(from);
            if (d->presences[bareFrom].isEmpty())
                d->presences.remove(bareFrom);
        }

        // broadcast it to subscribers
        foreach (const QString &subscriber, d->collectSubscribers(from)) {
            // avoid loop
            if (subscriber == to)
                continue;
            QDomElement changed = element.cloneNode(true).toElement();
            changed.setAttribute("to", subscriber);
            server()->handleElement(changed);
        }

        // get presences from subscriptions
        if (isInitial) {
            foreach (const QString &subscription, d->collectSubscriptions(from)) {
                if (jidToDomain(subscription) != domain) {
                    QXmppPresence probe;
                    probe.setType(QXmppPresence::Probe);
                    probe.setFrom(from);
                    probe.setTo(subscription);
                    server()->sendPacket(probe);
                } else {
                    QXmppPresence push;
                    foreach (push, availablePresences(subscription)) {
                        push.setTo(from);
                        server()->sendPacket(push);
                    }
                }
            }
        }

        // the presence was for us, stop here
        return true;
    } else {
        // directed presence
        if ((type.isEmpty() || type == QLatin1String("unavailable")) && jidToDomain(from) == domain) {
            // available or unavailable presence from local user
            if (type.isEmpty())
                d->subscribers[from].insert(to);
            else {
                d->subscribers[from].remove(to);
                if (d->subscribers[from].isEmpty())
                    d->subscribers.remove(from);
            }
        } else if (type == QLatin1String("error") && jidToDomain(to) == domain) {
            // error presence to a local user
            d->subscribers[to].remove(from);
            if (d->subscribers[to].isEmpty())
                d->subscribers.remove(to);
        }

        // the presence was not for us
        return false;
    }
}

QSet<QString> QXmppServerPresence::presenceSubscribers(const QString &jid)
{
    return d->subscribers.value(jid);
}

QXmppServerPresence* QXmppServerPresence::instance(QXmppServer *server)
{
    foreach (QXmppServerExtension *extension, server->extensions()) {
        QXmppServerPresence *presenceExtension = qobject_cast<QXmppServerPresence*>(extension);
        if (presenceExtension)
            return presenceExtension;
    }
    return 0;
}

bool QXmppServerPresence::start()
{
    bool check;
    Q_UNUSED(check);

    check = connect(server(), SIGNAL(clientDisconnected(QString)),
                    this, SLOT(_q_clientDisconnected(QString)));
    Q_ASSERT(check);

    return true;
}

void QXmppServerPresence::stop()
{
    disconnect(server(), SIGNAL(clientDisconnected(QString)),
               this, SLOT(_q_clientDisconnected(QString)));
}

void QXmppServerPresence::_q_clientDisconnected(const QString &jid)
{
    Q_ASSERT(!jid.isEmpty());

    // check the user exited cleanly
    const bool hadPresence = d->presences.value(jidToBareJid(jid)).contains(jid);
    if (hadPresence) {
        // the client had sent an initial available presence but did
        // not sent an unavailable presence, synthesize it
        QDomDocument doc;
        QDomElement presence = doc.createElement("presence");
        presence.setAttribute("from", jid);
        presence.setAttribute("type", "unavailable");
        presence.setAttribute("to", server()->domain());
        server()->handleElement(presence);
    } else {
        // synthesize unavailable presence to directed presence receivers
        foreach (const QString &recipient, presenceSubscribers(jid)) {
            QDomDocument doc;
            QDomElement presence = doc.createElement("presence");
            presence.setAttribute("from", jid);
            presence.setAttribute("type", "unavailable");
            presence.setAttribute("to", recipient);
            server()->handleElement(presence);
        }
    }
}

// PLUGIN

class QXmppServerPresencePlugin : public QXmppServerPlugin
{
public:
    QXmppServerExtension *create(const QString &key)
    {
        if (key == QLatin1String("presence"))
            return new QXmppServerPresence;
        else
            return 0;
    };

    QStringList keys() const
    {
        return QStringList() << QLatin1String("presence");
    };
};

Q_EXPORT_STATIC_PLUGIN2(mod_presence, QXmppServerPresencePlugin)

