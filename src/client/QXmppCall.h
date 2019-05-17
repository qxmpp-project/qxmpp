/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#ifndef QXMPPCALL_H
#define QXMPPCALL_H

#include "QXmppCallStream.h"
#include "QXmppClientExtension.h"
#include "QXmppLogger.h"

#include <QMetaType>
#include <QObject>

class QHostAddress;
class QXmppCallPrivate;
class QXmppCallManager;
class QXmppCallManagerPrivate;

/// \brief The QXmppCall class represents a Voice-Over-IP call to a remote party.
///
/// \note THIS API IS NOT FINALIZED YET

class QXMPP_EXPORT QXmppCall : public QXmppLoggable
{
    Q_OBJECT
    Q_PROPERTY(Direction direction READ direction CONSTANT)
    Q_PROPERTY(QString jid READ jid CONSTANT)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
    /// This enum is used to describe the direction of a call.
    enum Direction {
        IncomingDirection,  ///< The call is incoming.
        OutgoingDirection   ///< The call is outgoing.
    };
    Q_ENUM(Direction)

    /// This enum is used to describe the state of a call.
    enum State {
        ConnectingState = 0,     ///< The call is being connected.
        ActiveState = 1,         ///< The call is active.
        DisconnectingState = 2,  ///< The call is being disconnected.
        FinishedState = 3        ///< The call is finished.
    };
    Q_ENUM(State)

    ~QXmppCall();

    QXmppCall::Direction direction() const;
    QString jid() const;
    QString sid() const;
    QXmppCall::State state() const;

    GstElement *pipeline() const;
    QXmppCallStream *audioStream() const;
    QXmppCallStream *videoStream() const;

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

    /// \brief This signal is emitted when a stream is created.
    void streamCreated(QXmppCallStream *stream);

public slots:
    void accept();
    void hangup();
    void addVideo();

private slots:
    void localCandidatesChanged();
    void terminated();

private:
    QXmppCall(const QString &jid, QXmppCall::Direction direction, QXmppCallManager *parent);

    QXmppCallPrivate *d;
    friend class QXmppCallManager;
    friend class QXmppCallManagerPrivate;
    friend class QXmppCallPrivate;
};

Q_DECLARE_METATYPE(QXmppCall::State)

#endif
