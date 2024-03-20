// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixInvitation.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include <QDomElement>
#include <QSharedData>

using namespace QXmpp::Private;

class QXmppMixInvitationPrivate : public QSharedData
{
public:
    QString inviterJid;
    QString inviteeJid;
    QString channelJid;
    QString token;
};

///
/// \brief The QXmppMixInvitation class is used to invite a user to a
/// \xep{0369, Mediated Information eXchange (MIX)} channel as defined by
/// \xep{0407, Mediated Information eXchange (MIX): Miscellaneous Capabilities}.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.4
///

///
/// Default constructor
///
QXmppMixInvitation::QXmppMixInvitation()
    : d(new QXmppMixInvitationPrivate)
{
}

/// Copy constructor.
QXmppMixInvitation::QXmppMixInvitation(const QXmppMixInvitation &other) = default;
/// Copy constructor.
QXmppMixInvitation::QXmppMixInvitation(QXmppMixInvitation &&) = default;
/// Default assignment operator.
QXmppMixInvitation &QXmppMixInvitation::operator=(const QXmppMixInvitation &other) = default;
/// Default assignment operator.
QXmppMixInvitation &QXmppMixInvitation::operator=(QXmppMixInvitation &&) = default;
QXmppMixInvitation::~QXmppMixInvitation() = default;

///
/// Returns the JID of the inviter.
///
/// \return the inviter's JID
///
QString QXmppMixInvitation::inviterJid() const
{
    return d->inviterJid;
}

///
/// Sets the JID of the inviter.
///
/// \param inviterJid inviter's JID
///
void QXmppMixInvitation::setInviterJid(const QString &inviterJid)
{
    d->inviterJid = inviterJid;
}

///
/// Returns the JID of the invitee.
///
/// \return the invitee's JID
///
QString QXmppMixInvitation::inviteeJid() const
{
    return d->inviteeJid;
}

///
/// Sets the JID of the invitee.
///
/// \param inviteeJid invitee's JID
///
void QXmppMixInvitation::setInviteeJid(const QString &inviteeJid)
{
    d->inviteeJid = inviteeJid;
}

///
/// Returns the JID of the channel.
///
/// \return the channel's JID
///
QString QXmppMixInvitation::channelJid() const
{
    return d->channelJid;
}

///
/// Sets the JID of the channel.
///
/// \param channelJid channel JID
///
void QXmppMixInvitation::setChannelJid(const QString &channelJid)
{
    d->channelJid = channelJid;
}

///
/// Returns the token which is generated by the server and used by the invitee
/// for authentication.
///
/// \return the generated token used for authentication
///
QString QXmppMixInvitation::token() const
{
    return d->token;
}

///
/// Sets the token which is generated by the server and used by the invitee for
/// authentication.
///
/// \param token authentication token
///
void QXmppMixInvitation::setToken(const QString &token)
{
    d->token = token;
}

/// \cond
void QXmppMixInvitation::parse(const QDomElement &element)
{
    d->inviterJid = element.firstChildElement(QStringLiteral("inviter")).text();
    d->inviteeJid = element.firstChildElement(QStringLiteral("invitee")).text();
    d->channelJid = element.firstChildElement(QStringLiteral("channel")).text();
    d->token = element.firstChildElement(QStringLiteral("token")).text();
}

void QXmppMixInvitation::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("invitation"));
    writer->writeDefaultNamespace(toString65(ns_mix_misc));

    writeOptionalXmlTextElement(writer, u"inviter", d->inviterJid);
    writeOptionalXmlTextElement(writer, u"invitee", d->inviteeJid);
    writeOptionalXmlTextElement(writer, u"channel", d->channelJid);
    writeOptionalXmlTextElement(writer, u"token", d->token);

    writer->writeEndElement();
}
/// \endcond

///
/// Determines whether the given DOM element is a MIX invitation.
///
/// \param element DOM element being checked
///
/// \return true if element is a MIX invitation, otherwise false
///
bool QXmppMixInvitation::isMixInvitation(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("invitation") &&
        element.namespaceURI() == ns_mix_misc;
}
