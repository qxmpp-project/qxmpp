/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

/// \brief The QXmppPresence class represents an XMPP presence stanza.
///
/// \ingroup Stanzas
class QXmppPresence : public QXmppStanza
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

    // XEP-0153: vCard-Based Avatars
    enum VCardUpdateType
    {
        VCardUpdateNone = 0,    ///< Protocol is not supported
        VCardUpdateNoPhoto,     ///< User is not using any image
        VCardUpdateValidPhoto,  ///< User is advertising an image
        VCardUpdateNotReady     ///< User is not ready to advertise an image

/// \note This enables recipients to distinguish between the absence of an image
/// (empty photo element) and mere support for the protocol (empty update child).
    };

    /// \brief The QXmppPresence::Status class represents the status of an XMPP entity.
    ///
    /// It stores information such as the "away", "busy" status of a user, or
    /// a human-readable description.

    class Status
    {
    public:
        /// This enum is used to describe an availability status.
        enum Type
        {
            Offline = 0,
            Online,      ///< The entity or resource is online.
            Away,        ///< The entity or resource is temporarily away.
            XA,          ///< The entity or resource is away for an extended period. 
            DND,         ///< The entity or resource is busy ("Do Not Disturb").
            Chat,        ///< The entity or resource is actively interested in chatting.
            Invisible
        };

        Status(QXmppPresence::Status::Type type = QXmppPresence::Status::Online,
            const QString statusText = "", int priority = 0);

        QXmppPresence::Status::Type type() const;
        void setType(QXmppPresence::Status::Type);

        QString statusText() const;
        void setStatusText(const QString&);

        int priority() const;
        void setPriority(int);

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;

        // deprecated in release 0.2.0
        // deprecated accessors, use the form without "get" instead
        int Q_DECL_DEPRECATED getPriority() const;
        QString Q_DECL_DEPRECATED getStatusText() const;
        QXmppPresence::Status::Type Q_DECL_DEPRECATED getType() const;
        /// \endcond

    private:
        QString getTypeStr() const;
        void setTypeFromStr(const QString&);

        QXmppPresence::Status::Type m_type;
        QString m_statusText;
        int m_priority;
    };

    QXmppPresence(QXmppPresence::Type type = QXmppPresence::Available, 
        const QXmppPresence::Status& status = QXmppPresence::Status());
    ~QXmppPresence();

    QXmppPresence::Type type() const;
    void setType(QXmppPresence::Type);

    QXmppPresence::Status& status();
    const QXmppPresence::Status& status() const;
    void setStatus(const QXmppPresence::Status&);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    QByteArray photoHash() const;
    void setPhotoHash(const QByteArray&);

    VCardUpdateType vCardUpdateType() const;
    void setVCardUpdateType(VCardUpdateType type);

    QString capabilityHash() const;
    void setCapabilityHash(const QString&);

    QString capabilityNode() const;
    void setCapabilityNode(const QString&);

    QByteArray capabilityVer() const;
    void setCapabilityVer(const QByteArray&);

    QStringList capabilityExt() const;

    // deprecated in release 0.2.0
    // deprecated accessors, use the form without "get" instead
    /// \cond
    QXmppPresence::Type Q_DECL_DEPRECATED getType() const;
    QXmppPresence::Status Q_DECL_DEPRECATED & getStatus();
    const QXmppPresence::Status Q_DECL_DEPRECATED & getStatus() const;
    /// \endcond

private:
    QString getTypeStr() const;
    void setTypeFromStr(const QString&);

    Type m_type;
    QXmppPresence::Status m_status;


    /// XEP-0153: vCard-Based Avatars

    /// m_photoHash: the SHA1 hash of the avatar image data itself (not the base64-encoded version)
    /// in accordance with RFC 3174
    QByteArray m_photoHash;
    VCardUpdateType m_vCardUpdateType;

    // XEP-0115: Entity Capabilities
    QString m_capabilityHash;
    QString m_capabilityNode;
    QByteArray m_capabilityVer;
    // Legacy XEP-0115: Entity Capabilities
    QStringList m_capabilityExt;
};

#endif // QXMPPPRESENCE_H
