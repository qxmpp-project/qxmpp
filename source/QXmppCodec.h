/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include <QtGlobal>

/// The QXmppCodec class is the base class for audio codecs capable of
/// encoding / decoding 16-bit mono samples.

class QXmppCodec
{
public:
    virtual int bitrate() const = 0;
    virtual qint64 encode(QDataStream &input, QDataStream &output) = 0;
    virtual qint64 decode(QDataStream &input, QDataStream &output) = 0;
};

/// The QXmppG711aCodec class represent a G.711 a-law PCM codec.

class QXmppG711aCodec : public QXmppCodec
{
public:
    QXmppG711aCodec(int clockrate);

    int bitrate() const;
    qint64 encode(QDataStream &input, QDataStream &output);
    qint64 decode(QDataStream &input, QDataStream &output);

private:
    int m_frequency;
};

/// The QXmppG711uCodec class represent a G.711 u-law PCM codec.

class QXmppG711uCodec : public QXmppCodec
{
public:
    QXmppG711uCodec(int clockrate);

    int bitrate() const;
    qint64 encode(QDataStream &input, QDataStream &output);
    qint64 decode(QDataStream &input, QDataStream &output);

private:
    int m_frequency;
};

#ifdef QXMPP_USE_SPEEX
typedef struct SpeexBits SpeexBits;

/// The QXmppSpeexCodec class represent a SPEEX codec.

class QXmppSpeexCodec : public QXmppCodec
{
public:
    QXmppSpeexCodec(int clockrate);
    ~QXmppSpeexCodec();

    int bitrate() const;
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

#endif
