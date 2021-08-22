// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "QXmppOutgoingComponent.h"

#include <QSslSocket>
#include <QDomElement>
#include <QCryptographicHash>

#include <QXmppMessage.h>
#include <QXmppPresence.h>
#include <QXmppIq.h>

#include "QXmppComponentConfig.h"

constexpr auto ns_component = u"jabber:component:accept";

class Handshake
{
public:
    Handshake(const QString &secret, const QString &streamId)
    {
        const auto data = streamId.toUtf8() + secret.toUtf8();
        m_handshake = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();
    }

    QByteArray serialize() const
    {
        return "<handshake>" + m_handshake + "</handshake>";
    }

private:
    QByteArray m_handshake;
};

class QXmppOutgoingComponentPrivate
{
public:
    QXmppComponentConfig config;
    bool authenticated = false;
};

QXmppOutgoingComponent::QXmppOutgoingComponent(QObject *parent)
    : QXmppStream(parent),
      d(new QXmppOutgoingComponentPrivate)
{
    auto *socket = new QSslSocket(this);
    setSocket(socket);
}

QXmppOutgoingComponent::~QXmppOutgoingComponent()
{
}

QXmppComponentConfig &QXmppOutgoingComponent::config()
{
    return d->config;
}

void QXmppOutgoingComponent::connectToHost()
{
    const auto host = d->config.host();
    const auto port = d->config.port();

    if (host.isEmpty() || !port) {
        warning(QStringLiteral("Cannot connect to server: Invalid host or port!"));
        return;
    }

    info(QStringLiteral("Connecting to %1:%2").arg(host, QString::number(port)));

    socket()->connectToHost(host, port);
}

bool QXmppOutgoingComponent::isAuthenticated() const
{
    return d->authenticated;
}

void QXmppOutgoingComponent::handleStart()
{
    QXmppStream::handleStart();

    // clean up
    d->authenticated = false;

    const auto component = d->config.componentName().toUtf8();
    const QByteArray data =
            "<stream:stream xmlns='jabber:component:accept' "
                           "xmlns:stream='http://etherx.jabber.org/streams' "
                           "to='" + component + "'>";
    sendData(data);
}

void QXmppOutgoingComponent::handleStanza(const QDomElement &element)
{
    if (!d->authenticated && element.tagName() == u"handshake") {
        d->authenticated = true;
        info(QStringLiteral("Successfully connected and authenticated!"));
        emit connected();
    }

    bool handled = false;
    emit elementReceived(element, handled);
    if (handled)
        return;

    const auto xmlns = element.namespaceURI();
    if (xmlns == ns_component) {
        if (element.tagName() == u"iq") {
            QXmppIq iqPacket;
            iqPacket.parse(element);

            // if we didn't understant the iq, reply with error
            // except for "result" and "error" iqs
            if (iqPacket.type() != QXmppIq::Result &&
                    iqPacket.type() != QXmppIq::Error) {
                QXmppIq iq(QXmppIq::Error);
                iq.setId(iqPacket.id());
                iq.setFrom(iqPacket.to());
                iq.setTo(iqPacket.from());
                iq.setError({ QXmppStanza::Error::Cancel,
                              QXmppStanza::Error::FeatureNotImplemented });
                sendPacket(iq);
            } else {
                emit iqReceived(iqPacket);
            }
        } else if (d->config.parseAllPresences() && element.tagName() == u"presence") {
            QXmppPresence presence;
            presence.parse(element);

            // emit presence
            emit presenceReceived(presence);
        } else if (d->config.parseAllMessages() && element.tagName() == u"message") {
            QXmppMessage message;
            message.parse(element);

            // emit message
            emit messageReceived(message);
        }
    }
}

void QXmppOutgoingComponent::handleStream(const QDomElement &element)
{
    const auto streamId = element.attribute("id");

    sendData(Handshake(d->config.secret(), streamId).serialize());
}
