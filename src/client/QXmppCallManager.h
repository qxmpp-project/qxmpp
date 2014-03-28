/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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

#ifndef QXMPPCALLMANAGER_H
#define QXMPPCALLMANAGER_H

#include <QObject>
#include <QIODevice>
#include <QMetaType>

#include "QXmppClientExtension.h"
#include "QXmppLogger.h"

class QHostAddress;
class QXmppCallPrivate;
class QXmppCallManager;
class QXmppCallManagerPrivate;
class QXmppIq;
class QXmppJingleCandidate;
class QXmppJingleIq;
class QXmppJinglePayloadType;
class QXmppPresence;
class QXmppRtpAudioChannel;
class QXmppRtpVideoChannel;

/// \brief The QXmppCall class represents a Voice-Over-IP call to a remote party.
///
/// To get the QIODevice from which you can read / write audio samples, call
/// audioChannel().
///
/// \note THIS API IS NOT FINALIZED YET

class QXMPP_EXPORT QXmppCall : public QXmppLoggable
{
    Q_OBJECT
    Q_ENUMS(Direction State)
    Q_FLAGS(QIODevice::OpenModeFlag QIODevice::OpenMode)
    Q_PROPERTY(Direction direction READ direction CONSTANT)
    Q_PROPERTY(QString jid READ jid CONSTANT)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QIODevice::OpenMode audioMode READ audioMode NOTIFY audioModeChanged)
    Q_PROPERTY(QIODevice::OpenMode videoMode READ videoMode NOTIFY videoModeChanged)

public:
    /// This enum is used to describe the direction of a call.
    enum Direction
    {
        IncomingDirection, ///< The call is incoming.
        OutgoingDirection, ///< The call is outgoing.
    };

    /// This enum is used to describe the state of a call.
    enum State
    {
        ConnectingState = 0,    ///< The call is being connected.
        ActiveState = 1,        ///< The call is active.
        DisconnectingState = 2, ///< The call is being disconnected.
        FinishedState = 3,      ///< The call is finished.
    };

    ~QXmppCall();

    QXmppCall::Direction direction() const;
    QString jid() const;
    QString sid() const;
    QXmppCall::State state() const;

    QXmppRtpAudioChannel *audioChannel() const;
    QIODevice::OpenMode audioMode() const;
    QXmppRtpVideoChannel *videoChannel() const;
    QIODevice::OpenMode videoMode() const;

signals:
    /// \brief This signal is emitted when a call is connected.
    ///
    /// Once this signal is emitted, you can connect a QAudioOutput and
    /// QAudioInput to the call. You can determine the appropriate clockrate
    /// and the number of channels by calling payloadType().
    void connected();

    /// \brief This signal is emitted when a call is finished.
    ///
    /// Note: Do not delete the call in the slot connected to this signal,
    /// instead use deleteLater().
    void finished();

    /// \brief This signal is emitted when the remote party is ringing.
    void ringing();

    /// \brief This signal is emitted when the call state changes.
    void stateChanged(QXmppCall::State state);

    /// \brief This signal is emitted when the audio channel changes.
    void audioModeChanged(QIODevice::OpenMode mode);

    /// \brief This signal is emitted when the video channel changes.
    void videoModeChanged(QIODevice::OpenMode mode);

public slots:
    void accept();
    void hangup();
    void startVideo();
    void stopVideo();

private slots:
    void localCandidatesChanged();
    void terminated();
    void updateOpenMode();

private:
    QXmppCall(const QString &jid, QXmppCall::Direction direction, QXmppCallManager *parent);

    QXmppCallPrivate *d;
    friend class QXmppCallManager;
    friend class QXmppCallManagerPrivate;
    friend class QXmppCallPrivate;
};

/// \brief The QXmppCallManager class provides support for making and
/// receiving voice calls.
///
/// Session initiation is performed as described by XEP-0166: Jingle,
/// XEP-0167: Jingle RTP Sessions and XEP-0176: Jingle ICE-UDP Transport
/// Method.
///
/// The data stream is connected using Interactive Connectivity Establishment
/// (RFC 5245) and data is transferred using Real Time Protocol (RFC 3550)
/// packets.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// QXmppCallManager *manager = new QXmppCallManager;
/// client->addExtension(manager);
/// \endcode
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppCallManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppCallManager();
    ~QXmppCallManager();
    void setStunServer(const QHostAddress &host, quint16 port = 3478);
    void setTurnServer(const QHostAddress &host, quint16 port = 3478);
    void setTurnUser(const QString &user);
    void setTurnPassword(const QString &password);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when a new incoming call is received.
    ///
    /// To accept the call, invoke the call's QXmppCall::accept() method.
    /// To refuse the call, invoke the call's QXmppCall::hangup() method.
    void callReceived(QXmppCall *call);

    /// This signal is emitted when a call (incoming or outgoing) is started.
    void callStarted(QXmppCall *call);

public slots:
    QXmppCall *call(const QString &jid);

protected:
    /// \cond
    void setClient(QXmppClient* client);
    /// \endcond

private slots:
    void _q_callDestroyed(QObject *object);
    void _q_disconnected();
    void _q_iqReceived(const QXmppIq &iq);
    void _q_jingleIqReceived(const QXmppJingleIq &iq);
    void _q_presenceReceived(const QXmppPresence &presence);

private:
    QXmppCallManagerPrivate *d;
    friend class QXmppCall;
    friend class QXmppCallPrivate;
    friend class QXmppCallManagerPrivate;
};

Q_DECLARE_METATYPE(QXmppCall::State)

#endif
