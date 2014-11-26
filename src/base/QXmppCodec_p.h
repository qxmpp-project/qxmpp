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

#ifndef QXMPPCODEC_H
#define QXMPPCODEC_H

#include <QMap>

#include "QXmppGlobal.h"

class QXmppRtpPacket;
class QXmppVideoFormat;
class QXmppVideoFrame;

/// \brief The QXmppCodec class is the base class for audio codecs capable of
/// encoding and decoding audio samples.
///
/// Samples must be 16-bit little endian.

class QXMPP_AUTOTEST_EXPORT QXmppCodec
{
public:
    virtual ~QXmppCodec();

    /// Reads samples from the input stream, encodes them and writes the
    /// encoded data to the output stream.
    virtual qint64 encode(QDataStream &input, QDataStream &output) = 0;

    /// Reads encoded data from the input stream, decodes it and writes the
    /// decoded samples to the output stream.
    virtual qint64 decode(QDataStream &input, QDataStream &output) = 0;
};

/// \internal
///
/// The QXmppG711aCodec class represent a G.711 a-law PCM codec.

class QXmppG711aCodec : public QXmppCodec
{
public:
    QXmppG711aCodec(int clockrate);

    qint64 encode(QDataStream &input, QDataStream &output);
    qint64 decode(QDataStream &input, QDataStream &output);

private:
    int m_frequency;
};

/// \internal
///
/// The QXmppG711uCodec class represent a G.711 u-law PCM codec.

class QXmppG711uCodec : public QXmppCodec
{
public:
    QXmppG711uCodec(int clockrate);

    qint64 encode(QDataStream &input, QDataStream &output);
    qint64 decode(QDataStream &input, QDataStream &output);

private:
    int m_frequency;
};

#ifdef QXMPP_USE_SPEEX
typedef struct SpeexBits SpeexBits;

/// \internal
///
/// The QXmppSpeexCodec class represent a SPEEX codec.

class QXMPP_AUTOTEST_EXPORT QXmppSpeexCodec : public QXmppCodec
{
public:
    QXmppSpeexCodec(int clockrate);
    ~QXmppSpeexCodec();

    qint64 encode(QDataStream &input, QDataStream &output);
    qint64 decode(QDataStream &input, QDataStream &output);

private:
    SpeexBits *encoder_bits;
    void *encoder_state;
    SpeexBits *decoder_bits;
    void *decoder_state;
    int frame_samples;
};
#endif

#ifdef QXMPP_USE_OPUS
typedef struct OpusEncoder OpusEncoder;
typedef struct OpusDecoder OpusDecoder;

/// \internal
///
/// The QXmppOpusCodec class represent a Opus codec.

class QXMPP_AUTOTEST_EXPORT QXmppOpusCodec : public QXmppCodec
{
public:
    QXmppOpusCodec(int clockrate, int channels);
    ~QXmppOpusCodec();

    qint64 encode(QDataStream &input, QDataStream &output);
    qint64 decode(QDataStream &input, QDataStream &output);

private:
    OpusEncoder *encoder;
    OpusDecoder *decoder;
    int sampleRate;
    int nChannels;
    QList<float> validFrameSize;
    int nSamples;
    QByteArray sampleBuffer;

    int readWindow(int bufferSize);
};
#endif

/// \brief The QXmppVideoDecoder class is the base class for video decoders.
///

class QXMPP_AUTOTEST_EXPORT QXmppVideoDecoder
{
public:
    virtual ~QXmppVideoDecoder();

    /// Returns the format of the video stream.
    virtual QXmppVideoFormat format() const = 0;

    /// Handles an RTP \a packet and returns a list of decoded video frames.
    virtual QList<QXmppVideoFrame> handlePacket(const QXmppRtpPacket &packet) = 0;

    /// Sets the video stream's \a parameters.
    virtual bool setParameters(const QMap<QString, QString> &parameters) = 0;
};

/// \brief The QXmppVideoEncoder class is the base class for video encoders.
///

class QXMPP_AUTOTEST_EXPORT QXmppVideoEncoder
{
public:
    virtual ~QXmppVideoEncoder();

    /// Sets the \a format of the video stream.
    virtual bool setFormat(const QXmppVideoFormat &format) = 0;

    /// Handles a video \a frame and returns a list of RTP packet payloads.
    virtual QList<QByteArray> handleFrame(const QXmppVideoFrame &frame) = 0;

    /// Returns the video stream's parameters.
    virtual QMap<QString, QString> parameters() const = 0;
};

#ifdef QXMPP_USE_THEORA
class QXmppTheoraDecoderPrivate;
class QXmppTheoraEncoderPrivate;

class QXMPP_AUTOTEST_EXPORT QXmppTheoraDecoder : public QXmppVideoDecoder
{
public:
    QXmppTheoraDecoder();
    ~QXmppTheoraDecoder();

    QXmppVideoFormat format() const;
    QList<QXmppVideoFrame> handlePacket(const QXmppRtpPacket &packet);
    bool setParameters(const QMap<QString, QString> &parameters);

private:
    QXmppTheoraDecoderPrivate *d;
};

class QXMPP_AUTOTEST_EXPORT QXmppTheoraEncoder : public QXmppVideoEncoder
{
public:
    QXmppTheoraEncoder();
    ~QXmppTheoraEncoder();

    bool setFormat(const QXmppVideoFormat &format);
    QList<QByteArray> handleFrame(const QXmppVideoFrame &frame);
    QMap<QString, QString> parameters() const;

private:
    QXmppTheoraEncoderPrivate *d;
};
#endif

#ifdef QXMPP_USE_VPX
class QXmppVpxDecoderPrivate;
class QXmppVpxEncoderPrivate;

class QXMPP_AUTOTEST_EXPORT QXmppVpxDecoder : public QXmppVideoDecoder
{
public:
    QXmppVpxDecoder();
    ~QXmppVpxDecoder();

    QXmppVideoFormat format() const;
    QList<QXmppVideoFrame> handlePacket(const QXmppRtpPacket &packet);
    bool setParameters(const QMap<QString, QString> &parameters);

private:
    QXmppVpxDecoderPrivate *d;
};

class QXMPP_AUTOTEST_EXPORT QXmppVpxEncoder : public QXmppVideoEncoder
{
public:
    QXmppVpxEncoder(uint clockrate=0);
    ~QXmppVpxEncoder();

    bool setFormat(const QXmppVideoFormat &format);
    QList<QByteArray> handleFrame(const QXmppVideoFrame &frame);
    QMap<QString, QString> parameters() const;

private:
    QXmppVpxEncoderPrivate *d;
};
#endif

#endif
