/*
 * Copyright (C) 2008-2010 The QXmpp developers
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
    enum Type
    {
        Error = 0,
        Available,
        Unavailable,
        Subscribe,
        Subscribed,
        Unsubscribe,
        Unsubscribed,
        Probe
    };

    // XEP-0153: vCard-Based Avatars
    enum VCardUpdateType
    {
        // TODO fix the enum names PhotoNotAdvertized is confusing
        // All of these should start with VCardUpdate
        VCardUpdateNone = 0,    ///< Protocol not supported
        PhotoNotAdvertized,     ///< Not using any avatar
        PhotoAdvertised,        ///< Using an avatar
        PhotoNotReady           ///< No info available about the avatar.
                                ///< It might be there in the vCard but the contact's
                                ///< client doesn't know it temporarily.
    };

    class Status
    {
    public:
        enum Type
        {
            Offline = 0, 
            Online, 
            Away, 
            XA, 
            DND, 
            Chat,
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

    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    QByteArray photoHash() const;
    void setPhotoHash(const QByteArray&);
    VCardUpdateType vCardUpdateType();
    void setVCardUpdateType(VCardUpdateType type);

    QString capabilityHash();
    void setCapabilityHash(const QString&);

    QString capabilityNode();
    void setCapabilityNode(const QString&);

    QByteArray capabilityVer();
    void setCapabilityVer(const QByteArray&);

    QStringList capabilityExt();

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
