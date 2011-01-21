/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

#ifndef QXMPPROSTER_H
#define QXMPPROSTER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QStringList>

#include "QXmppClientExtension.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"

class QXmppRosterIq;

/// \brief The QXmppRosterManager class provides access to a connected client's roster.
///
/// \note It's object should not be created using it's constructor. Instead
/// QXmppClient::rosterManager() should be used to get the reference of instantiated
/// object this class.
///
/// It stores all the Roster and Presence details of all the roster entries (that
/// is all the bareJids) in the client's friend's list. It provides the
/// functionality to get all the bareJids in the client's roster and Roster and
/// Presence details of the same.
///
/// After the successful xmpp connection that after the signal QXmppClient::connected()
/// is emitted QXmpp requests for getting the roster. Once QXmpp receives the roster
/// the signal QXmppRosterManager::rosterReceived() is emitted and after that user can
/// use the functions of this class to get roster entries.
///
/// Function QXmppRosterManager::isRosterReceived() tells whether the roster has been
/// received or not.
///
/// Signals presenceChanged() or rosterChanged() are emitted whenever presence
/// or roster changes respectively.
///
/// \ingroup Managers

class QXmppRosterManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppRosterManager(QXmppClient* stream);
    
    bool isRosterReceived();
    QStringList getRosterBareJids() const;
    QXmppRosterIq::Item getRosterEntry(const QString& bareJid) const;
    void removeRosterEntry(const QString &bareJid);
    
    QStringList getResources(const QString& bareJid) const;
    QMap<QString, QXmppPresence> getAllPresencesForBareJid(
            const QString& bareJid) const;
    QXmppPresence getPresence(const QString& bareJid,
                              const QString& resource) const;

    /// \cond
    bool handleStanza(const QDomElement &element);
    /// \endcond

    // deprecated in release 0.2.0
    /// \cond
    QMap<QString, QXmppRosterIq::Item> Q_DECL_DEPRECATED getRosterEntries() const;
    QMap<QString, QMap<QString, QXmppPresence> > Q_DECL_DEPRECATED getAllPresences() const;
    /// \endcond

signals:
    /// This signal is emitted when the Roster IQ is received after a successful
    /// connection. That is the roster entries are empty before this signal is emitted.
    /// One should use getRosterBareJids() and getRosterEntry() only after 
    /// this signal has been emitted.
    void rosterReceived();

    /// This signal is emitted when the presence of a particular bareJid and resource changes.
    void presenceChanged(const QString& bareJid, const QString& resource);

    /// This signal is emitted when the roster entry of a particular bareJid changes.
    void rosterChanged(const QString& bareJid);

private:
    //map of bareJid and its rosterEntry
    QMap<QString, QXmppRosterIq::Item> m_entries;
    // map of resources of the jid and map of resources and presences
    QMap<QString, QMap<QString, QXmppPresence> > m_presences;
    // flag to store that the roster has been populated
    bool m_isRosterReceived;
    // id of the initial roster request
    QString m_rosterReqId;

private slots:
    void connected();
    void disconnected();
    void presenceReceived(const QXmppPresence&);

private:
    void rosterIqReceived(const QXmppRosterIq&);
};

#endif // QXMPPROSTER_H
