/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Niels Ole Salscheider
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

#ifndef QXMPPCALLSTREAM_P_H
#define QXMPPCALLSTREAM_P_H

#include "QXmppCall_p.h"
#include "QXmppJingleIq.h"

#include <gst/gst.h>

#include <QList>
#include <QObject>
#include <QString>

class QXmppIceConnection;

//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

static const int RTP_COMPONENT = 1;
static const int RTCP_COMPONENT = 2;

static const QLatin1String AUDIO_MEDIA("audio");
static const QLatin1String VIDEO_MEDIA("video");

class QXmppCallStreamPrivate : public QObject
{
    Q_OBJECT

public:
    QXmppCallStreamPrivate(QXmppCallStream *parent, GstElement *pipeline_, GstElement *rtpbin_,
                           QString media_, QString creator_, QString name_, int id_);
    ~QXmppCallStreamPrivate();

    GstFlowReturn sendDatagram(GstElement *appsink, int component);
    void datagramReceived(const QByteArray &datagram, GstElement *appsrc);

    void addEncoder(QXmppCallPrivate::GstCodec &codec);
    void addDecoder(GstPad *pad, QXmppCallPrivate::GstCodec &codec);
    void addRtpSender(GstPad *pad);
    void addRtcpSender(GstPad *pad);

    QXmppCallStream *q;

    quint32 localSsrc;

    GstElement *pipeline;
    GstElement *rtpbin;
    GstPad *sendPad;
    GstPad *receivePad;
    GstPad *internalReceivePad;
    GstPad *internalRtpPad;
    GstPad *internalRtcpPad;
    GstElement *encoderBin;
    GstElement *decoderBin;
    GstElement *iceReceiveBin;
    GstElement *iceSendBin;
    GstElement *apprtpsrc;
    GstElement *apprtcpsrc;
    GstElement *apprtpsink;
    GstElement *apprtcpsink;

    std::function<void(GstPad *)> sendPadCB;
    std::function<void(GstPad *)> receivePadCB;

    QXmppIceConnection *connection;
    QString media;
    QString creator;
    QString name;
    int id;

    QList<QXmppJinglePayloadType> payloadTypes;
};

#endif
