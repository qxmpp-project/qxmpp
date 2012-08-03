/*
 * Copyright (C) 2008-2012 The QXmpp developers
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

class QXmppVCardEmailPrivate;

/// \brief Represents a vCard e-mail address.

class QXMPP_EXPORT QXmppVCardEmail
{
public:
    /// \brief Describes e-mail address types.
    enum TypeFlag {
        None        = 0x0,
        Home        = 0x1,
        Work        = 0x2,
        Internet    = 0x4,
        Preferred   = 0x8,
        X400        = 0x10
    };
    Q_DECLARE_FLAGS(Type, TypeFlag);

    QXmppVCardEmail();
    QXmppVCardEmail(const QXmppVCardEmail &other);
    ~QXmppVCardEmail();

    QXmppVCardEmail& operator=(const QXmppVCardEmail &other);

    QString address() const;
    void setAddress(const QString &address);

    Type type() const;
    void setType(Type type);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *stream) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppVCardEmailPrivate> d;
};

/// \brief Represents the XMPP vCard.
///
/// The functions names are self explanatory.
/// Look at QXmppVCardManager and XEP-0054: vcard-temp for more details.
///
/// There are many field of XMPP vCard which are not present in
/// this class. File a issue for the same. We will add the requested
/// field to this class.
///

class QXMPP_EXPORT QXmppVCardIq : public QXmppIq
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
