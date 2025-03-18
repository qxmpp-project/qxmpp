// SPDX-FileCopyrightText: 2019 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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

class QXMPP_EXPORT QXmppCall : public QXmppLoggable
{
    Q_OBJECT
    /// The call's direction
    Q_PROPERTY(Direction direction READ direction CONSTANT)
    /// The remote party's JID
    Q_PROPERTY(QString jid READ jid CONSTANT)
    /// The call's state
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

    /// \brief This signal is emitted when a call is connected.
    ///
    /// Once this signal is emitted, you can connect a QAudioOutput and
    /// QAudioInput to the call. You can determine the appropriate clockrate
    /// and the number of channels by calling payloadType().
    Q_SIGNAL void connected();

    /// \brief This signal is emitted when a call is finished.
    ///
    /// Note: Do not delete the call in the slot connected to this signal,
    /// instead use deleteLater().
    Q_SIGNAL void finished();

    /// \brief This signal is emitted when the remote party is ringing.
    Q_SIGNAL void ringing();

    /// \brief This signal is emitted when the call state changes.
    Q_SIGNAL void stateChanged(QXmppCall::State state);

    /// \brief This signal is emitted when a stream is created.
    Q_SIGNAL void streamCreated(QXmppCallStream *stream);

    Q_SLOT void accept();
    Q_SLOT void hangup();
    Q_SLOT void addVideo();

private:
    void onLocalCandidatesChanged(QXmppCallStream *stream);
    void terminated();

    QXmppCall(const QString &jid, QXmppCall::Direction direction, QXmppCallManager *parent);

    const std::unique_ptr<QXmppCallPrivate> d;
    friend class QXmppCallManager;
    friend class QXmppCallManagerPrivate;
    friend class QXmppCallPrivate;
};

Q_DECLARE_METATYPE(QXmppCall::State)

#endif
