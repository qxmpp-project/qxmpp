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

#ifndef QXMPPRTPCHANNEL_H
#define QXMPPRTPCHANNEL_H

#include <QIODevice>
#include <QSize>

#include "QXmppJingleIq.h"
#include "QXmppLogger.h"

class QXmppCodec;
class QXmppJinglePayloadType;
class QXmppRtpAudioChannelPrivate;
class QXmppRtpVideoChannelPrivate;

/// \brief The QXmppRtpPacket class represents an RTP packet.
///

class QXmppRtpPacket
{
public:
    bool decode(const QByteArray &ba);
    QByteArray encode() const;
    QString toString() const;

    quint8 version;
    bool marker;
    quint8 type;
    quint32 ssrc;
    QList<quint32> csrc;
    quint16 sequence;
    quint32 stamp;
    QByteArray payload;
};

class QXmppRtpChannel
{
public:
    QXmppRtpChannel();

    virtual void close() = 0;
    virtual QIODevice::OpenMode openMode() const = 0;
    QList<QXmppJinglePayloadType> localPayloadTypes();
    void setRemotePayloadTypes(const QList<QXmppJinglePayloadType> &remotePayloadTypes);

protected:
    virtual void payloadTypesChanged();

    QList<QXmppJinglePayloadType> m_incomingPayloadTypes;
    QList<QXmppJinglePayloadType> m_outgoingPayloadTypes;
    bool m_outgoingPayloadNumbered;
};

/// \brief The QXmppRtpAudioChannel class represents an RTP audio channel to a remote party.
///
/// It acts as a QIODevice so that you can read / write audio samples, for
/// instance using a QAudioOutput and a QAudioInput.
///
/// \note THIS API IS NOT FINALIZED YET

class QXmppRtpAudioChannel : public QIODevice, public QXmppRtpChannel
{
    Q_OBJECT
    Q_ENUMS(Tone)

public:
    /// This enum is used to describe a DTMF tone.
    enum Tone {
        Tone_0 = 0, ///< Tone for the 0 key.
        Tone_1,     ///< Tone for the 1 key.
        Tone_2,     ///< Tone for the 2 key.
        Tone_3,     ///< Tone for the 3 key.
        Tone_4,     ///< Tone for the 4 key.
        Tone_5,     ///< Tone for the 5 key.
        Tone_6,     ///< Tone for the 6 key.
        Tone_7,     ///< Tone for the 7 key.
        Tone_8,     ///< Tone for the 8 key.
        Tone_9,     ///< Tone for the 9 key.
        Tone_Star,  ///< Tone for the * key.
        Tone_Pound, ///< Tone for the # key.
        Tone_A,     ///< Tone for the A key.
        Tone_B,     ///< Tone for the B key.
        Tone_C,     ///< Tone for the C key.
        Tone_D      ///< Tone for the D key.
    };

    QXmppRtpAudioChannel(QObject *parent = 0);
    ~QXmppRtpAudioChannel();

    QXmppJinglePayloadType payloadType() const;

    /// \cond
    qint64 bytesAvailable() const;
    void close();
    bool isSequential() const;
    QIODevice::OpenMode openMode() const;
    qint64 pos() const;
    bool seek(qint64 pos);
    /// \endcond

signals:
    /// \brief This signal is emitted when a datagram needs to be sent.
    void sendDatagram(const QByteArray &ba);

    /// \brief This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

public slots:
    void datagramReceived(const QByteArray &ba);
    void startTone(QXmppRtpAudioChannel::Tone tone);
    void stopTone(QXmppRtpAudioChannel::Tone tone);

protected:
    /// \cond
    void debug(const QString &message)
    {
        emit logMessage(QXmppLogger::DebugMessage, qxmpp_loggable_trace(message));
    }

    void warning(const QString &message)
    {
        emit logMessage(QXmppLogger::WarningMessage, qxmpp_loggable_trace(message));
    }

    void logReceived(const QString &message)
    {
        emit logMessage(QXmppLogger::ReceivedMessage, qxmpp_loggable_trace(message));
    }

    void logSent(const QString &message)
    {
        emit logMessage(QXmppLogger::SentMessage, qxmpp_loggable_trace(message));
    }

    void payloadTypesChanged();
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    /// \endcond

private slots:
    void emitSignals();
    void writeDatagram();

private:
    friend class QXmppRtpAudioChannelPrivate;
    QXmppRtpAudioChannelPrivate * d;
};

/// \brief The QXmppVideoFrame class provides a representation of a frame of video data.
///
/// \note THIS API IS NOT FINALIZED YET

class QXmppVideoFrame
{
public:
    enum PixelFormat {
        Format_Invalid = 0,
        Format_RGB32 = 3,
        Format_RGB24 = 4,
        Format_YUV420P = 18,
        Format_UYVY = 20,
        Format_YUYV = 21,
    };

    QXmppVideoFrame();
    QXmppVideoFrame(int bytes, const QSize &size, int bytesPerLine, PixelFormat format);
    uchar *bits();
    const uchar *bits() const;
    int bytesPerLine() const;
    int height() const;
    bool isValid() const;
    int mappedBytes() const;
    PixelFormat pixelFormat() const;
    QSize size() const;
    int width() const;

private:
    int m_bytesPerLine;
    QByteArray m_data;
    int m_height;
    int m_mappedBytes;
    PixelFormat m_pixelFormat;
    int m_width;
};

class QXmppVideoFormat
{
public:
    int frameHeight() const {
        return m_frameSize.height();
    }

    int frameWidth() const {
        return m_frameSize.width();
    }

    qreal frameRate() const {
        return m_frameRate;
    }

    void setFrameRate(qreal frameRate) {
        m_frameRate = frameRate;
    }

    QSize frameSize() const {
        return m_frameSize;
    }

    void setFrameSize(const QSize &frameSize) {
        m_frameSize = frameSize;
    }

    QXmppVideoFrame::PixelFormat pixelFormat() const {
        return m_pixelFormat;
    }

    void setPixelFormat(QXmppVideoFrame::PixelFormat pixelFormat) {
        m_pixelFormat = pixelFormat;
    }

private:
    qreal m_frameRate;
    QSize m_frameSize;
    QXmppVideoFrame::PixelFormat m_pixelFormat;
};


/// \brief The QXmppRtpVideoChannel class represents an RTP video channel to a remote party.
///
/// \note THIS API IS NOT FINALIZED YET

class QXmppRtpVideoChannel : public QXmppLoggable, public QXmppRtpChannel
{
    Q_OBJECT

public:
    QXmppRtpVideoChannel(QObject *parent = 0);
    ~QXmppRtpVideoChannel();

    // incoming stream
    QXmppVideoFormat decoderFormat() const;
    QList<QXmppVideoFrame> readFrames();

    // outgoing stream
    QXmppVideoFormat encoderFormat() const;
    void setEncoderFormat(const QXmppVideoFormat &format);
    void writeFrame(const QXmppVideoFrame &frame);

    QIODevice::OpenMode openMode() const;
    void close();

signals:
    /// \brief This signal is emitted when a datagram needs to be sent.
    void sendDatagram(const QByteArray &ba);

public slots:
    void datagramReceived(const QByteArray &ba);

protected:
    void payloadTypesChanged();

private:
    friend class QXmppRtpVideoChannelPrivate;
    QXmppRtpVideoChannelPrivate * d;
};

#endif
