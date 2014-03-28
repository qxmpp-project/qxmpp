/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#ifndef QXMPPPRESENCE_H
#define QXMPPPRESENCE_H

#include "QXmppStanza.h"
#include "QXmppMucIq.h"

class QXmppPresencePrivate;

/// \brief The QXmppPresence class represents an XMPP presence stanza.
///
/// \ingroup Stanzas
class QXMPP_EXPORT QXmppPresence : public QXmppStanza
{
public:
    /// This enum is used to describe a presence type.
    enum Type
    {
        Error = 0,      ///< An error has occurred regarding processing or delivery of a previously-sent presence stanza.
        Available,      ///< Signals that the sender is online and available for communication.
        Unavailable,    ///< Signals that the sender is no longer available for communication.
        Subscribe,      ///< The sender wishes to subscribe to the recipient's  presence.
        Subscribed,     ///< The sender has allowed the recipient to receive their presence.
        Unsubscribe,    ///< The sender is unsubscribing from another entity's presence.
        Unsubscribed,   ///< The subscription request has been denied or a previously-granted subscription has been cancelled.
        Probe           ///< A request for an entity's current presence; SHOULD be generated only by a server on behalf of a user.
    };

    /// This enum is used to describe an availability status.
    enum AvailableStatusType
    {
        Online = 0,      ///< The entity or resource is online.
        Away,           ///< The entity or resource is temporarily away.
        XA,             ///< The entity or resource is away for an extended period.
        DND,            ///< The entity or resource is busy ("Do Not Disturb").
        Chat,           ///< The entity or resource is actively interested in chatting.
        Invisible       ///< obsolete XEP-0018: Invisible Presence
    };

    /// This enum is used to describe vCard updates as defined by
    /// XEP-0153: vCard-Based Avatars
    enum VCardUpdateType
    {
        VCardUpdateNone = 0,    ///< Protocol is not supported
        VCardUpdateNoPhoto,     ///< User is not using any image
        VCardUpdateValidPhoto,  ///< User is advertising an image
        VCardUpdateNotReady     ///< User is not ready to advertise an image

/// \note This enables recipients to distinguish between the absence of an image
/// (empty photo element) and mere support for the protocol (empty update child).
    };

    QXmppPresence(QXmppPresence::Type type = QXmppPresence::Available);
    QXmppPresence(const QXmppPresence &other);
    ~QXmppPresence();

    QXmppPresence& operator=(const QXmppPresence &other);

    AvailableStatusType availableStatusType() const;
    void setAvailableStatusType(AvailableStatusType type);

    int priority() const;
    void setPriority(int priority);

    QXmppPresence::Type type() const;
    void setType(QXmppPresence::Type);

    QString statusText() const;
    void setStatusText(const QString& statusText);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    // XEP-0045: Multi-User Chat
    QXmppMucItem mucItem() const;
    void setMucItem(const QXmppMucItem &item);

    QString mucPassword() const;
    void setMucPassword(const QString &password);

    QList<int> mucStatusCodes() const;
    void setMucStatusCodes(const QList<int> &codes);

    bool isMucSupported() const;
    void setMucSupported(bool supported);

    /// XEP-0153: vCard-Based Avatars
    QByteArray photoHash() const;
    void setPhotoHash(const QByteArray&);

    VCardUpdateType vCardUpdateType() const;
    void setVCardUpdateType(VCardUpdateType type);

    // XEP-0115: Entity Capabilities
    QString capabilityHash() const;
    void setCapabilityHash(const QString&);

    QString capabilityNode() const;
    void setCapabilityNode(const QString&);

    QByteArray capabilityVer() const;
    void setCapabilityVer(const QByteArray&);

    QStringList capabilityExt() const;

private:
    QSharedDataPointer<QXmppPresencePrivate> d;
};

#endif // QXMPPPRESENCE_H
