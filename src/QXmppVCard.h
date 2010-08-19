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


#ifndef QXMPPVCARD_H
#define QXMPPVCARD_H

#include "QXmppIq.h"
#include <QMap>
#include <QDomElement>

class QImage;

/// \brief Represents the XMPP vCard.
///
/// The functions names are self explanatory.
/// Look at QXmppVCardManager and <B>XEP-0054: vcard-temp</B> for more details.
///
/// There are many field of XMPP vCard which are not present in
/// this class. File a issue for the same. We will add the requested
/// field to this class.
///

class QXmppVCard : public QXmppIq
{
public:
    QXmppVCard(const QString& bareJid = "");

    void setFirstName(const QString&);
    void setFullName(const QString&);
    void setLastName(const QString&);
    void setMiddleName(const QString&);
    void setNickName(const QString&);
    void setUrl(const QString&);

    void setPhoto(const QByteArray&);
    void setPhoto(const QImage&);

    QString firstName() const;
    QString fullName() const;
    QString lastName() const;
    QString middleName() const;
    QString nickName() const;
    QString url() const;

    QImage photoAsImage() const;
    const QByteArray& photo() const;

    // deprecated accessors, use the form without "get" instead
    /// \cond
    QString Q_DECL_DEPRECATED getFullName() const;
    QString Q_DECL_DEPRECATED getNickName() const;
    QImage Q_DECL_DEPRECATED getPhotoAsImage() const;
    const QByteArray Q_DECL_DEPRECATED & getPhoto() const;
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement&);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_firstName;
    QString m_fullName;
    QString m_lastName;
    QString m_middleName;
    QString m_nickName;
    QString m_url;

    // not as 64 base
    QByteArray m_photo;
};

#endif // QXMPPVCARD_H
