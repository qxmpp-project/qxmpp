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


#ifndef QXMPPVCARDIQ_H
#define QXMPPVCARDIQ_H

#include "QXmppIq.h"
#include <QDate>
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

class QXmppVCardIq : public QXmppIq
{
public:
    QXmppVCardIq(const QString& bareJid = "");

    QDate birthday() const;
    void setBirthday(const QDate &birthday);

    QString email() const;
    void setEmail(const QString&);

    QString firstName() const;
    void setFirstName(const QString&);

    QString fullName() const;
    void setFullName(const QString&);

    QString lastName() const;
    void setLastName(const QString&);

    QString middleName() const;
    void setMiddleName(const QString&);

    QString nickName() const;
    void setNickName(const QString&);

    QByteArray photo() const;
    void setPhoto(const QByteArray&);

    QString photoType() const;
    void setPhotoType(const QString &type);

    QString url() const;
    void setUrl(const QString&);

    /// \cond
    static bool isVCard(const QDomElement &element);

    // deprecated accessors, use the form without "get" instead
    // deprecated in release 0.2.0
    QString Q_DECL_DEPRECATED getFullName() const;
    QString Q_DECL_DEPRECATED getNickName() const;
    const QByteArray Q_DECL_DEPRECATED & getPhoto() const;
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement&);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QDate m_birthday;
    QString m_email;
    QString m_firstName;
    QString m_fullName;
    QString m_lastName;
    QString m_middleName;
    QString m_nickName;
    QString m_url;

    // not as 64 base
    QByteArray m_photo;
    QString m_photoType;
};

#endif // QXMPPVCARDIQ_H
