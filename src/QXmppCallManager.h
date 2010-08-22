/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#ifndef QXMPPCALLMANAGER_H
#define QXMPPCALLMANAGER_H

#include <QObject>
#include <QIODevice>

#include "QXmppJingleIq.h"
#include "QXmppLogger.h"

class QXmppCodec;
class QXmppIceConnection;
class QXmppJingleCandidate;
class QXmppJingleIq;
class QXmppJinglePayloadType;
class QXmppStream;

/// \brief The QXmppCall class represents a Voice-Over-IP call to a remote party.
///
/// It acts as a QIODevice so that you can read / write audio samples, for
/// instance using a QAudioOutput and a QAudioInput.
///
/// \note THIS API IS NOT FINALIZED YET

class QXmppCall : public QIODevice
{
    Q_OBJECT

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
        OfferState = 0,         ///< The remote part is being called.
        ConnectingState = 1,    ///< The call is being connected.
        ActiveState = 2,        ///< The call is active.
        DisconnectingState = 3, ///< The call is being disconnected.
        FinishedState = 4,      ///< The call is finished.
    };

    QXmppCall::Direction direction() const;
    QString jid() const;
    QString sid() const;
    QXmppCall::State state() const;

    QXmppJinglePayloadType payloadType() const;

    /// \cond
    qint64 bytesAvailable() const;
    bool isSequential() const;
    /// \endcond

signals:
    /// This signal is emitted when a call is connected.
    ///
    /// Once this signal is emitted, you can connect a QAudioOutput and
    /// QAudioInput to the call. You can determine the appropriate clockrate
    /// and the number of channels by calling payloadType().
    void connected();

    /// This signal is emitted when a call is finished.
    ///
    /// Note: Do not delete the call in the slot connected to this signal,
    /// instead use deleteLater().
    void finished();

    /// \internal
    void localCandidatesChanged();

    /// This signal is emitted when the remote party is ringing.
    void ringing();

    /// This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

    /// This signal is emitted when the call state changes.
    void stateChanged(QXmppCall::State state);

public slots:
    void accept();
    void hangup();

private slots:
    void datagramReceived(int component, const QByteArray &datagram);
    void emitSignals();
    void updateOpenMode();
    void terminate();
    void terminated();

protected:
    /// \cond
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    /// \endcond

private:
    QXmppCall(const QString &jid, QXmppCall::Direction direction, QObject *parent);
    void setPayloadType(const QXmppJinglePayloadType &type);
    void addRemoteCandidates(const QList<QXmppJingleCandidate> &candidates);
    void setState(QXmppCall::State state);

    Direction m_direction;
    QString m_jid;
    QString m_sid;
    State m_state;
    QString m_contentCreator;
    QString m_contentName;
    QXmppJinglePayloadType m_payloadType;

    QList<QXmppJingleIq> m_requests;
    QList<QXmppJinglePayloadType> m_commonPayloadTypes;

    // ICE-UDP
    QXmppIceConnection *m_connection;

    // signals
    bool m_signalsEmitted;
    qint64 m_writtenSinceLastEmit;

    // RTP
    QXmppCodec *m_codec;

    QByteArray m_incomingBuffer;
    bool m_incomingBuffering;
    int m_incomingMinimum;
    quint16 m_incomingSequence;
    quint32 m_incomingStamp;

    quint16 m_outgoingChunk;
    QByteArray m_outgoingBuffer;
    bool m_outgoingMarker;
    quint16 m_outgoingSequence;
    quint32 m_outgoingStamp;

    friend class QXmppCallManager;
};

/// \brief The QXmppCallManager class provides support for making and
/// receiving voice calls.
///
/// Session initiation is performed as described by XEP-0166: Jingle,
/// XEP-0167: Jingle RTP Sessions and XEP-0176: Jingle ICE-UDP Transport
/// Method.
///
/// The data stream is connected using Interactive Connectivity Establishment
/// (RFC 5245) and data is transfered using Real Time Protocol (RFC 3550)
/// packets.
///
/// \ingroup Managers

class QXmppCallManager : public QObject
{
    Q_OBJECT

public:
    QXmppCallManager(QXmppStream *stream, QObject *parent = 0);
    QXmppCall *call(const QString &jid);

signals:
    /// This signal is emitted when a new incoming call is received.
    ///
    /// To accept the call, invoke the call's QXmppCall::accept() method.
    /// To refuse the call, invoke the call's QXmppCall::abort() method.
    void callReceived(QXmppCall *call);

    /// This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

private slots:
    void callDestroyed(QObject *object);
    void callStateChanged(QXmppCall::State state);
    void iqReceived(const QXmppIq &iq);
    void jingleIqReceived(const QXmppJingleIq &iq);
    void localCandidatesChanged();

private:
    bool checkPayloadTypes(QXmppCall *call, const QList<QXmppJinglePayloadType> &remotePayloadTypes);
    QXmppCall *findCall(const QString &sid) const;
    QXmppCall *findCall(const QString &sid, QXmppCall::Direction direction) const;
    QList<QXmppJinglePayloadType> localPayloadTypes() const;
    bool sendAck(const QXmppJingleIq &iq);
    bool sendRequest(QXmppCall *call, const QXmppJingleIq &iq);

    QList<QXmppCall*> m_calls;

    // reference to xmpp stream (no ownership)
    QXmppStream* m_stream;
};

#endif
