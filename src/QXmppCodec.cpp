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

/*
 *  G.711 based on reference implementation by Sun Microsystems, Inc.
 */

#include <QDataStream>
#include <QDebug>
#include <QSize>

#include "QXmppCodec.h"
#include "QXmppRtpChannel.h"

#include <cstring>

#ifdef QXMPP_USE_SPEEX
#include <speex/speex.h>
#endif

#ifdef QXMPP_USE_THEORA
#include <theora/theoradec.h>
#include <theora/theoraenc.h>
#endif

#define BIAS        (0x84)  /* Bias for linear code. */
#define CLIP        8159

#define SIGN_BIT    (0x80)  /* Sign bit for a A-law byte. */
#define QUANT_MASK  (0xf)   /* Quantization field mask. */
#define NSEGS       (8)     /* Number of A-law segments. */
#define SEG_SHIFT   (4)     /* Left shift for segment number. */
#define SEG_MASK    (0x70)  /* Segment field mask. */

static qint16 seg_aend[8] = {0x1F, 0x3F, 0x7F, 0xFF,
                0x1FF, 0x3FF, 0x7FF, 0xFFF};
static qint16 seg_uend[8] = {0x3F, 0x7F, 0xFF, 0x1FF,
                0x3FF, 0x7FF, 0xFFF, 0x1FFF};

static qint16 search(qint16 val, qint16 *table, qint16 size)
{
   qint16 i;
   
   for (i = 0; i < size; i++) {
      if (val <= *table++)
     return (i);
   }
   return (size);
}

/*
 * linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *
 * Accepts a 16-bit integer and encodes it as A-law data.
 *
 *      Linear Input Code   Compressed Code
 *  ------------------------    ---------------
 *  0000000wxyza            000wxyz
 *  0000001wxyza            001wxyz
 *  000001wxyzab            010wxyz
 *  00001wxyzabc            011wxyz
 *  0001wxyzabcd            100wxyz
 *  001wxyzabcde            101wxyz
 *  01wxyzabcdef            110wxyz
 *  1wxyzabcdefg            111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */
quint8 linear2alaw(qint16 pcm_val)
{
   qint16 mask;
   qint16 seg;
   quint8 aval;
   
   pcm_val = pcm_val >> 3;

   if (pcm_val >= 0) {
      mask = 0xD5;      /* sign (7th) bit = 1 */
   } else {
      mask = 0x55;      /* sign bit = 0 */
      pcm_val = -pcm_val - 1;
   }
   
   /* Convert the scaled magnitude to segment number. */
   seg = search(pcm_val, seg_aend, 8);
   
   /* Combine the sign, segment, and quantization bits. */
   
   if (seg >= 8)        /* out of range, return maximum value. */
      return (quint8) (0x7F ^ mask);
   else {
      aval = (quint8) seg << SEG_SHIFT;
      if (seg < 2)
     aval |= (pcm_val >> 1) & QUANT_MASK;
      else
     aval |= (pcm_val >> seg) & QUANT_MASK;
      return (aval ^ mask);
   }
}

/*
 * alaw2linear() - Convert an A-law value to 16-bit linear PCM
 *
 */
qint16 alaw2linear(quint8 a_val)
{
   qint16 t;
   qint16 seg;
   
   a_val ^= 0x55;
   
   t = (a_val & QUANT_MASK) << 4;
   seg = ((qint16)a_val & SEG_MASK) >> SEG_SHIFT;
   switch (seg) {
   case 0:
      t += 8;
      break;
   case 1:
      t += 0x108;
      break;
   default:
      t += 0x108;
      t <<= seg - 1;
   }
   return ((a_val & SIGN_BIT) ? t : -t);
}

/*
 * linear2ulaw() - Convert a linear PCM value to u-law
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *   Biased Linear Input Code    Compressed Code
 *   ------------------------    ---------------
 *   00000001wxyza           000wxyz
 *   0000001wxyzab           001wxyz
 *   000001wxyzabc           010wxyz
 *   00001wxyzabcd           011wxyz
 *   0001wxyzabcde           100wxyz
 *   001wxyzabcdef           101wxyz
 *   01wxyzabcdefg           110wxyz
 *   1wxyzabcdefgh           111wxyz
 *
 * Each biased linear code has a leading 1 which identifies the segment
 * number. The value of the segment number is equal to 7 minus the number
 * of leading 0's. The quantization interval is directly available as the
 * four bits wxyz.  * The trailing bits (a - h) are ignored.
 *
 * Ordinarily the complement of the resulting code word is used for
 * transmission, and so the code word is complemented before it is returned.
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */
quint8 linear2ulaw(qint16 pcm_val)
{
   qint16 mask;
   qint16 seg;
   quint8 uval;
   
   /* Get the sign and the magnitude of the value. */
   pcm_val = pcm_val >> 2;
   if (pcm_val < 0) {
      pcm_val = -pcm_val;
      mask = 0x7F;
   } else {
      mask = 0xFF;
   }
   if (pcm_val > CLIP) pcm_val = CLIP;        /* clip the magnitude */
   pcm_val += (BIAS >> 2);
   
   /* Convert the scaled magnitude to segment number. */
   seg = search(pcm_val, seg_uend, 8);
   
   /*
   * Combine the sign, segment, quantization bits;
   * and complement the code word.
   */
   if (seg >= 8)        /* out of range, return maximum value. */
      return (quint8) (0x7F ^ mask);
   else {
      uval = (quint8) (seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
      return (uval ^ mask);
   }
}

/*
 * ulaw2linear() - Convert a u-law value to 16-bit linear PCM
 *
 * First, a biased linear code is derived from the code word. An unbiased
 * output can then be obtained by subtracting 33 from the biased code.
 *
 * Note that this function expects to be passed the complement of the
 * original code word. This is in keeping with ISDN conventions.
 */
qint16 ulaw2linear(quint8 u_val)
{
   qint16 t;
   
   /* Complement to obtain normal u-law value. */
   u_val = ~u_val;
   
   /*
    * Extract and bias the quantization bits. Then
    * shift up by the segment number and subtract out the bias.
    */
   t = ((u_val & QUANT_MASK) << 3) + BIAS;
   t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;
   
   return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

QXmppG711aCodec::QXmppG711aCodec(int clockrate)
{
    m_frequency = clockrate;
}

qint64 QXmppG711aCodec::encode(QDataStream &input, QDataStream &output)
{
    qint64 samples = 0;
    qint16 pcm;
    while (!input.atEnd())
    {
        input >> pcm;
        output << linear2alaw(pcm);
        ++samples;
    }
    return samples;
}

qint64 QXmppG711aCodec::decode(QDataStream &input, QDataStream &output)
{
    qint64 samples = 0;
    quint8 g711;
    while (!input.atEnd())
    {
        input >> g711;
        output << alaw2linear(g711);
        ++samples;
    }
    return samples;
}

QXmppG711uCodec::QXmppG711uCodec(int clockrate)
{
    m_frequency = clockrate;
}

qint64 QXmppG711uCodec::encode(QDataStream &input, QDataStream &output)
{
    qint64 samples = 0;
    qint16 pcm;
    while (!input.atEnd())
    {
        input >> pcm;
        output << linear2ulaw(pcm);
        ++samples;
    }
    return samples;
}

qint64 QXmppG711uCodec::decode(QDataStream &input, QDataStream &output)
{
    qint64 samples = 0;
    quint8 g711;
    while (!input.atEnd())
    {
        input >> g711;
        output << ulaw2linear(g711);
        ++samples;
    }
    return samples;
}

#ifdef QXMPP_USE_SPEEX
QXmppSpeexCodec::QXmppSpeexCodec(int clockrate)
{
    const SpeexMode *mode = &speex_nb_mode;
    if (clockrate == 32000)
        mode = &speex_uwb_mode;
    else if (clockrate == 16000)
        mode = &speex_wb_mode;
    else if (clockrate == 8000)
        mode = &speex_nb_mode;
    else
        qWarning() << "QXmppSpeexCodec got invalid clockrate" << clockrate;

    // encoder
    encoder_bits = new SpeexBits;
    speex_bits_init(encoder_bits);
    encoder_state = speex_encoder_init(mode);

    // decoder
    decoder_bits = new SpeexBits;
    speex_bits_init(decoder_bits);
    decoder_state = speex_decoder_init(mode);

    // get frame size in samples
    speex_encoder_ctl(encoder_state, SPEEX_GET_FRAME_SIZE, &frame_samples);
}

QXmppSpeexCodec::~QXmppSpeexCodec()
{
    delete encoder_bits;
    delete decoder_bits;
}

qint64 QXmppSpeexCodec::encode(QDataStream &input, QDataStream &output)
{
    QByteArray pcm_buffer(frame_samples * 2, 0);
    const int length = input.readRawData(pcm_buffer.data(), pcm_buffer.size());
    if (length != pcm_buffer.size())
    {
        qWarning() << "Read only read" << length << "bytes";
        return 0;
    }
    speex_bits_reset(encoder_bits);
    speex_encode_int(encoder_state, (short*)pcm_buffer.data(), encoder_bits);
    QByteArray speex_buffer(speex_bits_nbytes(encoder_bits), 0);
    speex_bits_write(encoder_bits, speex_buffer.data(), speex_buffer.size());
    output.writeRawData(speex_buffer.data(), speex_buffer.size());
    return frame_samples;
}

qint64 QXmppSpeexCodec::decode(QDataStream &input, QDataStream &output)
{
    const int length = input.device()->bytesAvailable();
    QByteArray speex_buffer(length, 0);
    input.readRawData(speex_buffer.data(), speex_buffer.size());
    speex_bits_read_from(decoder_bits, speex_buffer.data(), speex_buffer.size());
    QByteArray pcm_buffer(frame_samples * 2, 0);
    speex_decode_int(decoder_state, decoder_bits, (short*)pcm_buffer.data());
    output.writeRawData(pcm_buffer.data(), pcm_buffer.size());
    return frame_samples;
}

#endif

#ifdef QXMPP_USE_THEORA

class QXmppTheoraDecoderPrivate
{
public:
    bool decodeFrame(const QByteArray &buffer, QXmppVideoFrame *frame);

    th_comment comment;
    th_info info;
    th_setup_info *setup_info;
    th_dec_ctx *ctx;

    QByteArray packetBuffer;
};

bool QXmppTheoraDecoderPrivate::decodeFrame(const QByteArray &buffer, QXmppVideoFrame *frame)
{
    ogg_packet packet;
    packet.packet = (unsigned char*) buffer.data();
    packet.bytes = buffer.size();
    packet.b_o_s = 1;
    packet.e_o_s = 0;
    packet.granulepos = -1;
    packet.packetno = 0;
    if (th_decode_packetin(ctx, &packet, 0) != 0) {
        qWarning("Theora packet could not be decoded");
        return false;
    }

    th_ycbcr_buffer ycbcr_buffer;
    if (th_decode_ycbcr_out(ctx, ycbcr_buffer) != 0) {
        qWarning("Theora packet has no Y'CbCr");
        return false;
    }

    for (int i = 0; i < 3; ++i) {
        QXmppVideoPlane *plane = &frame->planes[i];
        plane->width = ycbcr_buffer[i].width;
        plane->height = ycbcr_buffer[i].height;
        plane->stride = ycbcr_buffer[i].stride;
        plane->data.resize(plane->stride * plane->height);
        memcpy(plane->data.data(), ycbcr_buffer[i].data, plane->data.size());
    }
    return true;
}


QXmppTheoraDecoder::QXmppTheoraDecoder()
{
    d = new QXmppTheoraDecoderPrivate;
    th_comment_init(&d->comment);
    th_info_init(&d->info);
    d->setup_info = 0;
    d->ctx = 0;
}

QXmppTheoraDecoder::~QXmppTheoraDecoder()
{
    th_comment_clear(&d->comment);
    th_info_clear(&d->info);
    if (d->setup_info)
        th_setup_free(d->setup_info);
    if (d->ctx)
        th_decode_free(d->ctx);
    delete d;
}

QXmppVideoFormat QXmppTheoraDecoder::format() const
{
    QXmppVideoFormat format;
    format.setFrameSize(QSize(d->info.frame_width, d->info.frame_height));
    if (d->info.pixel_fmt == TH_PF_420) {
        format.setPixelFormat(QXmppVideoFrame::Format_YUV420P);
    }
    return format;
}

QList<QXmppVideoFrame> QXmppTheoraDecoder::handlePacket(QDataStream &stream)
{
    QList<QXmppVideoFrame> frames;

    // theora deframing: draft-ietf-avt-rtp-theora-00
    quint32 theora_header;
    stream >> theora_header;

    quint32 theora_ident = (theora_header >> 8) & 0xffffff;
    quint8 theora_frag = (theora_header & 0xc0) >> 6;
    quint8 theora_type = (theora_header & 0x30) >> 4;
    quint8 theora_packets = (theora_header & 0x0f);

    //qDebug("ident: 0x%08x, F: %d, TDT: %d, packets: %d", theora_ident, theora_frag, theora_type, theora_packets);

    // We only handle raw theora data
    if (theora_type != 0)
        return frames;

    QXmppVideoFrame frame;
    quint16 packetLength;

    if (theora_frag == 0) {
        // unfragmented packet(s)
        for (int i = 0; i < theora_packets; ++i) {
            stream >> packetLength;
            if (packetLength > stream.device()->bytesAvailable()) {
                qWarning("Theora unfragmented packet has an invalid length");
                return frames;
            }

            d->packetBuffer.resize(packetLength);
            stream.readRawData(d->packetBuffer.data(), packetLength);
            if (d->ctx && d->decodeFrame(d->packetBuffer, &frame))
                frames << frame;
        }
    } else {
        // fragments
        stream >> packetLength;
        if (packetLength > stream.device()->bytesAvailable()) {
            qWarning("Theora packet has an invalid length");
            return frames;
        }

        int pos;
        if (theora_frag == 1) {
            // start fragment
            pos = 0;
            d->packetBuffer.resize(packetLength);
        } else {
            // continuation or end fragment
            pos = d->packetBuffer.size();
            d->packetBuffer.resize(pos + packetLength);
        }
        stream.readRawData(d->packetBuffer.data() + pos, packetLength);

        if (theora_frag == 3) {
            // end fragment
            if (d->ctx && d->decodeFrame(d->packetBuffer, &frame))
                frames << frame;
            d->packetBuffer.resize(0);
        }
    }
    return frames;
}

bool QXmppTheoraDecoder::setParameters(const QMap<QString, QString> &parameters)
{
    QByteArray config = QByteArray::fromBase64(parameters.value("configuration").toAscii());
    QDataStream stream(config);
    const QIODevice *device = stream.device();

    if (device->bytesAvailable() < 4) {
        qWarning("Theora configuration is too small");
        return false;
    }

    // Process packed headers
    int done = 0;
    quint32 header_count;
    stream >> header_count;
    for (quint32 i = 0; i < header_count; ++i) {
        if (device->bytesAvailable() < 6) {
            qWarning("Theora configuration is too small");
            return false;
        }
        QByteArray ident(3, 0);
        quint16 length;
        quint8 h_count;

        stream.readRawData(ident.data(), ident.size());
        stream >> length;
        stream >> h_count;
#ifdef QXMPP_DEBUG_THEORA
        qDebug("Theora packed header %u ident=%s bytes=%u count=%u", i, ident.toHex().data(), length, h_count);
#endif

        // get header sizes
        QList<qint64> h_sizes;
        for (int h = 0; h < h_count; ++h) {
            quint16 h_size = 0;
            quint8 b;
            do {
                if (device->bytesAvailable() < 1) {
                    qWarning("Theora configuration is too small");
                    return false;
                }
                stream >> b;
                h_size = (h_size << 7) | (b & 0x7f);
            } while (b & 0x80);
            h_sizes << h_size;
#ifdef QXMPP_DEBUG_THEORA
            qDebug("Theora header %d size %u", h_sizes.size() - 1, h_sizes.last());
#endif
            length -= h_size;
        }
        h_sizes << length;
#ifdef QXMPP_DEBUG_THEORA
        qDebug("Theora header %d size %u", h_sizes.size() - 1, h_sizes.last());
#endif

        // decode headers
        ogg_packet packet;
        packet.b_o_s = 1;
        packet.e_o_s = 0;
        packet.granulepos = -1;
        packet.packetno = 0;

        foreach (int h_size, h_sizes) {
            if (device->bytesAvailable() < h_size) {
                qWarning("Theora configuration is too small");
                return false;
            }

            packet.packet = (unsigned char*) (config.data() + device->pos());
            packet.bytes = h_size;
            int ret = th_decode_headerin(&d->info, &d->comment, &d->setup_info, &packet);
            if (ret < 0) {
                qWarning("Theora header could not be decoded");
                return false;
            }
            done += ret;
            stream.skipRawData(h_size);
        }
    }

    // check for completion
    if (done < 3) {
        qWarning("Theora configuration did not contain enough headers");
        return false;
    }
    qDebug("Theora frame_width %i, frame_height %i, colorspace %i, pixel_fmt: %i, target_bitrate: %i, quality: %i, keyframe_granule_shift: %i",
        d->info.frame_width,
        d->info.frame_height,
        d->info.colorspace,
        d->info.pixel_fmt,
        d->info.target_bitrate,
        d->info.quality,
        d->info.keyframe_granule_shift);
    if (d->info.pixel_fmt != TH_PF_420) {
        qWarning("Theora frames have an unsupported pixel format %d", d->info.pixel_fmt);
        return false;
    }
    if (d->ctx)
        th_decode_free(d->ctx);
    d->ctx = th_decode_alloc(&d->info, d->setup_info);
    if (!d->ctx) {
        qWarning("Theora decoder could not be allocated");
        return false;
    }
    return true;
}

class QXmppTheoraEncoderPrivate
{
public:
    void writeFrame(QDataStream &stream, quint8 theora_frag, quint8 theora_packets, const char *data, quint16 length);

    th_comment comment;
    th_info info;
    th_setup_info *setup_info;
    th_enc_ctx *ctx;

    QByteArray configuration;
    QByteArray ident;
};

void QXmppTheoraEncoderPrivate::writeFrame(QDataStream &stream, quint8 theora_frag, quint8 theora_packets, const char *data, quint16 length)
{
    // raw data
    const quint8 theora_type = 0;
    stream.writeRawData(ident.constData(), ident.size());
    stream << quint8(((theora_frag << 6) & 0xc0) |
                     ((theora_type << 4) & 0x30) |
                     (theora_packets & 0x0f));
    stream << quint16(length);
    stream.writeRawData(data, length);
}

QXmppTheoraEncoder::QXmppTheoraEncoder()
{
    d = new QXmppTheoraEncoderPrivate;
    d->ident = QByteArray("\xc3\x45\xae");
    th_comment_init(&d->comment);
    th_info_init(&d->info);
    d->setup_info = 0;
    d->ctx = 0;
}

QXmppTheoraEncoder::~QXmppTheoraEncoder()
{
    th_comment_clear(&d->comment);
    th_info_clear(&d->info);
    if (d->setup_info)
        th_setup_free(d->setup_info);
    if (d->ctx)
        th_encode_free(d->ctx);
    delete d;
}

bool QXmppTheoraEncoder::setFormat(const QXmppVideoFormat &format)
{
    if (format.pixelFormat() == QXmppVideoFrame::Format_YUV420P) {
        d->info.pixel_fmt = TH_PF_420;
    } else {
        qWarning("Theora decoder does not support the given format");
        return false;
    }
    d->info.frame_width = format.frameSize().width();
    d->info.frame_height = format.frameSize().height();
    d->info.pic_height = format.frameSize().height();
    d->info.pic_width = format.frameSize().width();
    d->info.pic_x = 0;
    d->info.pic_y = 0;
    d->info.colorspace = TH_CS_UNSPECIFIED;
    d->info.target_bitrate = 0;
    d->info.quality = 48;
    d->info.keyframe_granule_shift = 6;

    // frame rate
    d->info.fps_numerator = 30;
    d->info.fps_denominator = 1;

    if (d->ctx) {
        th_encode_free(d->ctx);
        d->ctx = 0;
    }
    d->ctx = th_encode_alloc(&d->info);
    if (!d->ctx) {
        qWarning("Theora encoder could not be allocated");
        return false;
    }

    // fetch headers
    QList<QByteArray> headers;
    ogg_packet packet;
    while (th_encode_flushheader(d->ctx, &d->comment, &packet) > 0)
        headers << QByteArray((const char*)packet.packet, packet.bytes);

    // store configuration
    d->configuration.clear();
    QDataStream stream(&d->configuration, QIODevice::WriteOnly);
    stream << quint32(1);

    quint16 length = 0;
    foreach (const QByteArray &header, headers)
        length += header.size();

    quint8 h_count = headers.size() - 1;
    stream.writeRawData(d->ident.constData(), d->ident.size());
    stream << length;
    stream << h_count;
#ifdef QXMPP_DEBUG_THEORA
    qDebug("Theora packed header %u ident=%s bytes=%u count=%u", 0, d->ident.toHex().data(), length, h_count);
#endif

    // write header sizes
    for (int h = 0; h < h_count; ++h) {
        quint16 h_size = headers[h].size();
        do {
            quint8 b = (h_size & 0x7f);
            h_size >>= 7;
            if (h_size)
                b |= 0x80;
            stream << b;
        } while (h_size);
    }

    // write headers
    for (int h = 0; h < headers.size(); ++h) {
#ifdef QXMPP_DEBUG_THEORA
        qDebug("Header %d size %d", h, headers[h].size());
#endif
        stream.writeRawData(headers[h].data(), headers[h].size());
    }
    
    return true;
}

QList<QByteArray> QXmppTheoraEncoder::handleFrame(const QXmppVideoFrame &frame)
{
    QList<QByteArray> packets;
    const int PACKET_MAX = 1388;

    th_ycbcr_buffer ycbcr_buffer;
    for (int i = 0; i < 3; ++i) {
        const QXmppVideoPlane *plane = &frame.planes[i];
        ycbcr_buffer[i].width = plane->width;
        ycbcr_buffer[i].height = plane->height;
        ycbcr_buffer[i].stride = plane->stride;
        ycbcr_buffer[i].data = (unsigned char*)plane->data.constData();
    }
    if (th_encode_ycbcr_in(d->ctx, ycbcr_buffer) != 0) {
        qWarning("Theora encoder could not handle frame");
        return packets;
    }

    // raw data
    quint8 theora_type = 0;
    QByteArray payload;
    ogg_packet packet;
    while (th_encode_packetout(d->ctx, 0, &packet) > 0) {
#ifdef QXMPP_DEBUG_THEORA
        qDebug("Theora encoded packet %d bytes", packet.bytes);
#endif
        QDataStream stream(&payload, QIODevice::WriteOnly);
        const char *data = (const char*) packet.packet;
        int size = packet.bytes;
        quint8 theora_frag = 0;
        if (size <= PACKET_MAX) {
            // no fragmentation
            stream.device()->reset();
            payload.clear();
            d->writeFrame(stream, theora_frag, 1, data, size);
            packets << payload;
       } else {
            // fragmentation
            theora_frag = 1;
            while (size) {
                const int length = qMin(PACKET_MAX, size);
                payload.clear();
                stream.device()->reset();
                d->writeFrame(stream, theora_frag, 0, data, length);
                data += length;
                size -= length;
                theora_frag = (size < length) ? 3 : 2;
                packets << payload;
            }
        }
    }

    return packets;
}

QMap<QString, QString> QXmppTheoraEncoder::parameters() const
{
    QMap<QString, QString> params;
    if (d->ctx) {
        params.insert("delivery-method", "inline");
        params.insert("configuration", d->configuration.toBase64());
    }
    return params;
}

#endif
