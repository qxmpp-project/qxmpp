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

#ifndef QXMPPJINGLEIQ_H
#define QXMPPJINGLEIQ_H

#include <QHostAddress>

#include "QXmppIq.h"

class QXmppJinglePayloadType
{
public:
    QXmppJinglePayloadType();

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

    unsigned int ptime() const;
    void setPtime(unsigned int ptime);

    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    bool operator==(const QXmppJinglePayloadType &other) const;

private:
    unsigned char m_channels;
    unsigned int m_clockrate;
    unsigned char m_id;
    unsigned int m_maxptime;
    QString m_name;
    unsigned int m_ptime;
};

class QXmppJingleCandidate
{
public:
    QXmppJingleCandidate();

    int component() const;
    void setComponent(int component);

    int foundation() const;
    void setFoundation(int foundation);

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

    QString type() const;
    void setType(const QString &type);

    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

private:
    int m_component;
    int m_foundation;
    int m_generation;
    QHostAddress m_host;
    QString m_id;
    int m_network;
    quint16 m_port;
    QString m_protocol;
    int m_priority;
    QString m_type;
};

class QXmppJingleIq : public QXmppIq
{
public:
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
        TransportReplace,
    };

    class Content
    {
    public:
        Content();

        QString creator() const;
        void setCreator(const QString &creator);

        QString name() const;
        void setName(const QString &name);

        QString senders() const;
        void setSenders(const QString &senders);

        // XEP-0167: Jingle RTP Sessions
        QString descriptionMedia() const;
        void setDescriptionMedia(const QString &media);

        void addPayloadType(const QXmppJinglePayloadType &payload);
        QList<QXmppJinglePayloadType> payloadTypes() const;
        void setPayloadTypes(const QList<QXmppJinglePayloadType> &payloadTypes);

        void addTransportCandidate(const QXmppJingleCandidate &candidate);
        QList<QXmppJingleCandidate> transportCandidates() const;

        QString transportUser() const;
        void setTransportUser(const QString &user);

        QString transportPassword() const;
        void setTransportPassword(const QString &password);

        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;

    private:
        QString m_creator;
        QString m_disposition;
        QString m_name;
        QString m_senders;

        QString m_descriptionMedia;
        QString m_descriptionType;
        QString m_transportType;
        QString m_transportUser;
        QString m_transportPassword;
        QList<QXmppJinglePayloadType> m_payloadTypes;
        QList<QXmppJingleCandidate> m_transportCandidates;
    };

    class Reason
    {
    public:
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
            UnsupportedTransports,
        };

        Reason();

        QString text() const;
        void setText(const QString &text);

        Type type() const;
        void setType(Type type);

        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;

    private:
        QString m_text;
        Type m_type;
    };

    QXmppJingleIq();

    Action action() const;
    void setAction(Action action);

    QString initiator() const;
    void setInitiator(const QString &initiator);

    QString responder() const;
    void setResponder(const QString &responder);

    QString sid() const;
    void setSid(const QString &sid);

    Content& content() { return m_content; };
    const Content& content() const { return m_content; };

    Reason& reason() { return m_reason; };
    const Reason& reason() const { return m_reason; };

    // XEP-0167: Jingle RTP Sessions
    bool ringing() const;
    void setRinging(bool ringing);

    static bool isJingleIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    Action m_action;
    QString m_initiator;
    QString m_responder;
    QString m_sid;

    Content m_content;
    Reason m_reason;
    bool m_ringing;
};

#endif
