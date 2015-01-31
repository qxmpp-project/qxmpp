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

/*
 *  G.711 based on reference implementation by Sun Microsystems, Inc.
 */

#include <QDataStream>
#include <QDebug>
#include <QSize>
#include <QThread>

#include "QXmppCodec_p.h"
#include "QXmppRtpChannel.h"

#include <cstring>

#ifdef QXMPP_USE_SPEEX
#include <speex/speex.h>
#endif

#ifdef QXMPP_USE_OPUS
#include <opus/opus.h>
#endif

#ifdef QXMPP_USE_THEORA
#include <theora/theoradec.h>
#include <theora/theoraenc.h>
#endif

#ifdef QXMPP_USE_VPX
#define VPX_CODEC_DISABLE_COMPAT 1
#include <vpx/vpx_decoder.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
#include <vpx/vp8dx.h>
#endif

#define BIAS        (0x84)  /* Bias for linear code. */
#define CLIP        8159

#define SIGN_BIT    (0x80)  /* Sign bit for a A-law byte. */
#define QUANT_MASK  (0xf)   /* Quantization field mask. */
#define NSEGS       (8)     /* Number of A-law segments. */
#define SEG_SHIFT   (4)     /* Left shift for segment number. */
#define SEG_MASK    (0x70)  /* Segment field mask. */

// Distance (in frames) between two key frames (video only).
#define GOPSIZE 32

enum FragmentType {
    NoFragment = 0,
    StartFragment,
    MiddleFragment,
    EndFragment,
};

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
static quint8 linear2alaw(qint16 pcm_val)
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
static qint16 alaw2linear(quint8 a_val)
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
static quint8 linear2ulaw(qint16 pcm_val)
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
static qint16 ulaw2linear(quint8 u_val)
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

QXmppCodec::~QXmppCodec()
{
}

QXmppVideoDecoder::~QXmppVideoDecoder()
{
}

QXmppVideoEncoder::~QXmppVideoEncoder()
{
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

#ifdef QXMPP_USE_OPUS
QXmppOpusCodec::QXmppOpusCodec(int clockrate, int channels):
    sampleRate(clockrate),
    nChannels(channels)
{
    int error;
    encoder = opus_encoder_create(clockrate, channels, OPUS_APPLICATION_VOIP, &error);

    if (encoder || error == OPUS_OK) {
        // Add some options for error correction.
        opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(20));
        opus_encoder_ctl(encoder, OPUS_SET_DTX(1));
#ifdef OPUS_SET_PREDICTION_DISABLED
        opus_encoder_ctl(encoder, OPUS_SET_PREDICTION_DISABLED(1));
#endif
    }
    else
        qCritical() << "Opus encoder initialization error:" << opus_strerror(error);

    // Here, clockrate is synonym of sampleRate.
    decoder = opus_decoder_create(clockrate, channels, &error);

    if (!encoder || error != OPUS_OK)
        qCritical() << "Opus decoder initialization error:" << opus_strerror(error);

    // Opus only supports fixed frame durations from 2.5ms to 60ms.
    //
    // NOTE: https://mf4.xiph.org/jenkins/view/opus/job/opus/ws/doc/html/group__opus__encoder.html
    validFrameSize << 2.5e-3 << 5e-3 << 10e-3 << 20e-3 << 40e-3 << 60e-3;

    // so now, calculate the equivalent number of samples to process in each
    // frame.
    //
    // nSamples = t * sampleRate
    for (int i = 0; i < validFrameSize.size(); i++)
        validFrameSize[i] *= clockrate;

    // Maxmimum number of samples for the audio buffer.
    nSamples = validFrameSize.last();
}

QXmppOpusCodec::~QXmppOpusCodec()
{
    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = NULL;
    }

    if (decoder) {
        opus_decoder_destroy(decoder);
        decoder = NULL;
    }
}

qint64 QXmppOpusCodec::encode(QDataStream &input, QDataStream &output)
{
    // Read an audio frame.
    QByteArray pcm_buffer(input.device()->bytesAvailable(), 0);
    int length = input.readRawData(pcm_buffer.data(), pcm_buffer.size());

    // and append it to the sample buffer.
    sampleBuffer.append(pcm_buffer.left(length));

    // Get the maximum number of samples to encode. It must be a number
    // accepted by the Opus encoder
    int samples = readWindow(sampleBuffer.size());

    if (samples < 1)
        return 0;

    // The encoded stream is supposed to be smaller than the raw stream, so
    QByteArray opus_buffer(sampleBuffer.size(), 0);

    length = opus_encode(encoder,
                         (opus_int16 *) sampleBuffer.constData(),
                         samples,
                         (uchar *) opus_buffer.data(),
                         opus_buffer.size());

    if (length < 1)
        qWarning() << "Opus encoding error:" << opus_strerror(length);
    else
        // Write the encoded stream to the output.
        output.writeRawData(opus_buffer.constData(), length);

    // Remove the frame from the sample buffer.
    sampleBuffer.remove(0, samples * nChannels * 2);

    if (length < 1)
        return 0;

    return samples;
}

qint64 QXmppOpusCodec::decode(QDataStream &input, QDataStream &output)
{
    QByteArray opus_buffer(input.device()->bytesAvailable(), 0);
    int length = input.readRawData(opus_buffer.data(), opus_buffer.size());

    if (length < 1)
        return 0;

    // Audio frame is nSamples at maximum, so
    QByteArray pcm_buffer(nSamples * nChannels * 2, 0);

    // The last argumment must be 1 to enable FEC, but I don't why it results
    // in a SIGSEV.
    int samples = opus_decode(decoder,
                              (uchar *) opus_buffer.constData(),
                              length,
                              (opus_int16 *) pcm_buffer.data(),
                              pcm_buffer.size(),
                              0);

    if (samples < 1) {
        qWarning() << "Opus decoding error:" << opus_strerror(samples);

        return 0;
    }

    // Write the audio frame to the output.
    output.writeRawData(pcm_buffer.constData(), samples * nChannels * 2);

    return samples;
}

int QXmppOpusCodec::readWindow(int bufferSize)
{
    // WARNING: We are expecting 2 bytes signed samples, but this is wrong since
    // input stream can have a different sample formats.

    // Get the number of frames in the buffer.
    int samples = bufferSize / nChannels / 2;

    // Find an appropiate number of samples to read, according to Opus specs.
    for (int i = validFrameSize.size() - 1; i >= 0; i--)
        if (validFrameSize[i] <= samples)
            return validFrameSize[i];

    return 0;
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
    if (!ctx)
        return false;

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

    if (info.pixel_fmt == TH_PF_420) {
        if (!frame->isValid()) {
            const int bytes = ycbcr_buffer[0].stride * ycbcr_buffer[0].height
                            + ycbcr_buffer[1].stride * ycbcr_buffer[1].height
                            + ycbcr_buffer[2].stride * ycbcr_buffer[2].height;

            *frame = QXmppVideoFrame(bytes,
                QSize(ycbcr_buffer[0].width, ycbcr_buffer[0].height),
                ycbcr_buffer[0].stride,
                QXmppVideoFrame::Format_YUV420P);
        }
        uchar *output = frame->bits();
        for (int i = 0; i < 3; ++i) {
            const int length = ycbcr_buffer[i].stride * ycbcr_buffer[i].height;
            memcpy(output, ycbcr_buffer[i].data, length);
            output += length;
        }
        return true;
    } else if (info.pixel_fmt == TH_PF_422) {
        if (!frame->isValid()) {
            const int bytes = ycbcr_buffer[0].width * ycbcr_buffer[0].height * 2;

            *frame = QXmppVideoFrame(bytes,
                QSize(ycbcr_buffer[0].width, ycbcr_buffer[0].height),
                ycbcr_buffer[0].width * 2,
                QXmppVideoFrame::Format_YUYV);
        }

        // YUV 4:2:2 packing
        const int width = ycbcr_buffer[0].width;
        const int height = ycbcr_buffer[0].height;
        const int y_stride = ycbcr_buffer[0].stride;
        const int c_stride = ycbcr_buffer[1].stride;
        const uchar *y_row = ycbcr_buffer[0].data;
        const uchar *cb_row = ycbcr_buffer[1].data;
        const uchar *cr_row = ycbcr_buffer[2].data;
        uchar *output = frame->bits();
        for (int y = 0; y < height; ++y) {
            const uchar *y_ptr = y_row;
            const uchar *cb_ptr = cb_row;
            const uchar *cr_ptr = cr_row;
            for (int x = 0; x < width; x += 2) {
                *(output++) = *(y_ptr++);
                *(output++) = *(cb_ptr++);
                *(output++) = *(y_ptr++);
                *(output++) = *(cr_ptr++);
            }
            y_row += y_stride;
            cb_row += c_stride;
            cr_row += c_stride;
        }
        return true;
    } else {
        qWarning("Theora decoder received an unsupported frame format");
        return false;
    }
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
    if (d->info.pixel_fmt == TH_PF_420)
        format.setPixelFormat(QXmppVideoFrame::Format_YUV420P);
    else if (d->info.pixel_fmt == TH_PF_422)
        format.setPixelFormat(QXmppVideoFrame::Format_YUYV);
    else
        format.setPixelFormat(QXmppVideoFrame::Format_Invalid);
    if (d->info.fps_denominator > 0)
        format.setFrameRate(qreal(d->info.fps_numerator) / qreal(d->info.fps_denominator));
    return format;
}

QList<QXmppVideoFrame> QXmppTheoraDecoder::handlePacket(const QXmppRtpPacket &packet)
{
    QList<QXmppVideoFrame> frames;

    // theora deframing: draft-ietf-avt-rtp-theora-00
    QDataStream stream(packet.payload);
    quint32 theora_header;
    stream >> theora_header;

    quint32 theora_ident = (theora_header >> 8) & 0xffffff;
    Q_UNUSED(theora_ident);
    quint8 theora_frag = (theora_header & 0xc0) >> 6;
    quint8 theora_type = (theora_header & 0x30) >> 4;
    quint8 theora_packets = (theora_header & 0x0f);

    //qDebug("ident: 0x%08x, F: %d, TDT: %d, packets: %d", theora_ident, theora_frag, theora_type, theora_packets);

    // We only handle raw theora data
    if (theora_type != 0)
        return frames;

    QXmppVideoFrame frame;
    quint16 packetLength;

    if (theora_frag == NoFragment) {
        // unfragmented packet(s)
        for (int i = 0; i < theora_packets; ++i) {
            stream >> packetLength;
            if (packetLength > stream.device()->bytesAvailable()) {
                qWarning("Theora unfragmented packet has an invalid length");
                return frames;
            }

            d->packetBuffer.resize(packetLength);
            stream.readRawData(d->packetBuffer.data(), packetLength);
            if (d->decodeFrame(d->packetBuffer, &frame))
                frames << frame;
            d->packetBuffer.resize(0);
        }
    } else {
        // fragments
        stream >> packetLength;
        if (packetLength > stream.device()->bytesAvailable()) {
            qWarning("Theora packet has an invalid length");
            return frames;
        }

        int pos;
        if (theora_frag == StartFragment) {
            // start fragment
            pos = 0;
            d->packetBuffer.resize(packetLength);
        } else {
            // continuation or end fragment
            pos = d->packetBuffer.size();
            d->packetBuffer.resize(pos + packetLength);
        }
        stream.readRawData(d->packetBuffer.data() + pos, packetLength);

        if (theora_frag == EndFragment) {
            // end fragment
            if (d->decodeFrame(d->packetBuffer, &frame))
                frames << frame;
            d->packetBuffer.resize(0);
        }
    }
    return frames;
}

bool QXmppTheoraDecoder::setParameters(const QMap<QString, QString> &parameters)
{
    QByteArray config = QByteArray::fromBase64(parameters.value("configuration").toLatin1());
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

#ifdef QXMPP_DEBUG_THEORA
    qDebug("Theora frame_width %i, frame_height %i, colorspace %i, pixel_fmt: %i, target_bitrate: %i, quality: %i, keyframe_granule_shift: %i",
        d->info.frame_width,
        d->info.frame_height,
        d->info.colorspace,
        d->info.pixel_fmt,
        d->info.target_bitrate,
        d->info.quality,
        d->info.keyframe_granule_shift);
#endif
    if (d->info.pixel_fmt != TH_PF_420 && d->info.pixel_fmt != TH_PF_422) {
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
    void writeFragment(QDataStream &stream, FragmentType frag_type, quint8 theora_packets, const char *data, quint16 length);

    th_comment comment;
    th_info info;
    th_setup_info *setup_info;
    th_enc_ctx *ctx;
    th_ycbcr_buffer ycbcr_buffer;

    QByteArray buffer;
    QByteArray configuration;
    QByteArray ident;
};

void QXmppTheoraEncoderPrivate::writeFragment(QDataStream &stream, FragmentType frag_type, quint8 theora_packets, const char *data, quint16 length)
{
    // theora framing: draft-ietf-avt-rtp-theora-00
    const quint8 theora_type = 0; // raw data
    stream.writeRawData(ident.constData(), ident.size());
    stream << quint8(((frag_type << 6) & 0xc0) |
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
    const QXmppVideoFrame::PixelFormat pixelFormat = format.pixelFormat();
    if ((pixelFormat != QXmppVideoFrame::Format_YUV420P) &&
        (pixelFormat != QXmppVideoFrame::Format_YUYV)) {
        qWarning("Theora encoder does not support the given format");
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

    // FIXME: how do we handle floating point frame rates?
    d->info.fps_numerator = format.frameRate();
    d->info.fps_denominator = 1;

    if (pixelFormat == QXmppVideoFrame::Format_YUV420P) {
        d->info.pixel_fmt = TH_PF_420;
        d->ycbcr_buffer[0].width = d->info.frame_width;
        d->ycbcr_buffer[0].height = d->info.frame_height;
        d->ycbcr_buffer[1].width = d->ycbcr_buffer[0].width / 2;
        d->ycbcr_buffer[1].height = d->ycbcr_buffer[0].height / 2;
        d->ycbcr_buffer[2].width = d->ycbcr_buffer[1].width;
        d->ycbcr_buffer[2].height = d->ycbcr_buffer[1].height;
    } else if (pixelFormat == QXmppVideoFrame::Format_YUYV) {
        d->info.pixel_fmt = TH_PF_422;
        d->buffer.resize(d->info.frame_width * d->info.frame_height * 2);
        d->ycbcr_buffer[0].width = d->info.frame_width;
        d->ycbcr_buffer[0].height = d->info.frame_height;
        d->ycbcr_buffer[0].stride = d->info.frame_width;
        d->ycbcr_buffer[0].data = (uchar*) d->buffer.data();
        d->ycbcr_buffer[1].width = d->ycbcr_buffer[0].width / 2;
        d->ycbcr_buffer[1].height = d->ycbcr_buffer[0].height;
        d->ycbcr_buffer[1].stride = d->ycbcr_buffer[0].stride / 2;
        d->ycbcr_buffer[1].data = d->ycbcr_buffer[0].data + d->ycbcr_buffer[0].stride * d->ycbcr_buffer[0].height;
        d->ycbcr_buffer[2].width = d->ycbcr_buffer[1].width;
        d->ycbcr_buffer[2].height = d->ycbcr_buffer[1].height;
        d->ycbcr_buffer[2].stride = d->ycbcr_buffer[1].stride;
        d->ycbcr_buffer[2].data = d->ycbcr_buffer[1].data + d->ycbcr_buffer[1].stride * d->ycbcr_buffer[1].height;
    }

    // create encoder
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

    if (!d->ctx)
        return packets;

    if (d->info.pixel_fmt == TH_PF_420) {
        d->ycbcr_buffer[0].stride = frame.bytesPerLine();
        d->ycbcr_buffer[0].data = (unsigned char*) frame.bits();
        d->ycbcr_buffer[1].stride = d->ycbcr_buffer[0].stride / 2;
        d->ycbcr_buffer[1].data = d->ycbcr_buffer[0].data + d->ycbcr_buffer[0].stride * d->ycbcr_buffer[0].height;
        d->ycbcr_buffer[2].stride = d->ycbcr_buffer[1].stride;
        d->ycbcr_buffer[2].data = d->ycbcr_buffer[1].data + d->ycbcr_buffer[1].stride * d->ycbcr_buffer[1].height;
    } else if (d->info.pixel_fmt == TH_PF_422) {
        // YUV 4:2:2 unpacking
        const int width = frame.width();
        const int height = frame.height();
        const int stride = frame.bytesPerLine();
        const uchar *row = frame.bits();
        uchar *y_out = d->ycbcr_buffer[0].data;
        uchar *cb_out = d->ycbcr_buffer[1].data;
        uchar *cr_out = d->ycbcr_buffer[2].data;
        for (int y = 0; y < height; ++y) {
            const uchar *ptr = row;
            for (int x = 0; x < width; x += 2) {
                *(y_out++) = *(ptr++);
                *(cb_out++) = *(ptr++);
                *(y_out++) = *(ptr++);
                *(cr_out++) = *(ptr++);
            }
            row += stride;
        }
    } else {
        qWarning("Theora encoder received an unsupported frame format");
        return packets;
    }

    if (th_encode_ycbcr_in(d->ctx, d->ycbcr_buffer) != 0) {
        qWarning("Theora encoder could not handle frame");
        return packets;
    }

    QByteArray payload;
    ogg_packet packet;
    while (th_encode_packetout(d->ctx, 0, &packet) > 0) {
#ifdef QXMPP_DEBUG_THEORA
        qDebug("Theora encoded packet %d bytes", packet.bytes);
#endif
        QDataStream stream(&payload, QIODevice::WriteOnly);
        const char *data = (const char*) packet.packet;
        int size = packet.bytes;
        if (size <= PACKET_MAX) {
            // no fragmentation
            stream.device()->reset();
            payload.resize(0);
            d->writeFragment(stream, NoFragment, 1, data, size);
            packets << payload;
       } else {
            // fragmentation
            FragmentType frag_type = StartFragment;
            while (size) {
                const int length = qMin(PACKET_MAX, size);
                stream.device()->reset();
                payload.resize(0);
                d->writeFragment(stream, frag_type, 0, data, length);
                data += length;
                size -= length;
                frag_type = (size > PACKET_MAX) ? MiddleFragment : EndFragment;
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

#ifdef QXMPP_USE_VPX

class QXmppVpxDecoderPrivate
{
public:
    bool decodeFrame(const QByteArray &buffer, QXmppVideoFrame *frame);

    vpx_codec_ctx_t codec;
    QByteArray packetBuffer;
};

bool QXmppVpxDecoderPrivate::decodeFrame(const QByteArray &buffer, QXmppVideoFrame *frame)
{
    // With the VPX_DL_REALTIME option, tries to decode the frame as quick as
    // possible, if not possible discard it.
    if (vpx_codec_decode(&codec,
                         (const uint8_t*)buffer.constData(),
                         buffer.size(),
                         NULL,
                         VPX_DL_REALTIME) != VPX_CODEC_OK) {
        qWarning("Vpx packet could not be decoded: %s", vpx_codec_error_detail(&codec));
        return false;
    }

    vpx_codec_iter_t iter = NULL;
    vpx_image_t *img;
    while ((img = vpx_codec_get_frame(&codec, &iter))) {
        if (img->fmt == VPX_IMG_FMT_I420) {
            if (!frame->isValid()) {
                const int bytes = img->d_w * img->d_h * 3 / 2;

                *frame = QXmppVideoFrame(bytes,
                    QSize(img->d_w, img->d_h),
                    img->d_w,
                    QXmppVideoFrame::Format_YUV420P);
            }
            uchar *output = frame->bits();

            for (int i = 0; i < 3; ++i) {
                uchar *input = img->planes[i];
                const int div = (i == 0) ? 1 : 2;
                for (unsigned int y = 0; y < img->d_h / div; ++y) {
                    memcpy(output, input, img->d_w / div);
                    input += img->stride[i];
                    output += img->d_w / div;
                }
            }
        } else {
            qWarning("Vpx decoder received an unsupported frame format: %d", img->fmt);
        }
    }

    return true;
}

QXmppVpxDecoder::QXmppVpxDecoder()
{
    d = new QXmppVpxDecoderPrivate;
    vpx_codec_flags_t flags = 0;

    // Enable FEC if codec support it.
    if (vpx_codec_get_caps(vpx_codec_vp8_dx()) & VPX_CODEC_CAP_ERROR_CONCEALMENT)
        flags |= VPX_CODEC_USE_ERROR_CONCEALMENT;

    if (vpx_codec_dec_init(&d->codec,
                           vpx_codec_vp8_dx(),
                           NULL,
                           flags) != VPX_CODEC_OK) {
        qWarning("Vpx decoder could not be initialised");
    }
}

QXmppVpxDecoder::~QXmppVpxDecoder()
{
    vpx_codec_destroy(&d->codec);
    delete d;
}

QXmppVideoFormat QXmppVpxDecoder::format() const
{
    QXmppVideoFormat format;
    format.setFrameRate(15.0);
    format.setFrameSize(QSize(320, 240));
    format.setPixelFormat(QXmppVideoFrame::Format_YUV420P);
    return format;
}

QList<QXmppVideoFrame> QXmppVpxDecoder::handlePacket(const QXmppRtpPacket &packet)
{
    QList<QXmppVideoFrame> frames;

    // vp8 deframing: http://tools.ietf.org/html/draft-westin-payload-vp8-00
    QDataStream stream(packet.payload);
    quint8 vpx_header;
    stream >> vpx_header;

    const bool have_id = (vpx_header & 0x10) != 0;
    const quint8 frag_type = (vpx_header & 0x6) >> 1;
    if (have_id) {
        qWarning("Vpx decoder does not support pictureId yet");
        return frames;
    }

    const int packetLength = packet.payload.size() - 1;
#ifdef QXMPP_DEBUG_VPX
    qDebug("Vpx fragment FI: %d, size %d", frag_type, packetLength);
#endif

    QXmppVideoFrame frame;
    static quint16 sequence = 0;

    // If the incomming packet sequence is wrong discard all packets until a
    // complete keyframe arrives.
    // If a partition of a keyframe is missing, discard it until a next
    // keyframe.
    //
    // NOTE: https://tools.ietf.org/html/draft-ietf-payload-vp8-13#section-4.3
    // Sections: 4.3, 4.5, 4.5.1

    if (frag_type == NoFragment) {
        // unfragmented packet
        if ((packet.payload[1] & 0x1) == 0 // is key frame
            || packet.sequence == sequence) {
            if (d->decodeFrame(packet.payload.mid(1), &frame))
                frames << frame;

            sequence = packet.sequence + 1;
        }

        d->packetBuffer.resize(0);
    } else {
        // fragments
        if (frag_type == StartFragment) {
            // start fragment
            if ((packet.payload[1] & 0x1) == 0 // is key frame
                || packet.sequence == sequence) {
                d->packetBuffer = packet.payload.mid(1);
                sequence = packet.sequence + 1;
            }
        } else {
            // continuation or end fragment
            if (packet.sequence == sequence) {
                const int packetPos = d->packetBuffer.size();
                d->packetBuffer.resize(packetPos + packetLength);
                stream.readRawData(d->packetBuffer.data() + packetPos, packetLength);

                if (frag_type == EndFragment) {
                    // end fragment
                    if (d->decodeFrame(d->packetBuffer, &frame)) {
                        frames << frame;
                        d->packetBuffer.resize(0);
                    }
                }

                sequence++;
            }
        }
    }

    return frames;
}

bool QXmppVpxDecoder::setParameters(const QMap<QString, QString> &parameters)
{
    return true;
}

class QXmppVpxEncoderPrivate
{
public:
    void writeFragment(QDataStream &stream, FragmentType frag_type, const char *data, quint16 length);

    vpx_codec_ctx_t codec;
    vpx_codec_enc_cfg_t cfg;
    vpx_image_t *imageBuffer;
    int frameCount;
};

void QXmppVpxEncoderPrivate::writeFragment(QDataStream &stream, FragmentType frag_type, const char *data, quint16 length)
{
    // vp8 framing: http://tools.ietf.org/html/draft-westin-payload-vp8-00
#ifdef QXMPP_DEBUG_VPX
    qDebug("Vpx encoder writing packet frag: %i, size: %u", frag_type, length);
#endif
    stream << quint8(((frag_type << 1) & 0x6) |
                      (frag_type == NoFragment || frag_type == StartFragment));
    stream.writeRawData(data, length);
}

QXmppVpxEncoder::QXmppVpxEncoder(uint clockrate)
{
    d = new QXmppVpxEncoderPrivate;
    d->frameCount = 0;
    d->imageBuffer = 0;
    vpx_codec_enc_config_default(vpx_codec_vp8_cx(), &d->cfg, 0);

    // Set the encoding threads number to use
    int nThreads = QThread::idealThreadCount();

    if (nThreads > 0)
        d->cfg.g_threads = nThreads - 1;

    // Make stream error resiliant
    d->cfg.g_error_resilient = VPX_ERROR_RESILIENT_DEFAULT
                               | VPX_ERROR_RESILIENT_PARTITIONS;

    d->cfg.g_pass = VPX_RC_ONE_PASS;
    d->cfg.kf_mode = VPX_KF_AUTO;

    // Reduce GOP size
    if (d->cfg.kf_max_dist > GOPSIZE)
        d->cfg.kf_max_dist = GOPSIZE;

    // Here, clockrate is synonym of bitrate.
    d->cfg.rc_target_bitrate = clockrate / 1000;
}

QXmppVpxEncoder::~QXmppVpxEncoder()
{
    vpx_codec_destroy(&d->codec);
    if (d->imageBuffer)
        vpx_img_free(d->imageBuffer);
    delete d;
}

bool QXmppVpxEncoder::setFormat(const QXmppVideoFormat &format)
{
    const QXmppVideoFrame::PixelFormat pixelFormat = format.pixelFormat();
    if (pixelFormat != QXmppVideoFrame::Format_YUYV) {
        qWarning("Vpx encoder does not support the given format");
        return false;
    }
    d->cfg.g_w = format.frameSize().width();
    d->cfg.g_h = format.frameSize().height();
    if (vpx_codec_enc_init(&d->codec, vpx_codec_vp8_cx(), &d->cfg, 0) != VPX_CODEC_OK) {
        qWarning("Vpx encoder could not be initialised");
        return false;
    }

    d->imageBuffer = vpx_img_alloc(NULL, VPX_IMG_FMT_I420,
            format.frameSize().width(), format.frameSize().height(), 1);
    return true;
}

QList<QByteArray> QXmppVpxEncoder::handleFrame(const QXmppVideoFrame &frame)
{
    const int PACKET_MAX = 1388;
    QList<QByteArray> packets;

    // try to encode frame
    if (frame.pixelFormat() == QXmppVideoFrame::Format_YUYV) {
        // YUYV -> YUV420P
        const int width = frame.width();
        const int height = frame.height();
        const int stride = frame.bytesPerLine();
        const uchar *row = frame.bits();
        uchar *y_row = d->imageBuffer->planes[VPX_PLANE_Y];
        uchar *cb_row = d->imageBuffer->planes[VPX_PLANE_U];
        uchar *cr_row = d->imageBuffer->planes[VPX_PLANE_V];
        for (int y = 0; y < height; y += 2) {
            // odd row
            const uchar *ptr = row;
            uchar *y_out = y_row;
            uchar *cb_out = cb_row;
            uchar *cr_out = cr_row;
            for (int x = 0; x < width; x += 2) {
                *(y_out++) = *(ptr++);
                *(cb_out++) = *(ptr++);
                *(y_out++) = *(ptr++);
                *(cr_out++) = *(ptr++);
            }
            row += stride;
            y_row += d->imageBuffer->stride[VPX_PLANE_Y];
            cb_row += d->imageBuffer->stride[VPX_PLANE_U];
            cr_row += d->imageBuffer->stride[VPX_PLANE_V];

            // even row
            ptr = row;
            y_out = y_row;
            for (int x = 0; x < width; x += 2) {
                *(y_out++) = *(ptr++);
                ptr++;
                *(y_out++) = *(ptr++);
                ptr++;
            }
            row += stride;
            y_row += d->imageBuffer->stride[VPX_PLANE_Y];
        }
    } else {
        qWarning("Vpx encoder does not support the given format");
        return packets;
    }

    if (vpx_codec_encode(&d->codec, d->imageBuffer, d->frameCount, 1,  0, VPX_DL_REALTIME) != VPX_CODEC_OK) {
        qWarning("Vpx encoder could not handle frame: %s", vpx_codec_error_detail(&d->codec));
        return packets;
    }

    // extract data
    QByteArray payload;
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t *pkt;
    while ((pkt = vpx_codec_get_cx_data(&d->codec, &iter))) {
        if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
#ifdef QXMPP_DEBUG_VPX
            qDebug("Vpx encoded packet %lu bytes", pkt->data.frame.sz);
#endif
            QDataStream stream(&payload, QIODevice::WriteOnly);
            const char *data = (const char*) pkt->data.frame.buf;
            int size = pkt->data.frame.sz;
            if (size <= PACKET_MAX) {
                // no fragmentation
                stream.device()->reset();
                payload.resize(0);
                d->writeFragment(stream, NoFragment, data, size);
                packets << payload;
           } else {
                // fragmentation
                FragmentType frag_type = StartFragment;
                while (size) {
                    const int length = qMin(PACKET_MAX, size);
                    stream.device()->reset();
                    payload.resize(0);
                    d->writeFragment(stream, frag_type, data, length);
                    data += length;
                    size -= length;
                    frag_type = (size > PACKET_MAX) ? MiddleFragment : EndFragment;
                    packets << payload;
                }
            }
        }
    }
    d->frameCount++;

    return packets;
}

QMap<QString, QString> QXmppVpxEncoder::parameters() const
{
    return QMap<QString, QString>();
}

#endif
