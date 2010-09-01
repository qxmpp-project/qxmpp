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

class QXmppVCard : public QXmppIq
{
public:
    QXmppVCard(const QString& bareJid = "");

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

    static bool isVCard(const QDomElement &element);

    /// \cond
// deprecated in release 0.3.0, as it drags in a dependency
// on QtGui, whilst the rest of QXmpp does not require QtGui
#ifndef QXMPP_NO_GUI
    QImage Q_DECL_DEPRECATED photoAsImage() const;
    void Q_DECL_DEPRECATED setPhoto(const QImage&);
#endif

// deprecated accessors, use the form without "get" instead
// deprecated in release 0.2.0
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

#endif // QXMPPVCARD_H
