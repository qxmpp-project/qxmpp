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

class QXMPP_EXPORT QXmppRtpPacket
{
public:
    bool decode(const QByteArray &ba);
    QByteArray encode() const;
    QString toString() const;

    ///  RTP version.
    quint8 version;
    /// Marker flag.
    bool marker;
    /// Payload type.
    quint8 type;
    /// Synchronization source.
    quint32 ssrc;
    /// Contributing sources.
    QList<quint32> csrc;
    /// Sequence number.
    quint16 sequence;
    /// Timestamp.
    quint32 stamp;
    /// Raw payload data.
    QByteArray payload;
};

class QXMPP_EXPORT QXmppRtpChannel
{
public:
    QXmppRtpChannel();

    /// Closes the RTP channel.
    virtual void close() = 0;

    /// Returns the mode in which the channel has been opened.
    virtual QIODevice::OpenMode openMode() const = 0;

    QList<QXmppJinglePayloadType> localPayloadTypes();
    void setRemotePayloadTypes(const QList<QXmppJinglePayloadType> &remotePayloadTypes);

protected:
    /// \cond
    virtual void payloadTypesChanged() = 0;

    QList<QXmppJinglePayloadType> m_incomingPayloadTypes;
    QList<QXmppJinglePayloadType> m_outgoingPayloadTypes;
    bool m_outgoingPayloadNumbered;
    /// \endcond
};

/// \brief The QXmppRtpAudioChannel class represents an RTP audio channel to a remote party.
///
/// It acts as a QIODevice so that you can read / write audio samples, for
/// instance using a QAudioOutput and a QAudioInput.
///
/// \note THIS API IS NOT FINALIZED YET

class QXMPP_EXPORT QXmppRtpAudioChannel : public QIODevice, public QXmppRtpChannel
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

    qint64 bytesAvailable() const;
    void close();
    bool isSequential() const;
    QIODevice::OpenMode openMode() const;
    QXmppJinglePayloadType payloadType() const;
    qint64 pos() const;
    bool seek(qint64 pos);

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

class QXMPP_EXPORT QXmppVideoFrame
{
public:
    /// This enum describes a pixel format.
    enum PixelFormat {
        Format_Invalid = 0,     ///< The frame is invalid.
        Format_RGB32 = 3,       ///< The frame stored using a 32-bit RGB format (0xffRRGGBB).
        Format_RGB24 = 4,       ///< The frame is stored using a 24-bit RGB format (8-8-8).
        Format_YUV420P = 18,    ///< The frame is stored using an 8-bit per component planar
                                ///< YUV format with the U and V planes horizontally and
                                ///< vertically sub-sampled, i.e. the height and width of the
                                ///< U and V planes are half that of the Y plane.
        Format_UYVY = 20,       ///< The frame is stored using an 8-bit per component packed
                                ///< YUV format with the U and V planes horizontally
                                ///< sub-sampled (U-Y-V-Y), i.e. two horizontally adjacent
                                ///< pixels are stored as a 32-bit macropixel which has a Y
                                ///< value for each pixel and common U and V values.
        Format_YUYV = 21,       ///< The frame is stored using an 8-bit per component packed
                                ///< YUV format with the U and V planes horizontally
                                ///< sub-sampled (Y-U-Y-V), i.e. two horizontally adjacent
                                ///< pixels are stored as a 32-bit macropixel which has a Y
                                ///< value for each pixel and common U and V values.
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

class QXMPP_EXPORT QXmppVideoFormat
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

class QXMPP_EXPORT QXmppRtpVideoChannel : public QXmppLoggable, public QXmppRtpChannel
{
    Q_OBJECT

public:
    QXmppRtpVideoChannel(QObject *parent = 0);
    ~QXmppRtpVideoChannel();

    void close();
    QIODevice::OpenMode openMode() const;

    // incoming stream
    QXmppVideoFormat decoderFormat() const;
    QList<QXmppVideoFrame> readFrames();

    // outgoing stream
    QXmppVideoFormat encoderFormat() const;
    void setEncoderFormat(const QXmppVideoFormat &format);
    void writeFrame(const QXmppVideoFrame &frame);

signals:
    /// \brief This signal is emitted when a datagram needs to be sent.
    void sendDatagram(const QByteArray &ba);

public slots:
    void datagramReceived(const QByteArray &ba);

protected:
    /// \cond
    void payloadTypesChanged();
    /// \endcond

private:
    friend class QXmppRtpVideoChannelPrivate;
    QXmppRtpVideoChannelPrivate * d;
};

#endif
