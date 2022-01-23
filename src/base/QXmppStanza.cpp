// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2015 Georg Rudoy <0xd34df00d@gmail.com>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStanza.h"

#include "QXmppConstants_p.h"
#include "QXmppStanza_p.h"
#include "QXmppUtils.h"

#include <QDateTime>
#include <QDomElement>
#include <QXmlStreamWriter>

uint QXmppStanza::s_uniqeIdNo = 0;

class QXmppExtendedAddressPrivate : public QSharedData
{
public:
    bool delivered;
    QString description;
    QString jid;
    QString type;
};

///
/// Constructs an empty extended address.
///
QXmppExtendedAddress::QXmppExtendedAddress()
    : d(new QXmppExtendedAddressPrivate())
{
    d->delivered = false;
}

/// Default copy-constructur
QXmppExtendedAddress::QXmppExtendedAddress(const QXmppExtendedAddress &other) = default;

QXmppExtendedAddress::~QXmppExtendedAddress() = default;

/// Default assignment operator
QXmppExtendedAddress &QXmppExtendedAddress::operator=(const QXmppExtendedAddress &other) = default;

///
/// Returns the human-readable description of the address.
///
QString QXmppExtendedAddress::description() const
{
    return d->description;
}

///
/// Sets the human-readable \a description of the address.
///
void QXmppExtendedAddress::setDescription(const QString &description)
{
    d->description = description;
}

///
/// Returns the JID of the address.
///
QString QXmppExtendedAddress::jid() const
{
    return d->jid;
}

///
/// Sets the JID of the address.
///
void QXmppExtendedAddress::setJid(const QString &jid)
{
    d->jid = jid;
}

///
/// Returns the type of the address.
///
QString QXmppExtendedAddress::type() const
{
    return d->type;
}

///
/// Sets the \a type of the address.
///
void QXmppExtendedAddress::setType(const QString &type)
{
    d->type = type;
}

///
/// Returns whether the stanza has been delivered to this address.
///
bool QXmppExtendedAddress::isDelivered() const
{
    return d->delivered;
}

///
/// Sets whether the stanza has been \a delivered to this address.
///
void QXmppExtendedAddress::setDelivered(bool delivered)
{
    d->delivered = delivered;
}

///
/// Checks whether this address is valid. The extended address is considered
/// to be valid if at least type and JID fields are non-empty.
///
bool QXmppExtendedAddress::isValid() const
{
    return !d->type.isEmpty() && !d->jid.isEmpty();
}

/// \cond
void QXmppExtendedAddress::parse(const QDomElement &element)
{
    d->delivered = element.attribute(QStringLiteral("delivered")) == QStringLiteral("true");
    d->description = element.attribute(QStringLiteral("desc"));
    d->jid = element.attribute(QStringLiteral("jid"));
    d->type = element.attribute(QStringLiteral("type"));
}

void QXmppExtendedAddress::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement(QStringLiteral("address"));
    if (d->delivered)
        xmlWriter->writeAttribute(QStringLiteral("delivered"), QStringLiteral("true"));
    if (!d->description.isEmpty())
        xmlWriter->writeAttribute(QStringLiteral("desc"), d->description);
    xmlWriter->writeAttribute(QStringLiteral("jid"), d->jid);
    xmlWriter->writeAttribute(QStringLiteral("type"), d->type);
    xmlWriter->writeEndElement();
}
/// \endcond

class QXmppStanzaErrorPrivate : public QSharedData
{
public:
    QXmppStanzaErrorPrivate();

    int code;
    std::optional<QXmppStanza::Error::Type> type;
    std::optional<QXmppStanza::Error::Condition> condition;
    QString text;
    QString by;
    QString redirectionUri;

    // XEP-0363: HTTP File Upload
    bool fileTooLarge;
    qint64 maxFileSize;
    QDateTime retryDate;
};

QXmppStanzaErrorPrivate::QXmppStanzaErrorPrivate()
    : code(0),
      fileTooLarge(false)
{
}

///
/// Default constructor
///
QXmppStanza::Error::Error()
    : d(new QXmppStanzaErrorPrivate)
{
}

/// Copy constructor
QXmppStanza::Error::Error(const QXmppStanza::Error &) = default;

///
/// Initializes an error with a type, condition and text.
///
QXmppStanza::Error::Error(Type type, Condition cond, const QString &text)
    : d(new QXmppStanzaErrorPrivate)
{
    d->type = type;
    d->condition = cond;
    d->text = text;
}

///
/// Initializes an error with a type, condition and text (all from strings).
///
QXmppStanza::Error::Error(const QString &type, const QString &cond,
                          const QString &text)
    : d(new QXmppStanzaErrorPrivate)
{
    d->text = text;
    d->type = typeFromString(type);
    d->condition = conditionFromString(cond);
}

/// Default destructor
QXmppStanza::Error::~Error() = default;

/// Copy operator
QXmppStanza::Error &QXmppStanza::Error::operator=(const QXmppStanza::Error &) = default;

///
/// Returns the human-readable description of the error.
///
QString QXmppStanza::Error::text() const
{
    return d->text;
}

///
/// Sets the description of the error.
///
void QXmppStanza::Error::setText(const QString &text)
{
    d->text = text;
}

///
/// Returns the error code.
///
int QXmppStanza::Error::code() const
{
    return d->code;
}

///
/// Sets the error code.
///
void QXmppStanza::Error::setCode(int code)
{
    d->code = code;
}

///
/// Returns the error condition.
///
/// The conditions QXmppStanza::Error::Gone and QXmppStanza::Error::Redirect
/// can be used in combination with redirectUri().
///
QXmppStanza::Error::Condition QXmppStanza::Error::condition() const
{
    return d->condition.value_or(QXmppStanza::Error::Condition(-1));
}

///
/// Sets the error condition.
///
/// The conditions QXmppStanza::Error::Gone and QXmppStanza::Error::Redirect
/// can be used in combination with setRedirectUri().
///
void QXmppStanza::Error::setCondition(QXmppStanza::Error::Condition cond)
{
    if (int(cond) < 0) {
        d->condition = std::nullopt;
        return;
    }
    d->condition = cond;
}

///
/// Returns the type of the error.
///
QXmppStanza::Error::Type QXmppStanza::Error::type() const
{
    return d->type.value_or(QXmppStanza::Error::Type(-1));
}

///
/// Returns the optional JID of the creator of the error.
///
/// This is useful to ditinguish between errors generated by the local server
/// and by the remote server for example. However, the value is optional.
///
/// \since QXmpp 1.3
///
QString QXmppStanza::Error::by() const
{
    return d->by;
}

///
/// Sets the optional JID of the creator of the error.
///
/// This is useful to ditinguish between errors generated by the local server
/// and by the remote server for example. However, the value is optional.
///
/// \since QXmpp 1.3
///
void QXmppStanza::Error::setBy(const QString &by)
{
    d->by = by;
}

///
/// Sets the type of the error.
///
void QXmppStanza::Error::setType(QXmppStanza::Error::Type type)
{
    if (int(type) < 0) {
        d->type = std::nullopt;
        return;
    }
    d->type = type;
}

///
/// Returns the optionally included redirection URI for the error conditions
/// QXmppStanza::Error::Gone and QXmppStanza::Error::Redirect.
///
/// \sa setRedirectionUri()
///
/// \since QXmpp 1.3
///
QString QXmppStanza::Error::redirectionUri() const
{
    return d->redirectionUri;
}

///
/// Sets the optional redirection URI for the error conditions
/// QXmppStanza::Error::Gone and QXmppStanza::Error::Redirect.
///
/// \sa redirectionUri()
///
/// \since QXmpp 1.3
///
void QXmppStanza::Error::setRedirectionUri(const QString &redirectionUri)
{
    d->redirectionUri = redirectionUri;
}

///
/// Returns true, if an HTTP File Upload failed, because the file was too
/// large.
///
/// \since QXmpp 1.1
///
bool QXmppStanza::Error::fileTooLarge() const
{
    return d->fileTooLarge;
}

///
/// Sets whether the requested file for HTTP File Upload was too large.
///
/// You should also set maxFileSize in this case.
///
/// \since QXmpp 1.1
///
void QXmppStanza::Error::setFileTooLarge(bool fileTooLarge)
{
    d->fileTooLarge = fileTooLarge;
}

///
/// Returns the maximum file size allowed for uploading via. HTTP File Upload.
///
/// \since QXmpp 1.1
///
qint64 QXmppStanza::Error::maxFileSize() const
{
    return d->maxFileSize;
}

///
/// Sets the maximum file size allowed for uploading via. HTTP File Upload.
///
/// This sets fileTooLarge to true.
///
/// \since QXmpp 1.1
///
void QXmppStanza::Error::setMaxFileSize(qint64 maxFileSize)
{
    setFileTooLarge(true);
    d->maxFileSize = maxFileSize;
}

///
/// Returns when to retry the upload request via. HTTP File Upload.
///
/// \since QXmpp 1.1
///
QDateTime QXmppStanza::Error::retryDate() const
{
    return d->retryDate;
}

///
/// Sets the datetime when the client can retry to request the upload slot.
///
void QXmppStanza::Error::setRetryDate(const QDateTime &retryDate)
{
    d->retryDate = retryDate;
}

/// \cond
void QXmppStanza::Error::parse(const QDomElement &errorElement)
{
    d->code = errorElement.attribute(QStringLiteral("code")).toInt();
    d->type = typeFromString(errorElement.attribute(QStringLiteral("type")));
    d->by = errorElement.attribute(QStringLiteral("by"));

    QDomElement element = errorElement.firstChildElement();
    while (!element.isNull()) {
        if (element.namespaceURI() == ns_stanza) {
            if (element.tagName() == QStringLiteral("text")) {
                d->text = element.text();
            } else {
                d->condition = conditionFromString(element.tagName());

                // redirection URI
                if (d->condition == Gone || d->condition == Redirect) {
                    d->redirectionUri = element.text();

                    // .text() returns empty string if nothing was set
                    if (d->redirectionUri.isEmpty())
                        d->redirectionUri.clear();
                }
            }
        } else if (element.namespaceURI() == ns_http_upload) {
            // XEP-0363: HTTP File Upload
            // file is too large
            if (element.tagName() == QStringLiteral("file-too-large")) {
                d->fileTooLarge = true;
                d->maxFileSize = element.firstChildElement(QStringLiteral("max-file-size"))
                                     .text()
                                     .toLongLong();
                // retry later
            } else if (element.tagName() == QStringLiteral("retry")) {
                d->retryDate = QXmppUtils::datetimeFromString(
                    element.attribute(QStringLiteral("stamp")));
            }
        }
        element = element.nextSiblingElement();
    }
}

void QXmppStanza::Error::toXml(QXmlStreamWriter *writer) const
{
    if (!d->condition && !d->type)
        return;

    writer->writeStartElement(QStringLiteral("error"));
    helperToXmlAddAttribute(writer, QStringLiteral("by"), d->by);
    if (d->type) {
        writer->writeAttribute(QStringLiteral("type"), typeToString(*d->type));
    }

    if (d->code > 0)
        helperToXmlAddAttribute(writer, QStringLiteral("code"), QString::number(d->code));

    if (d->condition) {
        writer->writeStartElement(conditionToString(*d->condition));
        writer->writeDefaultNamespace(ns_stanza);

        // redirection URI
        if (!d->redirectionUri.isEmpty() && (d->condition == Gone || d->condition == Redirect)) {
            writer->writeCharacters(d->redirectionUri);
        }

        writer->writeEndElement();
    }
    if (!d->text.isEmpty()) {
        writer->writeStartElement(QStringLiteral("text"));
        writer->writeAttribute(QStringLiteral("xml:lang"), QStringLiteral("en"));
        writer->writeDefaultNamespace(ns_stanza);
        writer->writeCharacters(d->text);
        writer->writeEndElement();
    }

    // XEP-0363: HTTP File Upload
    if (d->fileTooLarge) {
        writer->writeStartElement(QStringLiteral("file-too-large"));
        writer->writeDefaultNamespace(ns_http_upload);
        helperToXmlAddTextElement(writer, QStringLiteral("max-file-size"),
                                  QString::number(d->maxFileSize));
        writer->writeEndElement();
    } else if (!d->retryDate.isNull() && d->retryDate.isValid()) {
        writer->writeStartElement(QStringLiteral("retry"));
        writer->writeDefaultNamespace(ns_http_upload);
        writer->writeAttribute(QStringLiteral("stamp"),
                               QXmppUtils::datetimeToString(d->retryDate));
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond

///
/// \class QXmppE2eeMetadata
///
/// \brief The QXmppE2eeMetadata class contains data used for end-to-end
/// encryption purposes.
///
/// \since QXmpp 1.5
///

class QXmppE2eeMetadataPrivate : public QSharedData
{
public:
    QByteArray senderKey;

    // XEP-0420: Stanza Content Encryption
    QDateTime sceTimestamp;
};

///
/// Constructs a class for end-to-end encryption metadata.
///
QXmppE2eeMetadata::QXmppE2eeMetadata()
    : d(new QXmppE2eeMetadataPrivate)
{
}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppE2eeMetadata::QXmppE2eeMetadata(const QXmppE2eeMetadata &other) = default;

QXmppE2eeMetadata::~QXmppE2eeMetadata() = default;

///
/// Assigns \a other to this end-to-end encryption metadata class.
///
/// \param other
///
QXmppE2eeMetadata &QXmppE2eeMetadata::operator=(const QXmppE2eeMetadata &other) = default;

///
/// Returns the ID of this stanza's sender's public long-term key.
///
/// The sender key ID is not part of a transmitted stanza and thus not de- /
/// serialized.
/// Instead, the key ID is set by an encryption protocol such as
/// \xep{0384, OMEMO Encryption} during decryption.
/// It can be used by trust management protocols such as
/// \xep{0450, Automatic Trust Management (ATM)}.
///
/// \return the ID of the sender's key
///
/// \since QXmpp 1.5
///
QByteArray QXmppE2eeMetadata::senderKey() const
{
    return d->senderKey;
}

///
/// Sets the ID of this stanza's sender's public long-term key.
///
/// The sender key ID is not part of a transmitted stanza and thus not de- /
/// serialized.
/// Instead, it is set by an encryption protocol such as
/// \xep{0384, OMEMO Encryption} during decryption.
/// It can be used by trust management protocols such as
/// \xep{0450, Automatic Trust Management (ATM)}.
///
/// \param keyId ID of the sender's key
///
/// \since QXmpp 1.5
///
void QXmppE2eeMetadata::setSenderKey(const QByteArray &keyId)
{
    d->senderKey = keyId;
}

///
/// Returns the timestamp affix element's content as defined by
/// \xep{0420, Stanza Content Encryption} (SCE).
///
/// The SCE timestamp is part of an encrypted stanza's SCE envelope,
/// not an unencrypted direct child of a transmitted stanza and thus not de- /
/// serialized by it.
/// Instead, it is set by an encryption protocol such as
/// \xep{0384, OMEMO Encryption} after decryption.
/// It can be used by trust management protocols such as
/// \xep{0450, Automatic Trust Management (ATM)}.
///
/// \since QXmpp 1.5
///
QDateTime QXmppE2eeMetadata::sceTimestamp() const
{
    return d->sceTimestamp;
}

///
/// Sets the timestamp affix element's content as defined by
/// \xep{0420, Stanza Content Encryption} (SCE).
///
/// The SCE timestamp is part of an encrypted stanza's SCE envelope,
/// not an unencrypted direct child of a transmitted stanza and thus not de- /
/// serialized by it.
/// Instead, it is set by an encryption protocol such as
/// \xep{0384, OMEMO Encryption} after decryption.
/// It can be used by trust management protocols such as
/// \xep{0450, Automatic Trust Management (ATM)}.
///
/// \since QXmpp 1.5
///
void QXmppE2eeMetadata::setSceTimestamp(const QDateTime &timestamp)
{
    d->sceTimestamp = timestamp;
}

class QXmppStanzaPrivate : public QSharedData
{
public:
    QString to;
    QString from;
    QString id;
    QString lang;
    QXmppStanza::Error error;
    QXmppElementList extensions;
    QList<QXmppExtendedAddress> extendedAddresses;
    QXmppE2eeMetadata e2eeMetadata;
};

///
/// Constructs a QXmppStanza with the specified sender and recipient.
///
/// \param from
/// \param to
///
QXmppStanza::QXmppStanza(const QString &from, const QString &to)
    : d(new QXmppStanzaPrivate)
{
    d->to = to;
    d->from = from;
}

/// Constructs a copy of \a other.
QXmppStanza::QXmppStanza(const QXmppStanza &other) = default;

/// Destroys a QXmppStanza.
QXmppStanza::~QXmppStanza() = default;

/// Assigns \a other to this stanza.
QXmppStanza &QXmppStanza::operator=(const QXmppStanza &other) = default;

///
/// Returns the stanza's recipient JID.
///
QString QXmppStanza::to() const
{
    return d->to;
}

///
/// Sets the stanza's recipient JID.
///
/// \param to
///
void QXmppStanza::setTo(const QString &to)
{
    d->to = to;
}

///
/// Returns the stanza's sender JID.
///
QString QXmppStanza::from() const
{
    return d->from;
}

///
/// Sets the stanza's sender JID.
///
/// \param from
///
void QXmppStanza::setFrom(const QString &from)
{
    d->from = from;
}

///
/// Returns the stanza's identifier.
///
QString QXmppStanza::id() const
{
    return d->id;
}

///
/// Sets the stanza's identifier.
///
/// \param id
///
void QXmppStanza::setId(const QString &id)
{
    d->id = id;
}

///
/// Returns the stanza's language.
///
QString QXmppStanza::lang() const
{
    return d->lang;
}

///
/// Sets the stanza's language.
///
/// \param lang
///
void QXmppStanza::setLang(const QString &lang)
{
    d->lang = lang;
}

///
/// Returns the stanza's error.
///
QXmppStanza::Error QXmppStanza::error() const
{
    return d->error;
}

///
/// Sets the stanza's error.
///
/// \param error
///
void QXmppStanza::setError(const QXmppStanza::Error &error)
{
    d->error = error;
}

///
/// Returns the stanza's "extensions".
///
/// Extensions are XML elements which are not handled internally by QXmpp.
///
QXmppElementList QXmppStanza::extensions() const
{
    return d->extensions;
}

///
/// Sets the stanza's "extensions".
///
/// \param extensions
///
void QXmppStanza::setExtensions(const QXmppElementList &extensions)
{
    d->extensions = extensions;
}

///
/// Returns the stanza's extended addresses as defined by \xep{0033, Extended
/// Stanza Addressing}.
///
QList<QXmppExtendedAddress> QXmppStanza::extendedAddresses() const
{
    return d->extendedAddresses;
}

///
/// Sets the stanza's extended addresses as defined by \xep{0033, Extended
/// Stanza Addressing}.
///
void QXmppStanza::setExtendedAddresses(const QList<QXmppExtendedAddress> &addresses)
{
    d->extendedAddresses = addresses;
}

///
/// Returns additional data for end-to-end encryption purposes.
///
/// \since QXmpp 1.5
///
QXmppE2eeMetadata QXmppStanza::e2eeMetadata() const
{
    return d->e2eeMetadata;
}

///
/// Sets additional data for end-to-end encryption purposes.
///
/// \since QXmpp 1.5
///
void QXmppStanza::setE2eeMetadata(const QXmppE2eeMetadata &e2eeMetadata)
{
    d->e2eeMetadata = e2eeMetadata;
}

/// \cond
void QXmppStanza::generateAndSetNextId()
{
    // get back
    ++s_uniqeIdNo;
    d->id = "qxmpp" + QString::number(s_uniqeIdNo);
}

void QXmppStanza::parse(const QDomElement &element)
{
    d->from = element.attribute("from");
    d->to = element.attribute("to");
    d->id = element.attribute("id");
    d->lang = element.attribute("lang");

    QDomElement errorElement = element.firstChildElement("error");
    if (!errorElement.isNull())
        d->error.parse(errorElement);

    // XEP-0033: Extended Stanza Addressing
    QDomElement addressElement = element.firstChildElement("addresses").firstChildElement("address");
    while (!addressElement.isNull()) {
        QXmppExtendedAddress address;
        address.parse(addressElement);
        if (address.isValid())
            d->extendedAddresses << address;
        addressElement = addressElement.nextSiblingElement("address");
    }
}

void QXmppStanza::extensionsToXml(QXmlStreamWriter *xmlWriter, QXmpp::SceMode sceMode) const
{
    // XEP-0033: Extended Stanza Addressing
    if (sceMode & QXmpp::ScePublic && !d->extendedAddresses.isEmpty()) {
        xmlWriter->writeStartElement("addresses");
        xmlWriter->writeDefaultNamespace(ns_extended_addressing);
        for (const auto &address : d->extendedAddresses)
            address.toXml(xmlWriter);
        xmlWriter->writeEndElement();
    }

    // other extensions
    for (const auto &extension : d->extensions)
        extension.toXml(xmlWriter);
}

/// \endcond
