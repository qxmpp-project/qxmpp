// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPresence.h"

#include "QXmppConstants_p.h"
#include "QXmppJingleIq.h"
#include "QXmppUtils.h"

#include <QDateTime>
#include <QDomElement>

static const QStringList PRESENCE_TYPES = {
    QStringLiteral("error"),
    QString(),
    QStringLiteral("unavailable"),
    QStringLiteral("subscribe"),
    QStringLiteral("subscribed"),
    QStringLiteral("unsubscribe"),
    QStringLiteral("unsubscribed"),
    QStringLiteral("probe")
};

static const QStringList AVAILABLE_STATUS_TYPES = {
    QString(),
    QStringLiteral("away"),
    QStringLiteral("xa"),
    QStringLiteral("dnd"),
    QStringLiteral("chat"),
    QStringLiteral("invisible")
};

class QXmppPresencePrivate : public QSharedData
{
public:
    QXmppPresencePrivate();

    QXmppPresence::Type type;
    QXmppPresence::AvailableStatusType availableStatusType;
    QString statusText;
    int priority;

    // XEP-0045: Multi-User Chat
    QXmppMucItem mucItem;
    QString mucPassword;
    QList<int> mucStatusCodes;
    bool mucSupported;

    // XEP-0115: Entity Capabilities
    QString capabilityHash;
    QString capabilityNode;
    QByteArray capabilityVer;
    // Legacy XEP-0115: Entity Capabilities
    QStringList capabilityExt;

    // XEP-0153: vCard-Based Avatars
    // photoHash: the SHA1 hash of the avatar image data itself (not the base64-encoded version)
    // in accordance with RFC 3174
    QByteArray photoHash;
    QXmppPresence::VCardUpdateType vCardUpdateType;

    // XEP-0272: Multiparty Jingle (Muji)
    bool isPreparingMujiSession = false;
    QVector<QXmppJingleIq::Content> mujiContents;

    // XEP-0319: Last User Interaction in Presence
    QDateTime lastUserInteraction;

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    QString mixUserJid;
    QString mixUserNick;
};

QXmppPresencePrivate::QXmppPresencePrivate()
    : type(QXmppPresence::Available),
      availableStatusType(QXmppPresence::Online),
      priority(0),
      mucSupported(false),
      vCardUpdateType(QXmppPresence::VCardUpdateNone)
{
}

///
/// Constructs a QXmppPresence.
///
QXmppPresence::QXmppPresence(QXmppPresence::Type type)
    : d(new QXmppPresencePrivate)
{
    d->type = type;
}

/// Copy-constructor.
QXmppPresence::QXmppPresence(const QXmppPresence &other) = default;
/// Move-constructor.
QXmppPresence::QXmppPresence(QXmppPresence &&) = default;
/// Destroys a QXmppPresence.
QXmppPresence::~QXmppPresence() = default;
/// Assignemnt operator.
QXmppPresence &QXmppPresence::operator=(const QXmppPresence &other) = default;
/// Move-assignemnt operator.
QXmppPresence &QXmppPresence::operator=(QXmppPresence &&) = default;

///
/// Indicates if the QXmppStanza is a stanza in the XMPP sence (i. e. a message,
/// iq or presence)
///
/// \since QXmpp 1.0
///
bool QXmppPresence::isXmppStanza() const
{
    return true;
}

/// Returns the availability status type, for instance busy or away.
///
/// This will not tell you whether a contact is connected, check whether
/// type() is QXmppPresence::Available instead.

QXmppPresence::AvailableStatusType QXmppPresence::availableStatusType() const
{
    return d->availableStatusType;
}

/// Sets the availability status type, for instance busy or away.

void QXmppPresence::setAvailableStatusType(AvailableStatusType type)
{
    d->availableStatusType = type;
}

/// Returns the priority level of the resource.

int QXmppPresence::priority() const
{
    return d->priority;
}

/// Sets the \a priority level of the resource.

void QXmppPresence::setPriority(int priority)
{
    d->priority = priority;
}

/// Returns the status text, a textual description of the user's status.

QString QXmppPresence::statusText() const
{
    return d->statusText;
}

/// Sets the status text, a textual description of the user's status.
///
/// \param statusText The status text, for example "Gone fishing".

void QXmppPresence::setStatusText(const QString &statusText)
{
    d->statusText = statusText;
}

/// Returns the presence type.
///
/// You can use this method to determine the action which needs to be
/// taken in response to receiving the presence. For instance, if the type is
/// QXmppPresence::Available or QXmppPresence::Unavailable, you could update
/// the icon representing a contact's availability.

QXmppPresence::Type QXmppPresence::type() const
{
    return d->type;
}

/// Sets the presence type.
///
/// \param type

void QXmppPresence::setType(QXmppPresence::Type type)
{
    d->type = type;
}

/// Returns the photo-hash of the VCardUpdate.
///
/// \return QByteArray

QByteArray QXmppPresence::photoHash() const
{
    return d->photoHash;
}

/// Sets the photo-hash of the VCardUpdate.
///
/// \param photoHash as QByteArray

void QXmppPresence::setPhotoHash(const QByteArray &photoHash)
{
    d->photoHash = photoHash;
}

/// Returns the type of VCardUpdate
///
/// \return VCardUpdateType

QXmppPresence::VCardUpdateType QXmppPresence::vCardUpdateType() const
{
    return d->vCardUpdateType;
}

/// Sets the type of VCardUpdate
///
/// \param type VCardUpdateType

void QXmppPresence::setVCardUpdateType(VCardUpdateType type)
{
    d->vCardUpdateType = type;
}

/// \xep{0115}: Entity Capabilities
QString QXmppPresence::capabilityHash() const
{
    return d->capabilityHash;
}

/// \xep{0115}: Entity Capabilities
void QXmppPresence::setCapabilityHash(const QString &hash)
{
    d->capabilityHash = hash;
}

/// \xep{0115}: Entity Capabilities
QString QXmppPresence::capabilityNode() const
{
    return d->capabilityNode;
}

/// \xep{0115}: Entity Capabilities
void QXmppPresence::setCapabilityNode(const QString &node)
{
    d->capabilityNode = node;
}

/// \xep{0115}: Entity Capabilities
QByteArray QXmppPresence::capabilityVer() const
{
    return d->capabilityVer;
}

/// \xep{0115}: Entity Capabilities
void QXmppPresence::setCapabilityVer(const QByteArray &ver)
{
    d->capabilityVer = ver;
}

/// Legacy \xep{0115}: Entity Capabilities
QStringList QXmppPresence::capabilityExt() const
{
    return d->capabilityExt;
}

///
/// Returns whether a \xep{0272, Multiparty Jingle (Muji)} session is being prepared.
///
/// \return whether a Muji session is being prepared
///
/// \since QXmpp 1.5
///
bool QXmppPresence::isPreparingMujiSession() const
{
    return d->isPreparingMujiSession;
}

///
/// Sets whether a \xep{0272, Multiparty Jingle (Muji)} session is being prepared.
///
/// \param isPreparingMujiSession whether a Muji session is being prepared
///
/// \since QXmpp 1.5
///
void QXmppPresence::setIsPreparingMujiSession(bool isPreparingMujiSession)
{
    d->isPreparingMujiSession = isPreparingMujiSession;
}

///
/// Returns \xep{0272, Multiparty Jingle (Muji)} contents.
///
/// \return Muji contents
///
/// \since QXmpp 1.5
///
QVector<QXmppJingleIq::Content> QXmppPresence::mujiContents() const
{
    return d->mujiContents;
}

///
/// Sets \xep{0272, Multiparty Jingle (Muji)} contents.
///
/// \param mujiContents Muji contents
///
/// \since QXmpp 1.5
///
void QXmppPresence::setMujiContents(const QVector<QXmppJingleIq::Content> &mujiContents)
{
    d->mujiContents = mujiContents;
}

/// Returns the MUC item.

QXmppMucItem QXmppPresence::mucItem() const
{
    return d->mucItem;
}

/// Sets the MUC item.
///
/// \param item

void QXmppPresence::setMucItem(const QXmppMucItem &item)
{
    d->mucItem = item;
}

/// Returns the password used to join a MUC room.

QString QXmppPresence::mucPassword() const
{
    return d->mucPassword;
}

/// Sets the password used to join a MUC room.

void QXmppPresence::setMucPassword(const QString &password)
{
    d->mucPassword = password;
}

/// Returns the MUC status codes.

QList<int> QXmppPresence::mucStatusCodes() const
{
    return d->mucStatusCodes;
}

/// Sets the MUC status codes.
///
/// \param codes

void QXmppPresence::setMucStatusCodes(const QList<int> &codes)
{
    d->mucStatusCodes = codes;
}

/// Returns true if the sender has indicated MUC support.

bool QXmppPresence::isMucSupported() const
{
    return d->mucSupported;
}

/// Sets whether MUC is \a supported.

void QXmppPresence::setMucSupported(bool supported)
{
    d->mucSupported = supported;
}

///
/// Returns when the last user interaction with the client took place. See
/// \xep{0319}: Last User Interaction in Presence for details.
///
/// \since QXmpp 1.0
///
QDateTime QXmppPresence::lastUserInteraction() const
{
    return d->lastUserInteraction;
}

///
/// Sets the time of the last user interaction as defined in \xep{0319}: Last
/// User Interaction in Presence.
///
/// \since QXmpp 1.0
///
void QXmppPresence::setLastUserInteraction(const QDateTime &lastUserInteraction)
{
    d->lastUserInteraction = lastUserInteraction;
}

/// Returns the actual (full) JID of the MIX channel participant.
///
/// \since QXmpp 1.1

QString QXmppPresence::mixUserJid() const
{
    return d->mixUserJid;
}

/// Sets the actual (full) JID of the MIX channel participant.
///
/// \since QXmpp 1.1

void QXmppPresence::setMixUserJid(const QString &mixUserJid)
{
    d->mixUserJid = mixUserJid;
}

/// Returns the MIX participant's nickname.
///
/// \since QXmpp 1.1

QString QXmppPresence::mixUserNick() const
{
    return d->mixUserNick;
}

/// Sets the MIX participant's nickname.
///
/// \since QXmpp 1.1

void QXmppPresence::setMixUserNick(const QString &mixUserNick)
{
    d->mixUserNick = mixUserNick;
}

/// \cond
void QXmppPresence::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    // attributes
    int type = PRESENCE_TYPES.indexOf(element.attribute(QStringLiteral("type")));
    if (type > -1) {
        d->type = Type(type);
    }

    QXmppElementList unknownElements;
    QDomElement childElement = element.firstChildElement();

    while (!childElement.isNull()) {
        if (childElement.tagName() == QStringLiteral("show")) {
            int availableStatusType = AVAILABLE_STATUS_TYPES.indexOf(childElement.text());
            if (availableStatusType > -1) {
                d->availableStatusType = AvailableStatusType(availableStatusType);
            }
        } else if (childElement.tagName() == QStringLiteral("status")) {
            d->statusText = childElement.text();
        } else if (childElement.tagName() == QStringLiteral("priority")) {
            d->priority = childElement.text().toInt();
            // parse presence extensions
            // XEP-0033: Extended Stanza Addressing and errors are parsed by QXmppStanza
        } else if (!(childElement.tagName() == QStringLiteral("addresses") && childElement.namespaceURI() == ns_extended_addressing) &&
                   childElement.tagName() != "error") {
            parseExtension(childElement, unknownElements);
        }
        childElement = childElement.nextSiblingElement();
    }

    setExtensions(unknownElements);
}

void QXmppPresence::parseExtension(const QDomElement &element, QXmppElementList &unknownElements)
{
    // XEP-0045: Multi-User Chat
    if (element.tagName() == QStringLiteral("x") && element.namespaceURI() == ns_muc) {
        d->mucSupported = true;
        d->mucPassword = element.firstChildElement(QStringLiteral("password")).text();
    } else if (element.tagName() == QStringLiteral("x") && element.namespaceURI() == ns_muc_user) {
        QDomElement itemElement = element.firstChildElement(QStringLiteral("item"));
        d->mucItem.parse(itemElement);
        QDomElement statusElement = element.firstChildElement(QStringLiteral("status"));
        d->mucStatusCodes.clear();

        while (!statusElement.isNull()) {
            d->mucStatusCodes << statusElement.attribute(QStringLiteral("code")).toInt();
            statusElement = statusElement.nextSiblingElement(QStringLiteral("status"));
        }
        // XEP-0115: Entity Capabilities
    } else if (element.tagName() == QStringLiteral("c") && element.namespaceURI() == ns_capabilities) {
        d->capabilityNode = element.attribute(QStringLiteral("node"));
        d->capabilityVer = QByteArray::fromBase64(element.attribute(QStringLiteral("ver")).toLatin1());
        d->capabilityHash = element.attribute(QStringLiteral("hash"));
        d->capabilityExt = element.attribute(QStringLiteral("ext")).split(' ', Qt::SkipEmptyParts);
        // XEP-0153: vCard-Based Avatars
    } else if (element.namespaceURI() == ns_vcard_update) {
        QDomElement photoElement = element.firstChildElement(QStringLiteral("photo"));
        if (photoElement.isNull()) {
            d->photoHash = {};
            d->vCardUpdateType = VCardUpdateNotReady;
        } else {
            d->photoHash = QByteArray::fromHex(photoElement.text().toLatin1());
            if (d->photoHash.isEmpty()) {
                d->vCardUpdateType = VCardUpdateNoPhoto;
            } else {
                d->vCardUpdateType = VCardUpdateValidPhoto;
            }
        }
        // XEP-0272: Multiparty Jingle (Muji)
    } else if (element.tagName() == QStringLiteral("muji") && element.namespaceURI() == ns_muji) {
        if (!element.firstChildElement(QStringLiteral("preparing")).isNull()) {
            d->isPreparingMujiSession = true;
        }

        for (auto contentElement = element.firstChildElement(QStringLiteral("content"));
             !contentElement.isNull();
             contentElement = contentElement.nextSiblingElement(QStringLiteral("content"))) {
            QXmppJingleIq::Content content;
            content.parse(contentElement);
            d->mujiContents.append(content);
        }
        // XEP-0319: Last User Interaction in Presence
    } else if (element.tagName() == QStringLiteral("idle") && element.namespaceURI() == ns_idle) {
        if (element.hasAttribute(QStringLiteral("since"))) {
            const QString since = element.attribute(QStringLiteral("since"));
            d->lastUserInteraction = QXmppUtils::datetimeFromString(since);
        }
        // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    } else if (element.tagName() == QStringLiteral("mix") && element.namespaceURI() == ns_mix_presence) {
        d->mixUserJid = element.firstChildElement(QStringLiteral("jid")).text();
        d->mixUserNick = element.firstChildElement(QStringLiteral("nick")).text();
    } else {
        unknownElements << element;
    }
}

void QXmppPresence::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement(QStringLiteral("presence"));
    helperToXmlAddAttribute(xmlWriter, QStringLiteral("xml:lang"), lang());
    helperToXmlAddAttribute(xmlWriter, QStringLiteral("id"), id());
    helperToXmlAddAttribute(xmlWriter, QStringLiteral("to"), to());
    helperToXmlAddAttribute(xmlWriter, QStringLiteral("from"), from());
    helperToXmlAddAttribute(xmlWriter, QStringLiteral("type"), PRESENCE_TYPES.at(d->type));

    const QString show = AVAILABLE_STATUS_TYPES.at(d->availableStatusType);
    if (!show.isEmpty()) {
        helperToXmlAddTextElement(xmlWriter, QStringLiteral("show"), show);
    }
    if (!d->statusText.isEmpty()) {
        helperToXmlAddTextElement(xmlWriter, QStringLiteral("status"), d->statusText);
    }
    if (d->priority != 0) {
        helperToXmlAddTextElement(xmlWriter, QStringLiteral("priority"), QString::number(d->priority));
    }

    error().toXml(xmlWriter);

    // XEP-0045: Multi-User Chat
    if (d->mucSupported) {
        xmlWriter->writeStartElement(QStringLiteral("x"));
        xmlWriter->writeDefaultNamespace(ns_muc);
        if (!d->mucPassword.isEmpty()) {
            xmlWriter->writeTextElement(QStringLiteral("password"), d->mucPassword);
        }
        xmlWriter->writeEndElement();
    }

    if (!d->mucItem.isNull() || !d->mucStatusCodes.isEmpty()) {
        xmlWriter->writeStartElement(QStringLiteral("x"));
        xmlWriter->writeDefaultNamespace(ns_muc_user);
        if (!d->mucItem.isNull()) {
            d->mucItem.toXml(xmlWriter);
        }
        for (const auto code : d->mucStatusCodes) {
            xmlWriter->writeStartElement(QStringLiteral("status"));
            xmlWriter->writeAttribute(QStringLiteral("code"), QString::number(code));
            xmlWriter->writeEndElement();
        }
        xmlWriter->writeEndElement();
    }

    // XEP-0115: Entity Capabilities
    if (!d->capabilityNode.isEmpty() &&
        !d->capabilityVer.isEmpty() &&
        !d->capabilityHash.isEmpty()) {
        xmlWriter->writeStartElement(QStringLiteral("c"));
        xmlWriter->writeDefaultNamespace(ns_capabilities);
        helperToXmlAddAttribute(xmlWriter, QStringLiteral("hash"), d->capabilityHash);
        helperToXmlAddAttribute(xmlWriter, QStringLiteral("node"), d->capabilityNode);
        helperToXmlAddAttribute(xmlWriter, QStringLiteral("ver"), d->capabilityVer.toBase64());
        xmlWriter->writeEndElement();
    }

    // XEP-0153: vCard-Based Avatars
    if (d->vCardUpdateType != VCardUpdateNone) {
        xmlWriter->writeStartElement(QStringLiteral("x"));
        xmlWriter->writeDefaultNamespace(ns_vcard_update);
        switch (d->vCardUpdateType) {
        case VCardUpdateNoPhoto:
            xmlWriter->writeEmptyElement(QStringLiteral("photo"));
            break;
        case VCardUpdateValidPhoto:
            helperToXmlAddTextElement(xmlWriter, QStringLiteral("photo"), d->photoHash.toHex());
            break;
        default:
            break;
        }
        xmlWriter->writeEndElement();
    }

    // XEP-0272: Multiparty Jingle (Muji)
    if (d->isPreparingMujiSession || !d->mujiContents.isEmpty()) {
        xmlWriter->writeStartElement(QStringLiteral("muji"));
        xmlWriter->writeDefaultNamespace(ns_muji);

        if (d->isPreparingMujiSession) {
            xmlWriter->writeEmptyElement(QStringLiteral("preparing"));
        }

        for (const auto &mujiContent : d->mujiContents) {
            mujiContent.toXml(xmlWriter);
        }

        xmlWriter->writeEndElement();
    }

    // XEP-0319: Last User Interaction in Presence
    if (!d->lastUserInteraction.isNull() && d->lastUserInteraction.isValid()) {
        xmlWriter->writeStartElement(QStringLiteral("idle"));
        xmlWriter->writeDefaultNamespace(ns_idle);
        helperToXmlAddAttribute(xmlWriter, QStringLiteral("since"), QXmppUtils::datetimeToString(d->lastUserInteraction));
        xmlWriter->writeEndElement();
    }

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    if (!d->mixUserJid.isEmpty() || !d->mixUserNick.isEmpty()) {
        xmlWriter->writeStartElement(QStringLiteral("mix"));
        xmlWriter->writeDefaultNamespace(ns_mix_presence);
        if (!d->mixUserJid.isEmpty()) {
            helperToXmlAddTextElement(xmlWriter, QStringLiteral("jid"), d->mixUserJid);
        }
        if (!d->mixUserNick.isEmpty()) {
            helperToXmlAddTextElement(xmlWriter, QStringLiteral("nick"), d->mixUserNick);
        }
        xmlWriter->writeEndElement();
    }

    // unknown extensions
    QXmppStanza::extensionsToXml(xmlWriter);

    xmlWriter->writeEndElement();
}
/// \endcond
