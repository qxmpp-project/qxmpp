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


#include <QBuffer>
#include <QXmlStreamWriter>

#include "QXmppVCardIq.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

static QString getImageType(const QByteArray &contents)
{
    if (contents.startsWith("\x89PNG\x0d\x0a\x1a\x0a"))
        return "image/png";
    else if (contents.startsWith("\x8aMNG"))
        return "video/x-mng";
    else if (contents.startsWith("GIF8"))
        return "image/gif";
    else if (contents.startsWith("BM"))
        return "image/bmp";
    else if (contents.contains("/* XPM */"))
        return "image/x-xpm";
    else if (contents.contains("<?xml") && contents.contains("<svg"))
        return "image/svg+xml";
    else if (contents.startsWith("\xFF\xD8\xFF\xE0"))
        return "image/jpeg";
    return "image/unknown";
}

class QXmppVCardEmailPrivate : public QSharedData
{
public:
    QXmppVCardEmailPrivate() : type(QXmppVCardEmail::None) {};
    QString address;
    QXmppVCardEmail::Type type;
};

/// Constructs an empty vCard e-mail address.

QXmppVCardEmail::QXmppVCardEmail()
    : d(new QXmppVCardEmailPrivate)
{
}

/// Constructs a copy of \a other.

QXmppVCardEmail::QXmppVCardEmail(const QXmppVCardEmail &other)
    : d(other.d)
{
}

QXmppVCardEmail::~QXmppVCardEmail()
{
}

/// Assigns \a other to this vCard e-mail address.

QXmppVCardEmail& QXmppVCardEmail::operator=(const QXmppVCardEmail &other)
{
    d = other.d;
    return *this;
}

/// Returns the e-mail address.

QString QXmppVCardEmail::address() const
{
    return d->address;
}

/// Sets the e-mail \a address.

void QXmppVCardEmail::setAddress(const QString &address)
{
    d->address = address;
}

/// Returns the e-mail type, which is a combination of TypeFlag.

QXmppVCardEmail::Type QXmppVCardEmail::type() const
{
    return d->type;
}

/// Sets the e-mail \a type, which is a combination of TypeFlag.

void QXmppVCardEmail::setType(QXmppVCardEmail::Type type)
{
    d->type = type;
}

void QXmppVCardEmail::parse(const QDomElement &element)
{
    if (!element.firstChildElement("HOME").isNull())
        d->type |= Home;
    if (!element.firstChildElement("WORK").isNull())
        d->type |= Work;
    if (!element.firstChildElement("INTERNET").isNull())
        d->type |= Internet;
    if (!element.firstChildElement("PREF").isNull())
        d->type |= Preferred;
    if (!element.firstChildElement("X400").isNull())
        d->type |= X400;
    d->address = element.firstChildElement("USERID").text();
}

void QXmppVCardEmail::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("EMAIL");
    if (d->type & Home)
        writer->writeEmptyElement("HOME");
    if (d->type & Work)
        writer->writeEmptyElement("WORK");
    if (d->type & Internet)
        writer->writeEmptyElement("INTERNET");
    if (d->type & Preferred)
        writer->writeEmptyElement("PREF");
    if (d->type & X400)
        writer->writeEmptyElement("X400");
    writer->writeTextElement("USERID", d->address);
    writer->writeEndElement();
}

class QXmppVCardIqPrivate : public QSharedData
{
public:
    QDate birthday;
    QString firstName;
    QString fullName;
    QString lastName;
    QString middleName;
    QString nickName;
    QString url;

    // not as 64 base
    QByteArray photo;
    QString photoType;

    QList<QXmppVCardEmail> emails;
};

/// Constructs a QXmppVCardIq for the specified recipient.
///
/// \param jid

QXmppVCardIq::QXmppVCardIq(const QString& jid)
    : QXmppIq()
    , d(new QXmppVCardIqPrivate)
{
    // for self jid should be empty
    setTo(jid);
}

/// Constructs a copy of \a other.

QXmppVCardIq::QXmppVCardIq(const QXmppVCardIq &other)
    : QXmppIq(other)
    , d(other.d)
{
}

QXmppVCardIq::~QXmppVCardIq()
{
}

/// Assigns \a other to this vCard IQ.

QXmppVCardIq& QXmppVCardIq::operator=(const QXmppVCardIq &other)
{
    QXmppIq::operator=(other);
    d = other.d;
    return *this;
}

/// Returns the date of birth of the individual associated with the vCard.
///

QDate QXmppVCardIq::birthday() const
{
    return d->birthday;
}

/// Sets the date of birth of the individual associated with the vCard.
///
/// \param birthday

void QXmppVCardIq::setBirthday(const QDate &birthday)
{
    d->birthday = birthday;
}

/// Returns the email address.
///

QString QXmppVCardIq::email() const
{
    if (d->emails.isEmpty())
        return QString();
    else
        return d->emails.first().address();
}

/// Sets the email address.
///
/// \param email

void QXmppVCardIq::setEmail(const QString &email)
{
    QXmppVCardEmail first;
    first.setAddress(email);
    first.setType(QXmppVCardEmail::Internet);
    d->emails = QList<QXmppVCardEmail>() << first;
}

/// Returns the first name.
///

QString QXmppVCardIq::firstName() const
{
    return d->firstName;
}

/// Sets the first name.
///
/// \param firstName

void QXmppVCardIq::setFirstName(const QString &firstName)
{
    d->firstName = firstName;
}

/// Returns the full name.
///

QString QXmppVCardIq::fullName() const
{
    return d->fullName;
}

/// Sets the full name.
///
/// \param fullName

void QXmppVCardIq::setFullName(const QString &fullName)
{
    d->fullName = fullName;
}

/// Returns the last name.
///

QString QXmppVCardIq::lastName() const
{
    return d->lastName;
}

/// Sets the last name.
///
/// \param lastName

void QXmppVCardIq::setLastName(const QString &lastName)
{
    d->lastName = lastName;
}

/// Returns the middle name.
///

QString QXmppVCardIq::middleName() const
{
    return d->middleName;
}

/// Sets the middle name.
///
/// \param middleName

void QXmppVCardIq::setMiddleName(const QString &middleName)
{
    d->middleName = middleName;
}

/// Returns the nickname.
///

QString QXmppVCardIq::nickName() const
{
    return d->nickName;
}

/// Sets the nickname.
///
/// \param nickName

void QXmppVCardIq::setNickName(const QString &nickName)
{
    d->nickName = nickName;
}

/// Returns the URL associated with the vCard. It can represent the user's
/// homepage or a location at which you can find real-time information about
/// the vCard.

QString QXmppVCardIq::url() const
{
    return d->url;
}

/// Sets the URL associated with the vCard. It can represent the user's
/// homepage or a location at which you can find real-time information about
/// the vCard.
///
/// \param url

void QXmppVCardIq::setUrl(const QString& url)
{
    d->url = url;
}

/// Returns the photo's binary contents.
///
/// If you want to use the photo as a QImage you can use:
///
/// \code
/// QBuffer buffer;
/// buffer.setData(myCard.photo());
/// buffer.open(QIODevice::ReadOnly);
/// QImageReader imageReader(&buffer);
/// QImage myImage = imageReader.read();
/// \endcode

QByteArray QXmppVCardIq::photo() const
{
    return d->photo;
}

/// Sets the photo's binary contents.

void QXmppVCardIq::setPhoto(const QByteArray& photo)
{
    d->photo = photo;
}

/// Returns the photo's MIME type.

QString QXmppVCardIq::photoType() const
{
    return d->photoType;
}

/// Sets the photo's MIME type.

void QXmppVCardIq::setPhotoType(const QString& photoType)
{
    d->photoType = photoType;
}

/// Returns the e-mail addresses.

QList<QXmppVCardEmail> QXmppVCardIq::emails() const
{
    return d->emails;
}

/// Sets the e-mail addresses.

void QXmppVCardIq::setEmails(const QList<QXmppVCardEmail> &emails)
{
    d->emails = emails;
}

/// \cond
bool QXmppVCardIq::isVCard(const QDomElement &nodeRecv)
{
    return nodeRecv.firstChildElement("vCard").namespaceURI() == ns_vcard;
}

void QXmppVCardIq::parseElementFromChild(const QDomElement& nodeRecv)
{
    // vCard
    QDomElement cardElement = nodeRecv.firstChildElement("vCard");
    d->birthday = QDate::fromString(cardElement.firstChildElement("BDAY").text(), "yyyy-MM-dd");
    d->fullName = cardElement.firstChildElement("FN").text();
    d->nickName = cardElement.firstChildElement("NICKNAME").text();
    QDomElement nameElement = cardElement.firstChildElement("N");
    d->firstName = nameElement.firstChildElement("GIVEN").text();
    d->lastName = nameElement.firstChildElement("FAMILY").text();
    d->middleName = nameElement.firstChildElement("MIDDLE").text();
    d->url = cardElement.firstChildElement("URL").text();
    QDomElement photoElement = cardElement.firstChildElement("PHOTO");
    QByteArray base64data = photoElement.
                            firstChildElement("BINVAL").text().toAscii();
    d->photo = QByteArray::fromBase64(base64data);
    d->photoType = photoElement.firstChildElement("TYPE").text();

    QDomElement child = cardElement.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == "EMAIL") {
            QXmppVCardEmail email;
            email.parse(child);
            d->emails << email;
        }
        child = child.nextSiblingElement();
    }
}

void QXmppVCardIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("vCard");
    writer->writeAttribute("xmlns", ns_vcard);
    if (d->birthday.isValid())
        helperToXmlAddTextElement(writer, "BDAY", d->birthday.toString("yyyy-MM-dd"));
    foreach (const QXmppVCardEmail &email, d->emails)
        email.toXml(writer);
    if (!d->fullName.isEmpty())
        helperToXmlAddTextElement(writer, "FN", d->fullName);
    if(!d->nickName.isEmpty())
        helperToXmlAddTextElement(writer, "NICKNAME", d->nickName);
    if (!d->firstName.isEmpty() ||
        !d->lastName.isEmpty() ||
        !d->middleName.isEmpty())
    {
        writer->writeStartElement("N");
        if (!d->firstName.isEmpty())
            helperToXmlAddTextElement(writer, "GIVEN", d->firstName);
        if (!d->lastName.isEmpty())
            helperToXmlAddTextElement(writer, "FAMILY", d->lastName);
        if (!d->middleName.isEmpty())
            helperToXmlAddTextElement(writer, "MIDDLE", d->middleName);
        writer->writeEndElement();
    }
    if (!d->url.isEmpty())
        helperToXmlAddTextElement(writer, "URL", d->url);

    if(!photo().isEmpty())
    {
        writer->writeStartElement("PHOTO");
        QString photoType = d->photoType;
        if (photoType.isEmpty())
            photoType = getImageType(d->photo);
        helperToXmlAddTextElement(writer, "TYPE", photoType);
        helperToXmlAddTextElement(writer, "BINVAL", d->photo.toBase64());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
