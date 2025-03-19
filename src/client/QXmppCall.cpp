// SPDX-FileCopyrightText: 2019 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCall.h"

#include "QXmppCallManager.h"
#include "QXmppCallManager_p.h"
#include "QXmppCallStream.h"
#include "QXmppCallStream_p.h"
#include "QXmppCall_p.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppJingleIq.h"
#include "QXmppStun.h"
#include "QXmppTask.h"
#include "QXmppUtils.h"

#include "StringLiterals.h"

#include <chrono>
#include <ranges>

// gstreamer
#include <gst/gst.h>

#include <QDomElement>
#include <QTimer>

using namespace std::chrono_literals;

/// \cond
QXmppCallPrivate::QXmppCallPrivate(QXmppCall *qq)
    : direction(QXmppCall::IncomingDirection),
      manager(0),
      state(QXmppCall::ConnectingState),
      nextId(0),
      q(qq)
{
    qRegisterMetaType<QXmppCall::State>();

    filterGStreamerFormats(videoCodecs);
    filterGStreamerFormats(audioCodecs);

    pipeline = gst_pipeline_new(nullptr);
    if (!pipeline) {
        qFatal("Failed to create pipeline");
        return;
    }
    rtpbin = gst_element_factory_make("rtpbin", nullptr);
    if (!rtpbin) {
        qFatal("Failed to create rtpbin");
        return;
    }
    // We do not want to build up latency over time
    g_object_set(rtpbin, "drop-on-latency", true, "async-handling", true, "latency", 25, nullptr);
    if (!gst_bin_add(GST_BIN(pipeline), rtpbin)) {
        qFatal("Could not add rtpbin to the pipeline");
    }
    g_signal_connect_swapped(rtpbin, "pad-added",
                             G_CALLBACK(+[](QXmppCallPrivate *p, GstPad *pad) {
                                 p->padAdded(pad);
                             }),
                             this);
    g_signal_connect_swapped(rtpbin, "request-pt-map",
                             G_CALLBACK(+[](QXmppCallPrivate *p, uint sessionId, uint pt) {
                                 p->ptMap(sessionId, pt);
                             }),
                             this);
    g_signal_connect_swapped(rtpbin, "on-ssrc-active",
                             G_CALLBACK(+[](QXmppCallPrivate *p, uint sessionId, uint ssrc) {
                                 p->ssrcActive(sessionId, ssrc);
                             }),
                             this);

    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        qFatal("Unable to set the pipeline to the playing state");
        return;
    }
}

QXmppCallPrivate::~QXmppCallPrivate()
{
    if (gst_element_set_state(pipeline, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
        qFatal("Unable to set the pipeline to the null state");
    }
    qDeleteAll(streams);
    gst_object_unref(pipeline);
}

void QXmppCallPrivate::ssrcActive(uint sessionId, uint ssrc)
{
    Q_UNUSED(ssrc)
    GstElement *rtpSession;
    g_signal_emit_by_name(rtpbin, "get-session", static_cast<uint>(sessionId), &rtpSession);
    // TODO: implement bitrate controller
}

void QXmppCallPrivate::padAdded(GstPad *pad)
{
    auto nameParts = QString::fromUtf8(gst_pad_get_name(pad)).split(u'_');
    if (nameParts.size() < 4) {
        return;
    }
    if (nameParts[0] == u"send" &&
        nameParts[1] == u"rtp" &&
        nameParts[2] == u"src") {
        if (nameParts.size() != 4) {
            return;
        }
        int sessionId = nameParts[3].toInt();
        auto stream = findStreamById(sessionId);
        stream->d->addRtpSender(pad);
    } else if (nameParts[0] == u"recv" ||
               nameParts[1] == u"rtp" ||
               nameParts[2] == u"src") {
        if (nameParts.size() != 6) {
            return;
        }

        int sessionId = nameParts[3].toInt();
        int pt = nameParts[5].toInt();
        auto stream = findStreamById(sessionId);
        if (stream->media() == VIDEO_MEDIA) {
            for (auto &codec : videoCodecs) {
                if (codec.pt == pt) {
                    stream->d->addDecoder(pad, codec);
                    return;
                }
            }
        } else if (stream->media() == AUDIO_MEDIA) {
            for (auto &codec : audioCodecs) {
                if (codec.pt == pt) {
                    stream->d->addDecoder(pad, codec);
                    return;
                }
            }
        }
    }
}

GstCaps *QXmppCallPrivate::ptMap(uint sessionId, uint pt)
{
    auto stream = findStreamById(sessionId);
    for (auto &payloadType : stream->d->payloadTypes) {
        if (payloadType.id() == pt) {
            return gst_caps_new_simple("application/x-rtp",
                                       "media", G_TYPE_STRING, stream->media().toLatin1().data(),
                                       "clock-rate", G_TYPE_INT, payloadType.clockrate(),
                                       "encoding-name", G_TYPE_STRING, payloadType.name().toLatin1().data(),
                                       nullptr);
        }
    }
    q->warning(u"Remote party %1 transmits wrong %2 payload for call %3"_s.arg(jid, stream->media(), sid));
    return nullptr;
}

bool QXmppCallPrivate::isFormatSupported(const QString &codecName)
{
    GstElementFactory *factory;
    factory = gst_element_factory_find(codecName.toLatin1().data());
    if (!factory) {
        return false;
    }
    g_object_unref(factory);
    return true;
}

bool QXmppCallPrivate::isCodecSupported(const GstCodec &codec)
{
    return isFormatSupported(codec.gstPay) &&
        isFormatSupported(codec.gstDepay) &&
        isFormatSupported(codec.gstEnc) &&
        isFormatSupported(codec.gstDec);
}

void QXmppCallPrivate::filterGStreamerFormats(QList<GstCodec> &formats)
{
    auto removedRange = std::ranges::remove_if(formats, std::not_fn(isCodecSupported));
    formats.erase(removedRange.begin(), removedRange.end());
}

QXmppCallStream *QXmppCallPrivate::findStreamByMedia(QStringView media)
{
    if (auto stream = std::ranges::find(streams, media, &QXmppCallStream::media);
        stream != streams.end()) {
        return *stream;
    }
    return nullptr;
}

QXmppCallStream *QXmppCallPrivate::findStreamByName(QStringView name)
{
    if (auto stream = std::ranges::find(streams, name, &QXmppCallStream::name);
        stream != streams.end()) {
        return *stream;
    }
    return nullptr;
}

QXmppCallStream *QXmppCallPrivate::findStreamById(int id)
{
    if (auto stream = std::ranges::find(streams, id, &QXmppCallStream::id);
        stream != streams.end()) {
        return *stream;
    }
    return nullptr;
}

bool QXmppCallPrivate::handleDescription(QXmppCallStream *stream, const QXmppJingleIq::Content &content)
{
    stream->d->payloadTypes = content.payloadTypes();
    auto it = stream->d->payloadTypes.begin();
    bool foundCandidate = false;
    while (it != stream->d->payloadTypes.end()) {
        bool dynamic = it->id() >= 96;
        bool supported = false;
        auto codecs = stream->media() == AUDIO_MEDIA ? audioCodecs : videoCodecs;
        for (auto &codec : codecs) {
            if (dynamic) {
                if (codec.name == it->name() &&
                    codec.clockrate == it->clockrate() &&
                    codec.channels == it->channels()) {
                    if (!foundCandidate) {
                        stream->d->addEncoder(codec);
                        foundCandidate = true;
                    }
                    supported = true;
                    /* Adopt id from other side. */
                    codec.pt = it->id();
                }
            } else {
                if (codec.pt == it->id() &&
                    codec.clockrate == it->clockrate() &&
                    codec.channels == it->channels()) {
                    if (!foundCandidate) {
                        stream->d->addEncoder(codec);
                        foundCandidate = true;
                    }
                    supported = true;
                    /* Keep our name just to be sure */
                    codec.name = it->name();
                }
            }
        }

        if (!supported) {
            it = stream->d->payloadTypes.erase(it);
        } else {
            ++it;
        }
    }

    if (stream->d->payloadTypes.empty()) {
        q->warning(u"Remote party %1 did not provide any known %2 payloads for call %3"_s.arg(jid, stream->media(), sid));
        return false;
    }

    return true;
}

bool QXmppCallPrivate::handleTransport(QXmppCallStream *stream, const QXmppJingleIq::Content &content)
{
    stream->d->connection->setRemoteUser(content.transportUser());
    stream->d->connection->setRemotePassword(content.transportPassword());
    const auto candidates = content.transportCandidates();
    for (const auto &candidate : candidates) {
        stream->d->connection->addRemoteCandidate(candidate);
    }

    // perform ICE negotiation
    if (!content.transportCandidates().isEmpty()) {
        stream->d->connection->connectToHost();
    }
    return true;
}

void QXmppCallPrivate::handleRequest(const QXmppJingleIq &iq)
{
    const auto content = iq.contents().isEmpty() ? QXmppJingleIq::Content() : iq.contents().constFirst();

    if (iq.action() == QXmppJingleIq::SessionAccept) {

        if (direction == QXmppCall::IncomingDirection) {
            q->warning(u"Ignoring Session-Accept for an incoming call"_s);
            return;
        }

        // send ack
        sendAck(iq);

        // check content description and transport
        QXmppCallStream *stream = findStreamByName(content.name());
        if (!stream ||
            !handleDescription(stream, content) ||
            !handleTransport(stream, content)) {

            // terminate call
            terminate(QXmppJingleIq::Reason::FailedApplication);
            return;
        }

        // check for call establishment
        setState(QXmppCall::ActiveState);

    } else if (iq.action() == QXmppJingleIq::SessionInfo) {

        // notify user
        QTimer::singleShot(0, q, [this]() {
            Q_EMIT q->ringing();
        });

    } else if (iq.action() == QXmppJingleIq::SessionTerminate) {

        // send ack
        sendAck(iq);

        // terminate
        q->info(u"Remote party %1 terminated call %2"_s.arg(iq.from(), iq.sid()));
        q->terminated();

    } else if (iq.action() == QXmppJingleIq::ContentAccept) {

        // send ack
        sendAck(iq);

        // check content description and transport
        QXmppCallStream *stream = findStreamByName(content.name());
        if (!stream ||
            !handleDescription(stream, content) ||
            !handleTransport(stream, content)) {

            // FIXME: what action?
            return;
        }

    } else if (iq.action() == QXmppJingleIq::ContentAdd) {

        // send ack
        sendAck(iq);

        // check media stream does not exist yet
        QXmppCallStream *stream = findStreamByName(content.name());
        if (stream) {
            return;
        }

        // create media stream
        stream = createStream(content.descriptionMedia(), content.creator(), content.name());
        if (!stream) {
            return;
        }
        streams << stream;

        // check content description
        if (!handleDescription(stream, content) ||
            !handleTransport(stream, content)) {

            QXmppJingleIq iq;
            iq.setTo(q->jid());
            iq.setType(QXmppIq::Set);
            iq.setAction(QXmppJingleIq::ContentReject);
            iq.setSid(q->sid());
            iq.reason().setType(QXmppJingleIq::Reason::FailedApplication);
            manager->client()->sendIq(std::move(iq));
            streams.removeAll(stream);
            delete stream;
            return;
        }

        // accept content
        QXmppJingleIq iq;
        iq.setTo(q->jid());
        iq.setType(QXmppIq::Set);
        iq.setAction(QXmppJingleIq::ContentAccept);
        iq.setSid(q->sid());
        iq.addContent(localContent(stream));
        manager->client()->sendIq(std::move(iq));

    } else if (iq.action() == QXmppJingleIq::TransportInfo) {

        // send ack
        sendAck(iq);

        // check content transport
        QXmppCallStream *stream = findStreamByName(content.name());
        if (!stream ||
            !handleTransport(stream, content)) {
            // FIXME: what action?
            return;
        }
    }
}

QXmppCallStream *QXmppCallPrivate::createStream(const QString &media, const QString &creator, const QString &name)
{
    Q_ASSERT(manager);

    if (media != AUDIO_MEDIA && media != VIDEO_MEDIA) {
        q->warning(u"Unsupported media type %1"_s.arg(media));
        return nullptr;
    }

    if (!isFormatSupported(u"rtpbin"_s)) {
        q->warning(u"The rtpbin GStreamer plugin is missing. Calls are not possible."_s);
        return nullptr;
    }

    auto *stream = new QXmppCallStream(pipeline, rtpbin, media, creator, name, ++nextId);

    // Fill local payload payload types
    auto &codecs = media == AUDIO_MEDIA ? audioCodecs : videoCodecs;
    for (auto &codec : codecs) {
        QXmppJinglePayloadType payloadType;
        payloadType.setId(codec.pt);
        payloadType.setName(codec.name);
        payloadType.setChannels(codec.channels);
        payloadType.setClockrate(codec.clockrate);
        stream->d->payloadTypes.append(payloadType);
    }

    // ICE connection
    stream->d->connection->setIceControlling(direction == QXmppCall::OutgoingDirection);
    stream->d->connection->setStunServers(manager->d->stunServers);
    stream->d->connection->setTurnServer(manager->d->turnHost, manager->d->turnPort);
    stream->d->connection->setTurnUser(manager->d->turnUser);
    stream->d->connection->setTurnPassword(manager->d->turnPassword);
    stream->d->connection->bind(QXmppIceComponent::discoverAddresses());

    // connect signals
    QObject::connect(stream->d->connection, &QXmppIceConnection::localCandidatesChanged,
                     q, [this, stream]() { q->onLocalCandidatesChanged(stream); });

    QObject::connect(stream->d->connection, &QXmppIceConnection::disconnected,
                     q, &QXmppCall::hangup);

    Q_EMIT q->streamCreated(stream);

    return stream;
}

QXmppJingleIq::Content QXmppCallPrivate::localContent(QXmppCallStream *stream) const
{
    QXmppJingleIq::Content content;
    content.setCreator(stream->creator());
    content.setName(stream->name());
    content.setSenders(u"both"_s);

    // description
    content.setDescriptionMedia(stream->media());
    content.setDescriptionSsrc(stream->d->localSsrc);
    content.setPayloadTypes(stream->d->payloadTypes);

    // transport
    content.setTransportUser(stream->d->connection->localUser());
    content.setTransportPassword(stream->d->connection->localPassword());
    content.setTransportCandidates(stream->d->connection->localCandidates());

    return content;
}

///
/// Sends an acknowledgement for a Jingle IQ.
///
bool QXmppCallPrivate::sendAck(const QXmppJingleIq &iq)
{
    QXmppIq ack;
    ack.setId(iq.id());
    ack.setTo(iq.from());
    ack.setType(QXmppIq::Result);
    return manager->client()->sendPacket(ack);
}

void QXmppCallPrivate::sendInvite()
{
    // create audio stream
    QXmppCallStream *stream = findStreamByMedia(AUDIO_MEDIA);
    Q_ASSERT(stream);

    QXmppJingleIq iq;
    iq.setTo(jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionInitiate);
    iq.setInitiator(ownJid);
    iq.setSid(sid);
    iq.addContent(localContent(stream));
    manager->client()->send(std::move(iq));
}

void QXmppCallPrivate::setState(QXmppCall::State newState)
{
    if (state != newState) {
        state = newState;
        Q_EMIT q->stateChanged(state);

        if (state == QXmppCall::ActiveState) {
            Q_EMIT q->connected();
        } else if (state == QXmppCall::FinishedState) {
            Q_EMIT q->finished();
        }
    }
}

///
/// Request graceful call termination
///
void QXmppCallPrivate::terminate(QXmppJingleIq::Reason::Type reasonType)
{
    if (state == QXmppCall::DisconnectingState ||
        state == QXmppCall::FinishedState) {
        return;
    }

    // hangup call
    QXmppJingleIq iq;
    iq.setTo(jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionTerminate);
    iq.setSid(sid);
    iq.reason().setType(reasonType);

    setState(QXmppCall::DisconnectingState);

    manager->client()->sendIq(std::move(iq)).then(q, [this](auto result) {
        // terminate on both success or error
        q->terminated();
    });

    // schedule forceful termination in 5s
    QTimer::singleShot(5s, q, &QXmppCall::terminated);
}
/// \endcond

///
/// \class QXmppCall
///
/// The QXmppCall class represents a Voice-Over-IP call to a remote party.
///
/// \note THIS API IS NOT FINALIZED YET
///

QXmppCall::QXmppCall(const QString &jid, QXmppCall::Direction direction, QXmppCallManager *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppCallPrivate>(this))
{
    d->direction = direction;
    d->jid = jid;
    d->ownJid = parent->client()->configuration().jid();
    d->manager = parent;
}

QXmppCall::~QXmppCall() = default;

///
/// Call this method if you wish to accept an incoming call.
///
void QXmppCall::accept()
{
    if (d->direction == IncomingDirection && d->state == ConnectingState) {
        Q_ASSERT(d->streams.size() == 1);
        QXmppCallStream *stream = d->streams.first();

        // accept incoming call
        QXmppJingleIq iq;
        iq.setTo(d->jid);
        iq.setType(QXmppIq::Set);
        iq.setAction(QXmppJingleIq::SessionAccept);
        iq.setResponder(d->ownJid);
        iq.setSid(d->sid);
        iq.addContent(d->localContent(stream));
        d->manager->client()->sendIq(std::move(iq));

        // notify user
        Q_EMIT d->manager->callStarted(this);

        // check for call establishment
        d->setState(QXmppCall::ActiveState);
    }
}

///
/// Returns the GStreamer pipeline.
///
/// \since QXmpp 1.3
///
GstElement *QXmppCall::pipeline() const
{
    return d->pipeline;
}

///
/// Returns the RTP stream for the audio data.
///
/// \since QXmpp 1.3
///
QXmppCallStream *QXmppCall::audioStream() const
{
    return d->findStreamByMedia(AUDIO_MEDIA);
}

///
/// Returns the RTP stream for the video data.
///
/// \since QXmpp 1.3
///
QXmppCallStream *QXmppCall::videoStream() const
{
    return d->findStreamByMedia(VIDEO_MEDIA);
}

void QXmppCall::terminated()
{
    // close streams
    for (auto stream : std::as_const(d->streams)) {
        stream->d->connection->close();
    }

    // update state
    d->setState(QXmppCall::FinishedState);
}

///
/// Returns the call's direction.
///
QXmppCall::Direction QXmppCall::direction() const
{
    return d->direction;
}

///
/// Hangs up the call.
///
void QXmppCall::hangup()
{
    d->terminate(QXmppJingleIq::Reason::None);
}

///
/// Sends a transport-info to inform the remote party of new local candidates.
///
void QXmppCall::onLocalCandidatesChanged(QXmppCallStream *stream)
{
    QXmppJingleIq iq;
    iq.setTo(d->jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::TransportInfo);
    iq.setSid(d->sid);
    iq.addContent(d->localContent(stream));
    d->manager->client()->sendIq(std::move(iq));
}

///
/// Returns the remote party's JID.
///
QString QXmppCall::jid() const
{
    return d->jid;
}

///
/// Returns the call's session identifier.
///
QString QXmppCall::sid() const
{
    return d->sid;
}

///
/// Returns the call's state.
///
/// \sa stateChanged()
///
QXmppCall::State QXmppCall::state() const
{
    return d->state;
}

///
/// Starts sending video to the remote party.
///
void QXmppCall::addVideo()
{
    if (d->state != QXmppCall::ActiveState) {
        warning(u"Cannot add video, call is not active"_s);
        return;
    }

    QXmppCallStream *stream = d->findStreamByMedia(VIDEO_MEDIA);
    if (stream) {
        return;
    }

    // create video stream
    QString creator = (d->direction == QXmppCall::OutgoingDirection) ? u"initiator"_s : u"responder"_s;
    stream = d->createStream(VIDEO_MEDIA.toString(), creator, u"webcam"_s);
    d->streams << stream;

    // build request
    QXmppJingleIq iq;
    iq.setTo(d->jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::ContentAdd);
    iq.setSid(d->sid);
    iq.addContent(d->localContent(stream));
    d->manager->client()->sendIq(std::move(iq));
}
