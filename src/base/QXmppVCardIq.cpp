// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppVCardIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QBuffer>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

static QString getImageType(const QByteArray &contents)
{
    if (contents.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
        return u"image/png"_s;
    } else if (contents.startsWith("\x8aMNG")) {
        return u"video/x-mng"_s;
    } else if (contents.startsWith("GIF8")) {
        return u"image/gif"_s;
    } else if (contents.startsWith("BM")) {
        return u"image/bmp"_s;
    } else if (contents.contains("/* XPM */")) {
        return u"image/x-xpm"_s;
    } else if (contents.contains("<?xml") && contents.contains("<svg")) {
        return u"image/svg+xml"_s;
    } else if (contents.startsWith("\xFF\xD8\xFF\xE0")) {
        return u"image/jpeg"_s;
    }
    return u"image/unknown"_s;
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

/// Copy-constructor
QXmppVCardAddress::QXmppVCardAddress(const QXmppVCardAddress &other) = default;
/// Move-constructor
QXmppVCardAddress::QXmppVCardAddress(QXmppVCardAddress &&) = default;
QXmppVCardAddress::~QXmppVCardAddress() = default;
/// Assignment operator.
QXmppVCardAddress &QXmppVCardAddress::operator=(const QXmppVCardAddress &other) = default;
/// Move-assignment operator.
QXmppVCardAddress &QXmppVCardAddress::operator=(QXmppVCardAddress &&) = default;

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
    if (!element.firstChildElement(u"HOME"_s).isNull()) {
        d->type |= Home;
    }
    if (!element.firstChildElement(u"WORK"_s).isNull()) {
        d->type |= Work;
    }
    if (!element.firstChildElement(u"POSTAL"_s).isNull()) {
        d->type |= Postal;
    }
    if (!element.firstChildElement(u"PREF"_s).isNull()) {
        d->type |= Preferred;
    }

    d->country = element.firstChildElement(u"CTRY"_s).text();
    d->locality = element.firstChildElement(u"LOCALITY"_s).text();
    d->postcode = element.firstChildElement(u"PCODE"_s).text();
    d->region = element.firstChildElement(u"REGION"_s).text();
    d->street = element.firstChildElement(u"STREET"_s).text();
}

void QXmppVCardAddress::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("ADR"));
    if (d->type & Home) {
        writer->writeEmptyElement(u"HOME"_s);
    }
    if (d->type & Work) {
        writer->writeEmptyElement(u"WORK"_s);
    }
    if (d->type & Postal) {
        writer->writeEmptyElement(u"POSTAL"_s);
    }
    if (d->type & Preferred) {
        writer->writeEmptyElement(u"PREF"_s);
    }

    if (!d->country.isEmpty()) {
        writer->writeTextElement(QSL65("CTRY"), d->country);
    }
    if (!d->locality.isEmpty()) {
        writer->writeTextElement(QSL65("LOCALITY"), d->locality);
    }
    if (!d->postcode.isEmpty()) {
        writer->writeTextElement(QSL65("PCODE"), d->postcode);
    }
    if (!d->region.isEmpty()) {
        writer->writeTextElement(QSL65("REGION"), d->region);
    }
    if (!d->street.isEmpty()) {
        writer->writeTextElement(QSL65("STREET"), d->street);
    }

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

/// Copy-constructor
QXmppVCardEmail::QXmppVCardEmail(const QXmppVCardEmail &) = default;

QXmppVCardEmail::~QXmppVCardEmail() = default;

/// Copy-assignment operator.
QXmppVCardEmail &QXmppVCardEmail::operator=(const QXmppVCardEmail &other) = default;

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
    if (!element.firstChildElement(u"HOME"_s).isNull()) {
        d->type |= Home;
    }
    if (!element.firstChildElement(u"WORK"_s).isNull()) {
        d->type |= Work;
    }
    if (!element.firstChildElement(u"INTERNET"_s).isNull()) {
        d->type |= Internet;
    }
    if (!element.firstChildElement(u"PREF"_s).isNull()) {
        d->type |= Preferred;
    }
    if (!element.firstChildElement(u"X400"_s).isNull()) {
        d->type |= X400;
    }
    d->address = element.firstChildElement(u"USERID"_s).text();
}

void QXmppVCardEmail::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("EMAIL"));
    if (d->type & Home) {
        writer->writeEmptyElement(u"HOME"_s);
    }
    if (d->type & Work) {
        writer->writeEmptyElement(u"WORK"_s);
    }
    if (d->type & Internet) {
        writer->writeEmptyElement(u"INTERNET"_s);
    }
    if (d->type & Preferred) {
        writer->writeEmptyElement(u"PREF"_s);
    }
    if (d->type & X400) {
        writer->writeEmptyElement(u"X400"_s);
    }
    writer->writeTextElement(QSL65("USERID"), d->address);
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

/// Copy-constructor
QXmppVCardPhone::QXmppVCardPhone(const QXmppVCardPhone &other) = default;

QXmppVCardPhone::~QXmppVCardPhone() = default;

/// Copy-assignment operator
QXmppVCardPhone &QXmppVCardPhone::operator=(const QXmppVCardPhone &other) = default;

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
    if (!element.firstChildElement(u"HOME"_s).isNull()) {
        d->type |= Home;
    }
    if (!element.firstChildElement(u"WORK"_s).isNull()) {
        d->type |= Work;
    }
    if (!element.firstChildElement(u"VOICE"_s).isNull()) {
        d->type |= Voice;
    }
    if (!element.firstChildElement(u"FAX"_s).isNull()) {
        d->type |= Fax;
    }
    if (!element.firstChildElement(u"PAGER"_s).isNull()) {
        d->type |= Pager;
    }
    if (!element.firstChildElement(u"MSG"_s).isNull()) {
        d->type |= Messaging;
    }
    if (!element.firstChildElement(u"CELL"_s).isNull()) {
        d->type |= Cell;
    }
    if (!element.firstChildElement(u"VIDEO"_s).isNull()) {
        d->type |= Video;
    }
    if (!element.firstChildElement(u"BBS"_s).isNull()) {
        d->type |= BBS;
    }
    if (!element.firstChildElement(u"MODEM"_s).isNull()) {
        d->type |= Modem;
    }
    if (!element.firstChildElement(u"ISDN"_s).isNull()) {
        d->type |= ISDN;
    }
    if (!element.firstChildElement(u"PCS"_s).isNull()) {
        d->type |= PCS;
    }
    if (!element.firstChildElement(u"PREF"_s).isNull()) {
        d->type |= Preferred;
    }
    d->number = element.firstChildElement(u"NUMBER"_s).text();
}

void QXmppVCardPhone::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("TEL"));
    if (d->type & Home) {
        writer->writeEmptyElement(u"HOME"_s);
    }
    if (d->type & Work) {
        writer->writeEmptyElement(u"WORK"_s);
    }
    if (d->type & Voice) {
        writer->writeEmptyElement(u"VOICE"_s);
    }
    if (d->type & Fax) {
        writer->writeEmptyElement(u"FAX"_s);
    }
    if (d->type & Pager) {
        writer->writeEmptyElement(u"PAGER"_s);
    }
    if (d->type & Messaging) {
        writer->writeEmptyElement(u"MSG"_s);
    }
    if (d->type & Cell) {
        writer->writeEmptyElement(u"CELL"_s);
    }
    if (d->type & Video) {
        writer->writeEmptyElement(u"VIDEO"_s);
    }
    if (d->type & BBS) {
        writer->writeEmptyElement(u"BBS"_s);
    }
    if (d->type & Modem) {
        writer->writeEmptyElement(u"MODEM"_s);
    }
    if (d->type & ISDN) {
        writer->writeEmptyElement(u"ISDN"_s);
    }
    if (d->type & PCS) {
        writer->writeEmptyElement(u"PCS"_s);
    }
    if (d->type & Preferred) {
        writer->writeEmptyElement(u"PREF"_s);
    }
    writer->writeTextElement(QSL65("NUMBER"), d->number);
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

QXmppVCardOrganization::~QXmppVCardOrganization() = default;

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
    d->title = cardElem.firstChildElement(u"TITLE"_s).text();
    d->role = cardElem.firstChildElement(u"ROLE"_s).text();

    const QDomElement &orgElem = cardElem.firstChildElement(u"ORG"_s);
    d->organization = orgElem.firstChildElement(u"ORGNAME"_s).text();
    d->unit = orgElem.firstChildElement(u"ORGUNIT"_s).text();
}

void QXmppVCardOrganization::toXml(QXmlStreamWriter *stream) const
{
    if (!d->unit.isEmpty() || !d->organization.isEmpty()) {
        stream->writeStartElement(QSL65("ORG"));
        stream->writeTextElement(QSL65("ORGNAME"), d->organization);
        stream->writeTextElement(QSL65("ORGUNIT"), d->unit);
        stream->writeEndElement();
    }

    writeXmlTextElement(stream, u"TITLE"_s, d->title);
    writeXmlTextElement(stream, u"ROLE"_s, d->role);
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
QDate QXmppVCardIq::birthday() const
{
    return d->birthday;
}

/// Sets the date of birth of the individual associated with the vCard.
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
QString QXmppVCardIq::email() const
{
    if (d->emails.isEmpty()) {
        return QString();
    } else {
        return d->emails.first().address();
    }
}

/// Sets the email address.
void QXmppVCardIq::setEmail(const QString &email)
{
    QXmppVCardEmail first;
    first.setAddress(email);
    first.setType(QXmppVCardEmail::Internet);
    d->emails = QList<QXmppVCardEmail>() << first;
}

/// Returns the first name.
QString QXmppVCardIq::firstName() const
{
    return d->firstName;
}

/// Sets the first name.
void QXmppVCardIq::setFirstName(const QString &firstName)
{
    d->firstName = firstName;
}

/// Returns the full name.
QString QXmppVCardIq::fullName() const
{
    return d->fullName;
}

/// Sets the full name.
void QXmppVCardIq::setFullName(const QString &fullName)
{
    d->fullName = fullName;
}

/// Returns the last name.
QString QXmppVCardIq::lastName() const
{
    return d->lastName;
}

/// Sets the last name.
void QXmppVCardIq::setLastName(const QString &lastName)
{
    d->lastName = lastName;
}

/// Returns the middle name.
QString QXmppVCardIq::middleName() const
{
    return d->middleName;
}

/// Sets the middle name.
void QXmppVCardIq::setMiddleName(const QString &middleName)
{
    d->middleName = middleName;
}

/// Returns the nickname.
QString QXmppVCardIq::nickName() const
{
    return d->nickName;
}

/// Sets the nickname.
void QXmppVCardIq::setNickName(const QString &nickName)
{
    d->nickName = nickName;
}

///
/// Returns the URL associated with the vCard. It can represent the user's
/// homepage or a location at which you can find real-time information about
/// the vCard.
///
QString QXmppVCardIq::url() const
{
    return d->url;
}

///
/// Sets the URL associated with the vCard. It can represent the user's
/// homepage or a location at which you can find real-time information about
/// the vCard.
///
void QXmppVCardIq::setUrl(const QString &url)
{
    d->url = url;
}

///
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
///
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
bool QXmppVCardIq::isVCard(const QDomElement &el)
{
    return isIqType(el, u"vCard", ns_vcard);
}

bool QXmppVCardIq::checkIqType(const QString &tagName, const QString &xmlNamespace)
{
    return tagName == u"vCard" && xmlNamespace == ns_vcard;
}

void QXmppVCardIq::parseElementFromChild(const QDomElement &nodeRecv)
{
    // vCard
    QDomElement cardElement = nodeRecv.firstChildElement(u"vCard"_s);
    d->birthday = QDate::fromString(cardElement.firstChildElement(u"BDAY"_s).text(), u"yyyy-MM-dd"_s);
    d->description = cardElement.firstChildElement(u"DESC"_s).text();
    d->fullName = cardElement.firstChildElement(u"FN"_s).text();
    d->nickName = cardElement.firstChildElement(u"NICKNAME"_s).text();
    QDomElement nameElement = cardElement.firstChildElement(u"N"_s);
    d->firstName = nameElement.firstChildElement(u"GIVEN"_s).text();
    d->lastName = nameElement.firstChildElement(u"FAMILY"_s).text();
    d->middleName = nameElement.firstChildElement(u"MIDDLE"_s).text();
    d->url = cardElement.firstChildElement(u"URL"_s).text();
    QDomElement photoElement = cardElement.firstChildElement(u"PHOTO"_s);
    QByteArray base64data = photoElement.firstChildElement(u"BINVAL"_s).text().toLatin1();
    d->photo = QByteArray::fromBase64(base64data);
    d->photoType = photoElement.firstChildElement(u"TYPE"_s).text();

    for (const auto &child : iterChildElements(cardElement)) {
        if (child.tagName() == u"ADR") {
            QXmppVCardAddress address;
            address.parse(child);
            d->addresses << address;
        } else if (child.tagName() == u"EMAIL") {
            QXmppVCardEmail email;
            email.parse(child);
            d->emails << email;
        } else if (child.tagName() == u"TEL") {
            QXmppVCardPhone phone;
            phone.parse(child);
            d->phones << phone;
        }
    }

    d->organization.parse(cardElement);
}

void QXmppVCardIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("vCard"));
    writer->writeDefaultNamespace(toString65(ns_vcard));
    for (const QXmppVCardAddress &address : d->addresses) {
        address.toXml(writer);
    }
    if (d->birthday.isValid()) {
        writeXmlTextElement(writer, u"BDAY", d->birthday.toString(u"yyyy-MM-dd"_s));
    }
    if (!d->description.isEmpty()) {
        writeXmlTextElement(writer, u"DESC", d->description);
    }
    for (const QXmppVCardEmail &email : d->emails) {
        email.toXml(writer);
    }
    if (!d->fullName.isEmpty()) {
        writeXmlTextElement(writer, u"FN", d->fullName);
    }
    if (!d->nickName.isEmpty()) {
        writeXmlTextElement(writer, u"NICKNAME", d->nickName);
    }
    if (!d->firstName.isEmpty() ||
        !d->lastName.isEmpty() ||
        !d->middleName.isEmpty()) {
        writer->writeStartElement(QSL65("N"));
        if (!d->firstName.isEmpty()) {
            writeXmlTextElement(writer, u"GIVEN", d->firstName);
        }
        if (!d->lastName.isEmpty()) {
            writeXmlTextElement(writer, u"FAMILY", d->lastName);
        }
        if (!d->middleName.isEmpty()) {
            writeXmlTextElement(writer, u"MIDDLE", d->middleName);
        }
        writer->writeEndElement();
    }

    for (const QXmppVCardPhone &phone : d->phones) {
        phone.toXml(writer);
    }
    if (!photo().isEmpty()) {
        writer->writeStartElement(QSL65("PHOTO"));
        QString photoType = d->photoType;
        if (photoType.isEmpty()) {
            photoType = getImageType(d->photo);
        }
        writeXmlTextElement(writer, u"TYPE", photoType);
        writeXmlTextElement(writer, u"BINVAL", QString::fromUtf8(d->photo.toBase64()));
        writer->writeEndElement();
    }
    if (!d->url.isEmpty()) {
        writeXmlTextElement(writer, u"URL", d->url);
    }

    d->organization.toXml(writer);

    writer->writeEndElement();
}
/// \endcond
