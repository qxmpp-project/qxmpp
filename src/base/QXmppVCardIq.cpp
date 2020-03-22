/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppVCardIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QXmlStreamWriter>

static QString getImageType(const QByteArray &contents)
{
    if (contents.startsWith("\x89PNG\x0d\x0a\x1a\x0a"))
        return QSL("image/png");
    else if (contents.startsWith("\x8aMNG"))
        return QSL("video/x-mng");
    else if (contents.startsWith("GIF8"))
        return QSL("image/gif");
    else if (contents.startsWith("BM"))
        return QSL("image/bmp");
    else if (contents.contains("/* XPM */"))
        return QSL("image/x-xpm");
    else if (contents.contains("<?xml") && contents.contains("<svg"))
        return QSL("image/svg+xml");
    else if (contents.startsWith("\xFF\xD8\xFF\xE0"))
        return QSL("image/jpeg");
    return QSL("image/unknown");
}

class QXmppVCardAddressPrivate : public QSharedData
{
public:
    QXmppVCardAddressPrivate() : type(QXmppVCardAddress::None) {};
    QString country;
    QString locality;
    QString postcode;
    QString region;
    QString street;
    QXmppVCardAddress::Type type;
};

/// Constructs an empty address.

QXmppVCardAddress::QXmppVCardAddress()
    : d(new QXmppVCardAddressPrivate)
{
}

/// Constructs a copy of \a other.

QXmppVCardAddress::QXmppVCardAddress(const QXmppVCardAddress &other)
    : d(other.d)
{
}

QXmppVCardAddress::~QXmppVCardAddress()
{
}

/// Assigns \a other to this address.

QXmppVCardAddress &QXmppVCardAddress::operator=(const QXmppVCardAddress &other)
{
    d = other.d;
    return *this;
}

/// \brief Checks if two address objects represent the same address.

bool operator==(const QXmppVCardAddress &left, const QXmppVCardAddress &right)
{
    return left.type() == right.type() &&
        left.country() == right.country() &&
        left.locality() == right.locality() &&
        left.postcode() == right.postcode() &&
        left.region() == right.region() &&
        left.street() == right.street();
}

/// \brief Checks if two address objects represent different addresses.

bool operator!=(const QXmppVCardAddress &left, const QXmppVCardAddress &right)
{
    return !(left == right);
}

/// Returns the country.

QString QXmppVCardAddress::country() const
{
    return d->country;
}

/// Sets the country.

void QXmppVCardAddress::setCountry(const QString &country)
{
    d->country = country;
}

/// Returns the locality.

QString QXmppVCardAddress::locality() const
{
    return d->locality;
}

/// Sets the locality.

void QXmppVCardAddress::setLocality(const QString &locality)
{
    d->locality = locality;
}

/// Returns the postcode.

QString QXmppVCardAddress::postcode() const
{
    return d->postcode;
}

/// Sets the postcode.

void QXmppVCardAddress::setPostcode(const QString &postcode)
{
    d->postcode = postcode;
}

/// Returns the region.

QString QXmppVCardAddress::region() const
{
    return d->region;
}

/// Sets the region.

void QXmppVCardAddress::setRegion(const QString &region)
{
    d->region = region;
}

/// Returns the street address.

QString QXmppVCardAddress::street() const
{
    return d->street;
}

/// Sets the street address.

void QXmppVCardAddress::setStreet(const QString &street)
{
    d->street = street;
}

/// Returns the address type, which is a combination of TypeFlag.

QXmppVCardAddress::Type QXmppVCardAddress::type() const
{
    return d->type;
}

/// Sets the address \a type, which is a combination of TypeFlag.

void QXmppVCardAddress::setType(QXmppVCardAddress::Type type)
{
    d->type = type;
}

/// \cond
void QXmppVCardAddress::parse(const QDomElement &element)
{
    if (!element.firstChildElement(QSL("HOME")).isNull())
        d->type |= Home;
    if (!element.firstChildElement(QSL("WORK")).isNull())
        d->type |= Work;
    if (!element.firstChildElement(QSL("POSTAL")).isNull())
        d->type |= Postal;
    if (!element.firstChildElement(QSL("PREF")).isNull())
        d->type |= Preferred;

    d->country = element.firstChildElement(QSL("CTRY")).text();
    d->locality = element.firstChildElement(QSL("LOCALITY")).text();
    d->postcode = element.firstChildElement(QSL("PCODE")).text();
    d->region = element.firstChildElement(QSL("REGION")).text();
    d->street = element.firstChildElement(QSL("STREET")).text();
}

void QXmppVCardAddress::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL("ADR"));
    if (d->type & Home)
        writer->writeEmptyElement(QSL("HOME"));
    if (d->type & Work)
        writer->writeEmptyElement(QSL("WORK"));
    if (d->type & Postal)
        writer->writeEmptyElement(QSL("POSTAL"));
    if (d->type & Preferred)
        writer->writeEmptyElement(QSL("PREF"));

    if (!d->country.isEmpty())
        writer->writeTextElement(QSL("CTRY"), d->country);
    if (!d->locality.isEmpty())
        writer->writeTextElement(QSL("LOCALITY"), d->locality);
    if (!d->postcode.isEmpty())
        writer->writeTextElement(QSL("PCODE"), d->postcode);
    if (!d->region.isEmpty())
        writer->writeTextElement(QSL("REGION"), d->region);
    if (!d->street.isEmpty())
        writer->writeTextElement(QSL("STREET"), d->street);

    writer->writeEndElement();
}
/// \endcond

class QXmppVCardEmailPrivate : public QSharedData
{
public:
    QXmppVCardEmailPrivate() : type(QXmppVCardEmail::None) {};
    QString address;
    QXmppVCardEmail::Type type;
};

/// Constructs an empty e-mail address.

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

/// Assigns \a other to this e-mail address.

QXmppVCardEmail &QXmppVCardEmail::operator=(const QXmppVCardEmail &other)
{
    d = other.d;
    return *this;
}

/// \brief Checks if two email objects represent the same email address.

bool operator==(const QXmppVCardEmail &left, const QXmppVCardEmail &right)
{
    return left.type() == right.type() &&
        left.address() == right.address();
}

/// \brief Checks if two email objects represent different email addresses.

bool operator!=(const QXmppVCardEmail &left, const QXmppVCardEmail &right)
{
    return !(left == right);
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

/// \cond
void QXmppVCardEmail::parse(const QDomElement &element)
{
    if (!element.firstChildElement(QSL("HOME")).isNull())
        d->type |= Home;
    if (!element.firstChildElement(QSL("WORK")).isNull())
        d->type |= Work;
    if (!element.firstChildElement(QSL("INTERNET")).isNull())
        d->type |= Internet;
    if (!element.firstChildElement(QSL("PREF")).isNull())
        d->type |= Preferred;
    if (!element.firstChildElement(QSL("X400")).isNull())
        d->type |= X400;
    d->address = element.firstChildElement(QSL("USERID")).text();
}

void QXmppVCardEmail::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL("EMAIL"));
    if (d->type & Home)
        writer->writeEmptyElement(QSL("HOME"));
    if (d->type & Work)
        writer->writeEmptyElement(QSL("WORK"));
    if (d->type & Internet)
        writer->writeEmptyElement(QSL("INTERNET"));
    if (d->type & Preferred)
        writer->writeEmptyElement(QSL("PREF"));
    if (d->type & X400)
        writer->writeEmptyElement(QSL("X400"));
    writer->writeTextElement(QSL("USERID"), d->address);
    writer->writeEndElement();
}
/// \endcond

class QXmppVCardPhonePrivate : public QSharedData
{
public:
    QXmppVCardPhonePrivate() : type(QXmppVCardPhone::None) {};
    QString number;
    QXmppVCardPhone::Type type;
};

/// Constructs an empty phone number.

QXmppVCardPhone::QXmppVCardPhone()
    : d(new QXmppVCardPhonePrivate)
{
}

/// Constructs a copy of \a other.

QXmppVCardPhone::QXmppVCardPhone(const QXmppVCardPhone &other)
    : d(other.d)
{
}

QXmppVCardPhone::~QXmppVCardPhone()
{
}

/// Assigns \a other to this phone number.

QXmppVCardPhone &QXmppVCardPhone::operator=(const QXmppVCardPhone &other)
{
    d = other.d;
    return *this;
}

/// Returns the phone number.

QString QXmppVCardPhone::number() const
{
    return d->number;
}

/// \brief Checks if two phone objects represent the same phone number.

bool operator==(const QXmppVCardPhone &left, const QXmppVCardPhone &right)
{
    return left.type() == right.type() &&
        left.number() == right.number();
}

/// \brief Checks if two phone objects represent different phone numbers.

bool operator!=(const QXmppVCardPhone &left, const QXmppVCardPhone &right)
{
    return !(left == right);
}

/// Sets the phone \a number.

void QXmppVCardPhone::setNumber(const QString &number)
{
    d->number = number;
}

/// Returns the phone number type, which is a combination of TypeFlag.

QXmppVCardPhone::Type QXmppVCardPhone::type() const
{
    return d->type;
}

/// Sets the phone number \a type, which is a combination of TypeFlag.

void QXmppVCardPhone::setType(QXmppVCardPhone::Type type)
{
    d->type = type;
}

/// \cond
void QXmppVCardPhone::parse(const QDomElement &element)
{
    if (!element.firstChildElement(QSL("HOME")).isNull())
        d->type |= Home;
    if (!element.firstChildElement(QSL("WORK")).isNull())
        d->type |= Work;
    if (!element.firstChildElement(QSL("VOICE")).isNull())
        d->type |= Voice;
    if (!element.firstChildElement(QSL("FAX")).isNull())
        d->type |= Fax;
    if (!element.firstChildElement(QSL("PAGER")).isNull())
        d->type |= Pager;
    if (!element.firstChildElement(QSL("MSG")).isNull())
        d->type |= Messaging;
    if (!element.firstChildElement(QSL("CELL")).isNull())
        d->type |= Cell;
    if (!element.firstChildElement(QSL("VIDEO")).isNull())
        d->type |= Video;
    if (!element.firstChildElement(QSL("BBS")).isNull())
        d->type |= BBS;
    if (!element.firstChildElement(QSL("MODEM")).isNull())
        d->type |= Modem;
    if (!element.firstChildElement(QSL("ISDN")).isNull())
        d->type |= ISDN;
    if (!element.firstChildElement(QSL("PCS")).isNull())
        d->type |= PCS;
    if (!element.firstChildElement(QSL("PREF")).isNull())
        d->type |= Preferred;
    d->number = element.firstChildElement(QSL("NUMBER")).text();
}

void QXmppVCardPhone::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL("TEL"));
    if (d->type & Home)
        writer->writeEmptyElement(QSL("HOME"));
    if (d->type & Work)
        writer->writeEmptyElement(QSL("WORK"));
    if (d->type & Voice)
        writer->writeEmptyElement(QSL("VOICE"));
    if (d->type & Fax)
        writer->writeEmptyElement(QSL("FAX"));
    if (d->type & Pager)
        writer->writeEmptyElement(QSL("PAGER"));
    if (d->type & Messaging)
        writer->writeEmptyElement(QSL("MSG"));
    if (d->type & Cell)
        writer->writeEmptyElement(QSL("CELL"));
    if (d->type & Video)
        writer->writeEmptyElement(QSL("VIDEO"));
    if (d->type & BBS)
        writer->writeEmptyElement(QSL("BBS"));
    if (d->type & Modem)
        writer->writeEmptyElement(QSL("MODEM"));
    if (d->type & ISDN)
        writer->writeEmptyElement(QSL("ISDN"));
    if (d->type & PCS)
        writer->writeEmptyElement(QSL("PCS"));
    if (d->type & Preferred)
        writer->writeEmptyElement(QSL("PREF"));
    writer->writeTextElement(QSL("NUMBER"), d->number);
    writer->writeEndElement();
}
/// \endcond

class QXmppVCardOrganizationPrivate : public QSharedData
{
public:
    QString organization;
    QString unit;
    QString role;
    QString title;
};

/// Constructs an empty organization information.

QXmppVCardOrganization::QXmppVCardOrganization()
    : d(new QXmppVCardOrganizationPrivate)
{
}

/// Constructs a copy of \a other.

QXmppVCardOrganization::QXmppVCardOrganization(const QXmppVCardOrganization &other)
    : d(other.d)
{
}

QXmppVCardOrganization::~QXmppVCardOrganization()
{
}

/// Assigns \a other to this organization info.

QXmppVCardOrganization &QXmppVCardOrganization::operator=(const QXmppVCardOrganization &other)
{
    d = other.d;
    return *this;
}

/// \brief Checks if two organization objects represent the same organization.

bool operator==(const QXmppVCardOrganization &left, const QXmppVCardOrganization &right)
{
    return left.organization() == right.organization() &&
        left.unit() == right.unit() &&
        left.title() == right.title() &&
        left.role() == right.role();
}

/// \brief Checks if two organization objects represent different organizations.

bool operator!=(const QXmppVCardOrganization &left, const QXmppVCardOrganization &right)
{
    return !(left == right);
}

/// Returns the name of the organization.

QString QXmppVCardOrganization::organization() const
{
    return d->organization;
}

/// Sets the organization \a name.

void QXmppVCardOrganization::setOrganization(const QString &name)
{
    d->organization = name;
}

/// Returns the organization unit (also known as department).

QString QXmppVCardOrganization::unit() const
{
    return d->unit;
}

/// Sets the \a unit within the organization.

void QXmppVCardOrganization::setUnit(const QString &unit)
{
    d->unit = unit;
}

/// Returns the job role within the organization.

QString QXmppVCardOrganization::role() const
{
    return d->role;
}

/// Sets the job \a role within the organization.

void QXmppVCardOrganization::setRole(const QString &role)
{
    d->role = role;
}

/// Returns the job title within the organization.

QString QXmppVCardOrganization::title() const
{
    return d->title;
}

/// Sets the job \a title within the organization.

void QXmppVCardOrganization::setTitle(const QString &title)
{
    d->title = title;
}

/// \cond
void QXmppVCardOrganization::parse(const QDomElement &cardElem)
{
    d->title = cardElem.firstChildElement(QSL("TITLE")).text();
    d->role = cardElem.firstChildElement(QSL("ROLE")).text();

    const QDomElement &orgElem = cardElem.firstChildElement(QSL("ORG"));
    d->organization = orgElem.firstChildElement(QSL("ORGNAME")).text();
    d->unit = orgElem.firstChildElement(QSL("ORGUNIT")).text();
}

void QXmppVCardOrganization::toXml(QXmlStreamWriter *stream) const
{
    if (!d->unit.isEmpty() || !d->organization.isEmpty()) {
        stream->writeStartElement(QSL("ORG"));
        stream->writeTextElement(QSL("ORGNAME"), d->organization);
        stream->writeTextElement(QSL("ORGUNIT"), d->unit);
        stream->writeEndElement();
    }

    helperToXmlAddTextElement(stream, QSL("TITLE"), d->title);
    helperToXmlAddTextElement(stream, QSL("ROLE"), d->role);
}
/// \endcond

class QXmppVCardIqPrivate : public QSharedData
{
public:
    QDate birthday;
    QString description;
    QString firstName;
    QString fullName;
    QString lastName;
    QString middleName;
    QString nickName;
    QString url;

    // not as 64 base
    QByteArray photo;
    QString photoType;

    QList<QXmppVCardAddress> addresses;
    QList<QXmppVCardEmail> emails;
    QList<QXmppVCardPhone> phones;
    QXmppVCardOrganization organization;
};

/// Constructs a QXmppVCardIq for the specified recipient.
///
/// \param jid

QXmppVCardIq::QXmppVCardIq(const QString &jid)
    : QXmppIq(), d(new QXmppVCardIqPrivate)
{
    // for self jid should be empty
    setTo(jid);
}

/// Constructs a copy of \a other.

QXmppVCardIq::QXmppVCardIq(const QXmppVCardIq &other)
    : QXmppIq(other), d(other.d)
{
}

QXmppVCardIq::~QXmppVCardIq()
{
}

/// Assigns \a other to this vCard IQ.

QXmppVCardIq &QXmppVCardIq::operator=(const QXmppVCardIq &other)
{
    QXmppIq::operator=(other);
    d = other.d;
    return *this;
}

/// \brief Checks if two VCard objects represent the same VCard.

bool operator==(const QXmppVCardIq &left, const QXmppVCardIq &right)
{
    return left.birthday() == right.birthday() &&
        left.description() == right.description() &&
        left.email() == right.email() &&
        left.firstName() == right.firstName() &&
        left.fullName() == right.fullName() &&
        left.lastName() == right.lastName() &&
        left.middleName() == right.middleName() &&
        left.nickName() == right.nickName() &&
        left.photo() == right.photo() &&
        left.photoType() == right.photoType() &&
        left.url() == right.url() &&
        left.addresses() == right.addresses() &&
        left.emails() == right.emails() &&
        left.phones() == right.phones() &&
        left.organization() == right.organization();
}

/// \brief Checks if two VCard objects represent different VCards.

bool operator!=(const QXmppVCardIq &left, const QXmppVCardIq &right)
{
    return !(left == right);
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

/// Returns the free-form descriptive text.

QString QXmppVCardIq::description() const
{
    return d->description;
}

/// Sets the free-form descriptive text.

void QXmppVCardIq::setDescription(const QString &description)
{
    d->description = description;
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

void QXmppVCardIq::setUrl(const QString &url)
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

void QXmppVCardIq::setPhoto(const QByteArray &photo)
{
    d->photo = photo;
}

/// Returns the photo's MIME type.

QString QXmppVCardIq::photoType() const
{
    return d->photoType;
}

/// Sets the photo's MIME type.

void QXmppVCardIq::setPhotoType(const QString &photoType)
{
    d->photoType = photoType;
}

/// Returns the addresses.

QList<QXmppVCardAddress> QXmppVCardIq::addresses() const
{
    return d->addresses;
}

/// Sets the addresses.

void QXmppVCardIq::setAddresses(const QList<QXmppVCardAddress> &addresses)
{
    d->addresses = addresses;
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

/// Returns the phone numbers.

QList<QXmppVCardPhone> QXmppVCardIq::phones() const
{
    return d->phones;
}

/// Sets the phone numbers.

void QXmppVCardIq::setPhones(const QList<QXmppVCardPhone> &phones)
{
    d->phones = phones;
}

/// Returns the organization info.

QXmppVCardOrganization QXmppVCardIq::organization() const
{
    return d->organization;
}

/// Sets the organization info.

void QXmppVCardIq::setOrganization(const QXmppVCardOrganization &org)
{
    d->organization = org;
}

/// \cond
bool QXmppVCardIq::isVCard(const QDomElement &nodeRecv)
{
    return nodeRecv.firstChildElement(QSL("vCard")).namespaceURI() == ns_vcard;
}

void QXmppVCardIq::parseElementFromChild(const QDomElement &nodeRecv)
{
    // vCard
    QDomElement cardElement = nodeRecv.firstChildElement(QSL("vCard"));
    d->birthday = QDate::fromString(cardElement.firstChildElement(QSL("BDAY")).text(), QSL("yyyy-MM-dd"));
    d->description = cardElement.firstChildElement(QSL("DESC")).text();
    d->fullName = cardElement.firstChildElement(QSL("FN")).text();
    d->nickName = cardElement.firstChildElement(QSL("NICKNAME")).text();
    QDomElement nameElement = cardElement.firstChildElement(QSL("N"));
    d->firstName = nameElement.firstChildElement(QSL("GIVEN")).text();
    d->lastName = nameElement.firstChildElement(QSL("FAMILY")).text();
    d->middleName = nameElement.firstChildElement(QSL("MIDDLE")).text();
    d->url = cardElement.firstChildElement(QSL("URL")).text();
    QDomElement photoElement = cardElement.firstChildElement(QSL("PHOTO"));
    QByteArray base64data = photoElement.firstChildElement(QSL("BINVAL")).text().toLatin1();
    d->photo = QByteArray::fromBase64(base64data);
    d->photoType = photoElement.firstChildElement(QSL("TYPE")).text();

    QDomElement child = cardElement.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QSL("ADR")) {
            QXmppVCardAddress address;
            address.parse(child);
            d->addresses << address;
        } else if (child.tagName() == QSL("EMAIL")) {
            QXmppVCardEmail email;
            email.parse(child);
            d->emails << email;
        } else if (child.tagName() == QSL("TEL")) {
            QXmppVCardPhone phone;
            phone.parse(child);
            d->phones << phone;
        }
        child = child.nextSiblingElement();
    }

    d->organization.parse(cardElement);
}

void QXmppVCardIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL("vCard"));
    writer->writeDefaultNamespace(ns_vcard);
    for (const QXmppVCardAddress &address : d->addresses)
        address.toXml(writer);
    if (d->birthday.isValid())
        helperToXmlAddTextElement(writer, QSL("BDAY"), d->birthday.toString(QSL("yyyy-MM-dd")));
    if (!d->description.isEmpty())
        helperToXmlAddTextElement(writer, QSL("DESC"), d->description);
    for (const QXmppVCardEmail &email : d->emails)
        email.toXml(writer);
    if (!d->fullName.isEmpty())
        helperToXmlAddTextElement(writer, QSL("FN"), d->fullName);
    if (!d->nickName.isEmpty())
        helperToXmlAddTextElement(writer, QSL("NICKNAME"), d->nickName);
    if (!d->firstName.isEmpty() ||
        !d->lastName.isEmpty() ||
        !d->middleName.isEmpty()) {
        writer->writeStartElement("N");
        if (!d->firstName.isEmpty())
            helperToXmlAddTextElement(writer, QSL("GIVEN"), d->firstName);
        if (!d->lastName.isEmpty())
            helperToXmlAddTextElement(writer, QSL("FAMILY"), d->lastName);
        if (!d->middleName.isEmpty())
            helperToXmlAddTextElement(writer, QSL("MIDDLE"), d->middleName);
        writer->writeEndElement();
    }

    for (const QXmppVCardPhone &phone : d->phones)
        phone.toXml(writer);
    if (!photo().isEmpty()) {
        writer->writeStartElement(QSL("PHOTO"));
        QString photoType = d->photoType;
        if (photoType.isEmpty())
            photoType = getImageType(d->photo);
        helperToXmlAddTextElement(writer, QSL("TYPE"), photoType);
        helperToXmlAddTextElement(writer, QSL("BINVAL"), d->photo.toBase64());
        writer->writeEndElement();
    }
    if (!d->url.isEmpty())
        helperToXmlAddTextElement(writer, QSL("URL"), d->url);

    d->organization.toXml(writer);

    writer->writeEndElement();
}
/// \endcond
