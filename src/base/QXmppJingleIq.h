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

#ifndef QXMPPJINGLEIQ_H
#define QXMPPJINGLEIQ_H

#include <QHostAddress>

#include "QXmppIq.h"

class QXmppJingleCandidatePrivate;
class QXmppJingleIqContentPrivate;
class QXmppJingleIqPrivate;
class QXmppJinglePayloadTypePrivate;

/// \brief The QXmppJinglePayloadType class represents a payload type
/// as specified by XEP-0167: Jingle RTP Sessions and RFC 5245.
///

class QXMPP_EXPORT QXmppJinglePayloadType
{
public:
    QXmppJinglePayloadType();
    QXmppJinglePayloadType(const QXmppJinglePayloadType &other);
    ~QXmppJinglePayloadType();

    unsigned char channels() const;
    void setChannels(unsigned char channels);

    unsigned int clockrate() const;
    void setClockrate(unsigned int clockrate);

    unsigned char id() const;
    void setId(unsigned char id);

    unsigned int maxptime() const;
    void setMaxptime(unsigned int maxptime);

    QString name() const;
    void setName(const QString &name);

    QMap<QString, QString> parameters() const;
    void setParameters(const QMap<QString, QString> &parameters);

    unsigned int ptime() const;
    void setPtime(unsigned int ptime);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    QXmppJinglePayloadType& operator=(const QXmppJinglePayloadType &other);
    bool operator==(const QXmppJinglePayloadType &other) const;

private:
    QSharedDataPointer<QXmppJinglePayloadTypePrivate> d;
};

/// \brief The QXmppJingleCandidate class represents a transport candidate
/// as specified by XEP-0176: Jingle ICE-UDP Transport Method.
///

class QXMPP_EXPORT QXmppJingleCandidate
{
public:
    /// This enum is used to describe a candidate's type.
    enum Type
    {
        HostType,               ///< Host candidate, a local address/port.
        PeerReflexiveType,      ///< Peer-reflexive candidate,
                                ///< the address/port as seen from the peer.
        ServerReflexiveType,    ///< Server-reflexive candidate,
                                ///< the address/port as seen by the STUN server
        RelayedType             ///< Relayed candidate, a candidate from
                                ///< a TURN relay.
    };

    QXmppJingleCandidate();
    QXmppJingleCandidate(const QXmppJingleCandidate &other);
    ~QXmppJingleCandidate();

    QXmppJingleCandidate& operator=(const QXmppJingleCandidate &other);

    int component() const;
    void setComponent(int component);

    QString foundation() const;
    void setFoundation(const QString &foundation);

    int generation() const;
    void setGeneration(int generation);

    QHostAddress host() const;
    void setHost(const QHostAddress &host);

    QString id() const;
    void setId(const QString &id);

    int network() const;
    void setNetwork(int network);

    quint16 port() const;
    void setPort(quint16 port);

    int priority() const;
    void setPriority(int priority);

    QString protocol() const;
    void setProtocol(const QString &protocol);

    QXmppJingleCandidate::Type type() const;
    void setType(QXmppJingleCandidate::Type);

    bool isNull() const;

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    static QXmppJingleCandidate::Type typeFromString(const QString &typeStr, bool *ok = 0);
    static QString typeToString(QXmppJingleCandidate::Type type);
    /// \endcond

private:
    QSharedDataPointer<QXmppJingleCandidatePrivate> d;
};

/// \brief The QXmppJingleIq class represents an IQ used for initiating media
/// sessions as specified by XEP-0166: Jingle.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppJingleIq : public QXmppIq
{
public:
    /// This enum is used to describe a Jingle action.
    enum Action {
        ContentAccept,
        ContentAdd,
        ContentModify,
        ContentReject,
        ContentRemove,
        DescriptionInfo,
        SecurityInfo,
        SessionAccept,
        SessionInfo,
        SessionInitiate,
        SessionTerminate,
        TransportAccept,
        TransportInfo,
        TransportReject,
        TransportReplace
    };

    /// \internal
    ///
    /// The QXmppJingleIq::Content class represents the "content" element of a
    /// QXmppJingleIq.

    class QXMPP_EXPORT Content
    {
    public:
        Content();
        Content(const QXmppJingleIq::Content &other);
        ~Content();

        Content& operator=(const Content &other);

        QString creator() const;
        void setCreator(const QString &creator);

        QString name() const;
        void setName(const QString &name);

        QString senders() const;
        void setSenders(const QString &senders);

        // XEP-0167: Jingle RTP Sessions
        QString descriptionMedia() const;
        void setDescriptionMedia(const QString &media);

        quint32 descriptionSsrc() const;
        void setDescriptionSsrc(quint32 ssrc);

        void addPayloadType(const QXmppJinglePayloadType &payload);
        QList<QXmppJinglePayloadType> payloadTypes() const;
        void setPayloadTypes(const QList<QXmppJinglePayloadType> &payloadTypes);

        void addTransportCandidate(const QXmppJingleCandidate &candidate);
        QList<QXmppJingleCandidate> transportCandidates() const;
        void setTransportCandidates(const QList<QXmppJingleCandidate> &candidates);

        QString transportUser() const;
        void setTransportUser(const QString &user);

        QString transportPassword() const;
        void setTransportPassword(const QString &password);

        // XEP-0320: Use of DTLS-SRTP in Jingle Sessions
        QByteArray transportFingerprint() const;
        void setTransportFingerprint(const QByteArray &fingerprint);

        QString transportFingerprintHash() const;
        void setTransportFingerprintHash(const QString &hash);

        QString transportFingerprintSetup() const;
        void setTransportFingerprintSetup(const QString &setup);

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;

        bool parseSdp(const QString &sdp);
        QString toSdp() const;
        /// \endcond

    private:
        QSharedDataPointer<QXmppJingleIqContentPrivate> d;
    };

    /// \internal
    ///
    /// The QXmppJingleIq::Reason class represents the "reason" element of a
    /// QXmppJingleIq.

    class QXMPP_EXPORT Reason
    {
    public:
        /// This enum is used to describe a reason's type.
        enum Type {
            None,
            AlternativeSession,
            Busy,
            Cancel,
            ConnectivityError,
            Decline,
            Expired,
            FailedApplication,
            FailedTransport,
            GeneralError,
            Gone,
            IncompatibleParameters,
            MediaError,
            SecurityError,
            Success,
            Timeout,
            UnsupportedApplications,
            UnsupportedTransports
        };

        Reason();

        QString text() const;
        void setText(const QString &text);

        Type type() const;
        void setType(Type type);

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;
        /// \endcond

    private:
        QString m_text;
        Type m_type;
    };

    QXmppJingleIq();
    QXmppJingleIq(const QXmppJingleIq &other);
    ~QXmppJingleIq();

    QXmppJingleIq& operator=(const QXmppJingleIq &other);

    Action action() const;
    void setAction(Action action);

    void addContent(const Content &content);
    QList<Content> contents() const;
    void setContents(const QList<Content> &contents);

    QString initiator() const;
    void setInitiator(const QString &initiator);

    Reason& reason();
    const Reason& reason() const;

    QString responder() const;
    void setResponder(const QString &responder);

    // XEP-0167: Jingle RTP Sessions
    bool ringing() const;
    void setRinging(bool ringing);

    QString sid() const;
    void setSid(const QString &sid);

    /// \cond
    static bool isJingleIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppJingleIqPrivate> d;
};

#endif
