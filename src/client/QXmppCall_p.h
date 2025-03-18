// SPDX-FileCopyrightText: 2019 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCALL_P_H
#define QXMPPCALL_P_H

#include "QXmppCall.h"
#include "QXmppJingleIq.h"

#include <gst/gst.h>

#include <QList>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

class QXmppCallStream;

class QXmppCallPrivate : public QObject
{
    Q_OBJECT
public:
    struct GstCodec {
        int pt;
        QString name;
        int channels;
        uint clockrate;
        QString gstPay;
        QString gstDepay;
        QString gstEnc;
        QString gstDec;
        struct Property {
            QString name;
            int value;
        };
        // Use e.g. gst-inspect-1.0 x264enc to find good encoder settings for live streaming
        QList<Property> encProps;
    };

    QXmppCallPrivate(QXmppCall *qq);
    ~QXmppCallPrivate();

    void ssrcActive(uint sessionId, uint ssrc);
    void padAdded(GstPad *pad);
    GstCaps *ptMap(uint sessionId, uint pt);
    static bool isFormatSupported(const QString &codecName);
    static bool isCodecSupported(const GstCodec &codec);
    static void filterGStreamerFormats(QList<GstCodec> &formats);

    QXmppCallStream *createStream(const QString &media, const QString &creator, const QString &name);
    QXmppCallStream *findStreamByMedia(QStringView media);
    QXmppCallStream *findStreamByName(QStringView name);
    QXmppCallStream *findStreamById(int id);
    QXmppJingleIq::Content localContent(QXmppCallStream *stream) const;

    void handleAck(const QXmppIq &iq);
    bool handleDescription(QXmppCallStream *stream, const QXmppJingleIq::Content &content);
    void handleRequest(const QXmppJingleIq &iq);
    bool handleTransport(QXmppCallStream *stream, const QXmppJingleIq::Content &content);
    void setState(QXmppCall::State state);
    bool sendAck(const QXmppJingleIq &iq);
    bool sendInvite();
    bool sendRequest(const QXmppJingleIq &iq);
    void terminate(QXmppJingleIq::Reason::Type reasonType);

    QXmppCall::Direction direction;
    QString jid;
    QString ownJid;
    QXmppCallManager *manager;
    QList<QXmppJingleIq> requests;
    QString sid;
    QXmppCall::State state;

    GstElement *pipeline;
    GstElement *rtpbin;

    // Media streams
    QList<QXmppCallStream *> streams;
    int nextId;

    // Supported codecs
    QList<GstCodec> videoCodecs = {
        { .pt = 100, .name = QStringLiteral("H264"), .channels = 1, .clockrate = 90000, .gstPay = QStringLiteral("rtph264pay"), .gstDepay = QStringLiteral("rtph264depay"), .gstEnc = QStringLiteral("x264enc"), .gstDec = QStringLiteral("avdec_h264"), .encProps = { { QStringLiteral("tune"), 4 }, { QStringLiteral("speed-preset"), 3 }, { QStringLiteral("byte-stream"), true }, { QStringLiteral("bitrate"), 512 } } },
        { .pt = 99, .name = QStringLiteral("VP8"), .channels = 1, .clockrate = 90000, .gstPay = QStringLiteral("rtpvp8pay"), .gstDepay = QStringLiteral("rtpvp8depay"), .gstEnc = QStringLiteral("vp8enc"), .gstDec = QStringLiteral("vp8dec"), .encProps = { { QStringLiteral("deadline"), 20000 }, { QStringLiteral("target-bitrate"), 512000 } } },
        // vp9enc and x265enc seem to be very slow. Give them a lower priority for now.
        { .pt = 102, .name = QStringLiteral("H265"), .channels = 1, .clockrate = 90000, .gstPay = QStringLiteral("rtph265pay"), .gstDepay = QStringLiteral("rtph265depay"), .gstEnc = QStringLiteral("x265enc"), .gstDec = QStringLiteral("avdec_h265"), .encProps = { { QStringLiteral("tune"), 4 }, { QStringLiteral("speed-preset"), 3 }, { QStringLiteral("bitrate"), 512 } } },
        { .pt = 101, .name = QStringLiteral("VP9"), .channels = 1, .clockrate = 90000, .gstPay = QStringLiteral("rtpvp9pay"), .gstDepay = QStringLiteral("rtpvp9depay"), .gstEnc = QStringLiteral("vp9enc"), .gstDec = QStringLiteral("vp9dec"), .encProps = { { QStringLiteral("deadline"), 20000 }, { QStringLiteral("target-bitrate"), 512000 } } }
    };

    QList<GstCodec> audioCodecs = {
        { .pt = 98, .name = QStringLiteral("OPUS"), .channels = 2, .clockrate = 48000, .gstPay = QStringLiteral("rtpopuspay"), .gstDepay = QStringLiteral("rtpopusdepay"), .gstEnc = QStringLiteral("opusenc"), .gstDec = QStringLiteral("opusdec") },
        { .pt = 98, .name = QStringLiteral("OPUS"), .channels = 1, .clockrate = 48000, .gstPay = QStringLiteral("rtpopuspay"), .gstDepay = QStringLiteral("rtpopusdepay"), .gstEnc = QStringLiteral("opusenc"), .gstDec = QStringLiteral("opusdec") },
        { .pt = 97, .name = QStringLiteral("SPEEX"), .channels = 1, .clockrate = 48000, .gstPay = QStringLiteral("rtpspeexpay"), .gstDepay = QStringLiteral("rtpspeexdepay"), .gstEnc = QStringLiteral("speexenc"), .gstDec = QStringLiteral("speexdec") },
        { .pt = 97, .name = QStringLiteral("SPEEX"), .channels = 1, .clockrate = 44100, .gstPay = QStringLiteral("rtpspeexpay"), .gstDepay = QStringLiteral("rtpspeexdepay"), .gstEnc = QStringLiteral("speexenc"), .gstDec = QStringLiteral("speexdec") },
        { .pt = 96, .name = QStringLiteral("AAC"), .channels = 2, .clockrate = 48000, .gstPay = QStringLiteral("rtpmp4apay"), .gstDepay = QStringLiteral("rtpmp4adepay"), .gstEnc = QStringLiteral("avenc_aac"), .gstDec = QStringLiteral("avdec_aac") },
        { .pt = 96, .name = QStringLiteral("AAC"), .channels = 2, .clockrate = 44100, .gstPay = QStringLiteral("rtpmp4apay"), .gstDepay = QStringLiteral("rtpmp4adepay"), .gstEnc = QStringLiteral("avenc_aac"), .gstDec = QStringLiteral("avdec_aac") },
        { .pt = 96, .name = QStringLiteral("AAC"), .channels = 1, .clockrate = 48000, .gstPay = QStringLiteral("rtpmp4apay"), .gstDepay = QStringLiteral("rtpmp4adepay"), .gstEnc = QStringLiteral("avenc_aac"), .gstDec = QStringLiteral("avdec_aac") },
        { .pt = 96, .name = QStringLiteral("AAC"), .channels = 1, .clockrate = 44100, .gstPay = QStringLiteral("rtpmp4apay"), .gstDepay = QStringLiteral("rtpmp4adepay"), .gstEnc = QStringLiteral("avenc_aac"), .gstDec = QStringLiteral("avdec_aac") },
        { .pt = 8, .name = QStringLiteral("PCMA"), .channels = 1, .clockrate = 8000, .gstPay = QStringLiteral("rtppcmapay"), .gstDepay = QStringLiteral("rtppcmadepay"), .gstEnc = QStringLiteral("alawenc"), .gstDec = QStringLiteral("alawdec") },
        { .pt = 0, .name = QStringLiteral("PCMU"), .channels = 1, .clockrate = 8000, .gstPay = QStringLiteral("rtppcmupay"), .gstDepay = QStringLiteral("rtppcmudepay"), .gstEnc = QStringLiteral("mulawenc"), .gstDec = QStringLiteral("mulawdec") }
    };

private:
    QXmppCall *q;
};

#endif
