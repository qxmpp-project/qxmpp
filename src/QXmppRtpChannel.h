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

#ifndef QXMPPRTPCHANNEL_H
#define QXMPPRTPCHANNEL_H

#include <QIODevice>

#include "QXmppLogger.h"

class QXmppCodec;
class QXmppJinglePayloadType;
class QXmppRtpChannelPrivate;

/// \brief The QXmppRtpChannel class represents an RTP channel to a remote party.
///
/// It acts as a QIODevice so that you can read / write audio samples, for
/// instance using a QAudioOutput and a QAudioInput.
///
/// \note THIS API IS NOT FINALIZED YET

class QXmppRtpChannel : public QIODevice
{
    Q_OBJECT

public:
    enum CodecId {
        G711u = 0,
        GSM = 3,
        G723 = 4,
        G711a = 8,
        G722 = 9,
        L16Stereo = 10,
        L16Mono = 11,
        G728 = 15,
        G729 = 18,
    };

    QXmppRtpChannel(QObject *parent = 0);
    ~QXmppRtpChannel();

    QXmppJinglePayloadType payloadType() const;

    QList<QXmppJinglePayloadType> localPayloadTypes() const;
    void setRemotePayloadTypes(const QList<QXmppJinglePayloadType> &remotePayloadTypes);

    /// \cond
    qint64 bytesAvailable() const;
    bool isSequential() const;
    /// \endcond

signals:
    /// This signal is emitted when a datagram needs to be sent.
    void sendDatagram(const QByteArray &ba);

    /// This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

public slots:
    void datagramReceived(const QByteArray &ba);

protected:
    void debug(const QString &message)
    {
        emit logMessage(QXmppLogger::DebugMessage, message);
    }

    void warning(const QString &message)
    {
        emit logMessage(QXmppLogger::WarningMessage, message);
    }

    void logReceived(const QString &message)
    {
        emit logMessage(QXmppLogger::ReceivedMessage, message);
    }

    void logSent(const QString &message)
    {
        emit logMessage(QXmppLogger::SentMessage, message);
    }

    /// \cond
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    /// \endcond

private slots:
    void emitSignals();
    void writeDatagram();

private:
    QXmppRtpChannelPrivate * const d;
};

#endif
