/*
 * Copyright (C) 2020 The QXmpp developers
 *
 * Author:
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

#include "QXmppCallStream.h"

#include "QXmppCallStream_p.h"
#include "QXmppCall_p.h"
#include "QXmppStun.h"

#include <cstring>
#include <gst/gst.h>

#include <QSslCertificate>
#include <QUuid>

QXmppCallStreamPrivate::QXmppCallStreamPrivate(QXmppCallStream *parent, GstElement *pipeline_,
                                               GstElement *rtpbin_, QString media_, QString creator_,
                                               QString name_, int id_, bool useDtls_)
    : QObject(parent),
      q(parent),
      pipeline(pipeline_),
      rtpbin(rtpbin_),
      sendPad(nullptr),
      receivePad(nullptr),
      encoderBin(nullptr),
      decoderBin(nullptr),
      sendPadCB(nullptr),
      receivePadCB(nullptr),
      media(media_),
      creator(creator_),
      name(name_),
      id(id_),
      useDtls(useDtls_),
      dtlsHandshakeComplete(false)
{
    localSsrc = qrand();

    iceReceiveBin = gst_bin_new(QStringLiteral("receive_%1").arg(id).toLatin1().data());
    iceSendBin = gst_bin_new(QStringLiteral("send_%1").arg(id).toLatin1().data());
    gst_bin_add_many(GST_BIN(pipeline), iceReceiveBin, iceSendBin, nullptr);

    internalRtpPad = gst_ghost_pad_new_no_target(nullptr, GST_PAD_SINK);
    internalRtcpPad = gst_ghost_pad_new_no_target(nullptr, GST_PAD_SINK);
    if (!gst_element_add_pad(iceSendBin, internalRtpPad) ||
        !gst_element_add_pad(iceSendBin, internalRtcpPad)) {
        qFatal("Failed to add pads to send bin");
    }

    /* Create DTLS SRTP elements */
    if (useDtls) {
        QString dtlsRtpId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString dtlsRtcpId = QUuid::createUuid().toString(QUuid::WithoutBraces);

        dtlsrtpdecoder = gst_element_factory_make("dtlssrtpdec", nullptr);
        dtlsrtcpdecoder = gst_element_factory_make("dtlssrtpdec", nullptr);
        if (!dtlsrtpdecoder || !dtlsrtcpdecoder) {
            qFatal("Failed to create dtls srtp decoders");
        }

        g_object_set(dtlsrtpdecoder, "async-handling", true, "connection-id", dtlsRtpId.toLatin1().data(), nullptr);
        g_object_set(dtlsrtcpdecoder, "async-handling", true, "connection-id", dtlsRtcpId.toLatin1().data(), nullptr);
        gchar *pem;
        g_object_get(dtlsrtpdecoder, "pem", &pem, nullptr);
        /* Copy the certificate to the RTCP decoder so that they both share the same fingerprint. */
        //g_object_set(dtlsrtcpdecoder, "pem", pem, nullptr); //TODO why does this fail?
        /* Calculate the fingerprint to transmit to the remote party. */
        QSslCertificate certificate(pem);
        digest = certificate.digest(QCryptographicHash::Sha256);
        g_free(pem);

        dtlsrtpencoder = gst_element_factory_make("dtlssrtpenc", nullptr);
        dtlsrtcpencoder = gst_element_factory_make("dtlssrtpenc", nullptr);
        if (!dtlsrtpencoder || !dtlsrtcpencoder) {
            qFatal("Failed to create dtls srtp encoders");
        }

        g_object_set(dtlsrtpencoder, "async-handling", true, "connection-id", dtlsRtpId.toLatin1().data(), "is-client", false, nullptr);
        g_object_set(dtlsrtcpencoder, "async-handling", true, "connection-id", dtlsRtcpId.toLatin1().data(), "is-client", false, nullptr);

        g_signal_connect_swapped(dtlsrtpencoder, "on-key-set",
                                 G_CALLBACK(+[](QXmppCallStreamPrivate *p) {
                                     // TODO check remote fingerprint (peer-pem on decoders)
                                     qWarning("================ ON_KEY_SET ==============");
                                     p->dtlsHandshakeComplete = true;
                                     if (p->sendPadCB && p->encoderBin) {
                                         p->sendPadCB(p->sendPad);
                                     }
                                     if (p->receivePadCB && p->decoderBin) {
                                         p->receivePadCB(p->receivePad);
                                     }
                                 }),
                                 this);

        if (!gst_bin_add(GST_BIN(iceReceiveBin), dtlsrtpdecoder) ||
            !gst_bin_add(GST_BIN(iceReceiveBin), dtlsrtcpdecoder) ||
            !gst_bin_add(GST_BIN(iceSendBin), dtlsrtpencoder) ||
            !gst_bin_add(GST_BIN(iceSendBin), dtlsrtcpencoder)) {
            qFatal("Failed to add dtls elements to corresponding bins");
        }
    }

    /* Create appsrc / appsink elements */
    connection = new QXmppIceConnection(this);
    connection->addComponent(RTP_COMPONENT);
    connection->addComponent(RTCP_COMPONENT);
    apprtpsink = gst_element_factory_make("appsink", nullptr);
    apprtcpsink = gst_element_factory_make("appsink", nullptr);
    if (!apprtpsink || !apprtcpsink) {
        qFatal("Failed to create appsinks");
    }

    g_signal_connect_swapped(apprtpsink, "new-sample",
                             G_CALLBACK(+[](QXmppCallStreamPrivate *p, GstElement *appsink) -> GstFlowReturn {
                                 return p->sendDatagram(appsink, RTP_COMPONENT);
                             }),
                             this);
    g_signal_connect_swapped(apprtcpsink, "new-sample",
                             G_CALLBACK(+[](QXmppCallStreamPrivate *p, GstElement *appsink) -> GstFlowReturn {
                                 return p->sendDatagram(appsink, RTCP_COMPONENT);
                             }),
                             this);

    apprtpsrc = gst_element_factory_make("appsrc", nullptr);
    apprtcpsrc = gst_element_factory_make("appsrc", nullptr);
    if (!apprtpsrc || !apprtcpsrc) {
        qFatal("Failed to create appsrcs");
    }

    // TODO check these parameters
    g_object_set(apprtpsink, "emit-signals", true, "async", false, "max-buffers", 1, "drop", true, nullptr);
    g_object_set(apprtcpsink, "emit-signals", true, "async", false, nullptr);
    g_object_set(apprtpsrc, "is-live", true, "max-latency", 5000000, nullptr);
    g_object_set(apprtcpsrc, "is-live", true, nullptr);

    connect(connection->component(RTP_COMPONENT), &QXmppIceComponent::datagramReceived,
            [&](const QByteArray &datagram) { datagramReceived(datagram, apprtpsrc); });
    connect(connection->component(RTCP_COMPONENT), &QXmppIceComponent::datagramReceived,
            [&](const QByteArray &datagram) { datagramReceived(datagram, apprtcpsrc); });

    if (!gst_bin_add(GST_BIN(iceReceiveBin), apprtpsrc) ||
        !gst_bin_add(GST_BIN(iceReceiveBin), apprtcpsrc) ||
        !gst_bin_add(GST_BIN(iceSendBin), apprtpsink) ||
        !gst_bin_add(GST_BIN(iceSendBin), apprtcpsink)) {
        qFatal("Failed to add appsrc / appsink elements to respective bins");
    }

    /* Trigger creation of necessary pads */
    GstPad *dummyPad = gst_element_get_request_pad(rtpbin, QStringLiteral("send_rtp_sink_%1").arg(id).toLatin1().data());
    gst_object_unref(dummyPad);

    /* Link pads - receiving side */
    GstPad *rtpRecvPad = gst_element_get_static_pad(apprtpsrc, "src");
    GstPad *rtcpRecvPad = gst_element_get_static_pad(apprtcpsrc, "src");

    if (useDtls) {
        GstPad *dtlsRtpSinkPad = gst_element_get_static_pad(dtlsrtpdecoder, "sink");
        GstPad *dtlsRtcpSinkPad = gst_element_get_static_pad(dtlsrtcpdecoder, "sink");
        gst_pad_link(rtpRecvPad, dtlsRtpSinkPad);
        gst_pad_link(rtcpRecvPad, dtlsRtcpSinkPad);
        gst_object_unref(dtlsRtpSinkPad);
        gst_object_unref(dtlsRtcpSinkPad);
        gst_object_unref(rtpRecvPad);
        gst_object_unref(rtcpRecvPad);
        rtpRecvPad = gst_element_get_static_pad(dtlsrtpdecoder, "rtp_src");
        rtcpRecvPad = gst_element_get_static_pad(dtlsrtcpdecoder, "rtcp_src");
    }

    GstPad *rtpSinkPad = gst_element_get_request_pad(rtpbin, QStringLiteral("recv_rtp_sink_%1").arg(id).toLatin1().data());
    GstPad *rtcpSinkPad = gst_element_get_request_pad(rtpbin, QStringLiteral("recv_rtp_sink_%1").arg(id).toLatin1().data());
    gst_pad_link(rtpRecvPad, rtpSinkPad);
    gst_pad_link(rtcpRecvPad, rtcpSinkPad);
    gst_object_unref(rtpSinkPad);
    gst_object_unref(rtcpSinkPad);
    gst_object_unref(rtpRecvPad);
    gst_object_unref(rtcpRecvPad);

    /* Link pads - sending side */
    GstPad *rtpSendPad = gst_element_get_static_pad(apprtpsink, "sink");
    GstPad *rtcpSendPad = gst_element_get_static_pad(apprtcpsink, "sink");

    if (useDtls) {
        GstPad *dtlsRtpSrcPad = gst_element_get_static_pad(dtlsrtpencoder, "src");
        GstPad *dtlsRtcpSrcPad = gst_element_get_static_pad(dtlsrtcpencoder, "src");
        gst_pad_link(dtlsRtpSrcPad, rtpSendPad);
        gst_pad_link(dtlsRtcpSrcPad, rtcpSendPad);
        gst_object_unref(dtlsRtpSrcPad);
        gst_object_unref(dtlsRtcpSrcPad);
        gst_object_unref(rtpSendPad);
        gst_object_unref(rtcpSendPad);
        rtpSendPad = gst_element_get_request_pad(dtlsrtpencoder, QStringLiteral("rtp_sink_%1").arg(id).toLatin1().data());
        rtcpSendPad = gst_element_get_request_pad(dtlsrtcpencoder, QStringLiteral("rtcp_sink_%1").arg(id).toLatin1().data());
    }

    if (!gst_ghost_pad_set_target(GST_GHOST_PAD(internalRtpPad), rtpSendPad) ||
        !gst_ghost_pad_set_target(GST_GHOST_PAD(internalRtcpPad), rtcpSendPad)) {
        qFatal("Failed to link rtp send pads to internal ghost pads");
    }
    gst_object_unref(rtpSendPad);
    gst_object_unref(rtcpSendPad);

    // We need frequent RTCP reports for the bandwidth controller
    GstElement *rtpSession;
    g_signal_emit_by_name(rtpbin, "get-session", static_cast<uint>(id), &rtpSession);
    g_object_set(rtpSession, "rtcp-min-interval", 100000000, nullptr);

    gst_element_sync_state_with_parent(iceReceiveBin);
    gst_element_sync_state_with_parent(iceSendBin);

    GstPad *rtpbinRtpSendPad = gst_element_get_static_pad(rtpbin, QStringLiteral("send_rtp_src_%1").arg(id).toLatin1().data());
    GstPad *rtpbinRtcpSendPad = gst_element_get_request_pad(rtpbin, QStringLiteral("send_rtcp_src_%1").arg(id).toLatin1().data());
    if (gst_pad_link(rtpbinRtpSendPad, internalRtpPad) != GST_PAD_LINK_OK ||
        gst_pad_link(rtpbinRtcpSendPad, internalRtcpPad) != GST_PAD_LINK_OK) {
        qFatal("Failed to link rtp pads");
    }
    gst_object_unref(rtpbinRtpSendPad);
    gst_object_unref(rtpbinRtcpSendPad);
}

QXmppCallStreamPrivate::~QXmppCallStreamPrivate()
{
    connection->close();

    // Remove elements from pipeline
    if ((encoderBin && !gst_bin_remove(GST_BIN(pipeline), encoderBin)) ||
        (decoderBin && !gst_bin_remove(GST_BIN(pipeline), decoderBin)) ||
        !gst_bin_remove(GST_BIN(pipeline), iceSendBin) ||
        !gst_bin_remove(GST_BIN(pipeline), iceReceiveBin)) {
        qFatal("Failed to remove bins from pipeline");
    }
}

GstFlowReturn QXmppCallStreamPrivate::sendDatagram(GstElement *appsink, int component)
{
    GstSample *sample;
    g_signal_emit_by_name(appsink, "pull-sample", &sample);
    if (!sample) {
        qFatal("Could not get sample");
        return GST_FLOW_ERROR;
    }

    GstMapInfo mapInfo;
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    if (!buffer) {
        qFatal("Could not get buffer");
        return GST_FLOW_ERROR;
    }
    if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
        qFatal("Could not map buffer");
        return GST_FLOW_ERROR;
    }
    QByteArray datagram;
    datagram.resize(mapInfo.size);
    std::memcpy(datagram.data(), mapInfo.data, mapInfo.size);
    gst_buffer_unmap(buffer, &mapInfo);
    gst_sample_unref(sample);

    if (connection->component(component)->isConnected() &&
        connection->component(component)->sendDatagram(datagram) != datagram.size()) {
        return GST_FLOW_ERROR;
    }
    return GST_FLOW_OK;
}

void QXmppCallStreamPrivate::datagramReceived(const QByteArray &datagram, GstElement *appsrc)
{
    GstBuffer *buffer = gst_buffer_new_and_alloc(datagram.size());
    GstMapInfo mapInfo;
    if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_WRITE)) {
        qFatal("Could not map buffer");
        return;
    }
    std::memcpy(mapInfo.data, datagram.data(), mapInfo.size);
    gst_buffer_unmap(buffer, &mapInfo);
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);
}

void QXmppCallStreamPrivate::addEncoder(QXmppCallPrivate::GstCodec &codec)
{
    // Remove old encoder and payloader if they exist
    if (encoderBin) {
        if (!gst_bin_remove(GST_BIN(pipeline), encoderBin)) {
            qFatal("Failed to remove existing encoder bin");
        }
    }
    encoderBin = gst_bin_new(QStringLiteral("encoder_%1").arg(id).toLatin1().data());
    if (!gst_bin_add(GST_BIN(pipeline), encoderBin)) {
        qFatal("Failed to add encoder bin to wrapper");
        return;
    }

    sendPad = gst_ghost_pad_new_no_target(nullptr, GST_PAD_SINK);
    gst_element_add_pad(encoderBin, sendPad);

    // Create new elements
    GstElement *queue = gst_element_factory_make("queue", nullptr);
    if (!queue) {
        qFatal("Failed to create queue");
        return;
    }

    GstElement *pay = gst_element_factory_make(codec.gstPay.toLatin1().data(), nullptr);
    if (!pay) {
        qFatal("Failed to create payloader");
        return;
    }
    g_object_set(pay, "pt", codec.pt, "ssrc", localSsrc, nullptr);

    GstElement *encoder = gst_element_factory_make(codec.gstEnc.toLatin1().data(), nullptr);
    if (!encoder) {
        qFatal("Failed to create encoder");
        return;
    }
    for (auto &encProp : codec.encProps) {
        g_object_set(encoder, encProp.name.toLatin1().data(), encProp.value, nullptr);
    }

    gst_bin_add_many(GST_BIN(encoderBin), queue, encoder, pay, nullptr);

    if (!gst_element_link_pads(pay, "src", rtpbin, QStringLiteral("send_rtp_sink_%1").arg(id).toLatin1().data()) ||
        !gst_element_link_many(queue, encoder, pay, nullptr)) {
        qFatal("Could not link all encoder pads");
        return;
    }

    GstPad *targetPad = gst_element_get_static_pad(queue, "sink");
    if (!gst_ghost_pad_set_target(GST_GHOST_PAD(sendPad), targetPad)) {
        qFatal("Failed to set send pad");
        return;
    }
    gst_object_unref(targetPad);

    if (sendPadCB && (dtlsHandshakeComplete || !useDtls)) {
        sendPadCB(sendPad);
    }

    gst_element_sync_state_with_parent(encoderBin);
}

void QXmppCallStreamPrivate::addDecoder(GstPad *pad, QXmppCallPrivate::GstCodec &codec)
{
    // Remove old decoder and depayloader if they exist
    if (decoderBin) {
        if (!gst_bin_remove(GST_BIN(pipeline), decoderBin)) {
            qFatal("Failed to remove existing decoder bin");
        }
    }
    decoderBin = gst_bin_new(QStringLiteral("decoder_%1").arg(id).toLatin1().data());
    if (!gst_bin_add(GST_BIN(pipeline), decoderBin)) {
        qFatal("Failed to add decoder bin to wrapper");
        return;
    }

    receivePad = gst_ghost_pad_new_no_target(nullptr, GST_PAD_SRC);
    internalReceivePad = gst_ghost_pad_new_no_target(nullptr, GST_PAD_SINK);
    gst_element_add_pad(decoderBin, receivePad);
    gst_element_add_pad(decoderBin, internalReceivePad);

    // Create new elements
    GstElement *depay = gst_element_factory_make(codec.gstDepay.toLatin1().data(), nullptr);
    if (!depay) {
        qFatal("Failed to create depayloader");
        return;
    }

    GstElement *decoder = gst_element_factory_make(codec.gstDec.toLatin1().data(), nullptr);
    if (!decoder) {
        qFatal("Failed to create decoder");
        return;
    }

    GstElement *queue = gst_element_factory_make("queue", nullptr);
    if (!queue) {
        qFatal("Failed to create queue");
        return;
    }

    gst_bin_add_many(GST_BIN(decoderBin), depay, decoder, queue, nullptr);

    GstPad *targetPad = gst_element_get_static_pad(depay, "sink");
    if (!gst_ghost_pad_set_target(GST_GHOST_PAD(internalReceivePad), targetPad)) {
        qFatal("Failed to set receive pad");
    }
    gst_object_unref(targetPad);

    targetPad = gst_element_get_static_pad(queue, "src");
    if (!gst_ghost_pad_set_target(GST_GHOST_PAD(receivePad), targetPad)) {
        qFatal("Failed to set receive pad");
    }
    gst_object_unref(targetPad);

    if (gst_pad_link(pad, internalReceivePad) != GST_PAD_LINK_OK ||
        !gst_element_link_many(depay, decoder, queue, nullptr)) {
        qFatal("Could not link all decoder pads");
        return;
    }

    gst_element_sync_state_with_parent(decoderBin);

    if (receivePadCB && (dtlsHandshakeComplete || !useDtls)) {
        receivePadCB(receivePad);
    }
}

QXmppCallStream::QXmppCallStream(GstElement *pipeline, GstElement *rtpbin,
                                 QString media, QString creator, QString name, int id, bool useDtls)
{
    d = new QXmppCallStreamPrivate(this, pipeline, rtpbin, media, creator, name, id, useDtls);
}

QString QXmppCallStream::creator() const
{
    return d->creator;
}

QString QXmppCallStream::media() const
{
    return d->media;
}

QString QXmppCallStream::name() const
{
    return d->name;
}

int QXmppCallStream::id() const
{
    return d->id;
}

void QXmppCallStream::setReceivePadCallback(std::function<void(GstPad *)> cb)
{
    d->receivePadCB = cb;
    if (d->receivePad) {
        d->receivePadCB(d->receivePad);
    }
}

void QXmppCallStream::setSendPadCallback(std::function<void(GstPad *)> cb)
{
    d->sendPadCB = cb;
    if (d->sendPad) {
        d->sendPadCB(d->sendPad);
    }
}

void QXmppCallStreamPrivate::toDtlsClientMode()
{
    gst_element_set_state(dtlsrtpencoder, GST_STATE_READY);
    gst_element_set_state(dtlsrtcpencoder, GST_STATE_READY);
    g_object_set(dtlsrtpencoder, "is-client", true, nullptr);
    g_object_set(dtlsrtcpencoder, "is-client", true, nullptr);
    gst_element_set_state(dtlsrtpencoder, GST_STATE_PLAYING);
    gst_element_set_state(dtlsrtcpencoder, GST_STATE_PLAYING);
}
