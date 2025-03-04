// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSharedData>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

class QXmppBitsOfBinaryDataPrivate : public QSharedData
{
public:
    QXmppBitsOfBinaryDataPrivate();

    QXmppBitsOfBinaryContentId cid;
    int maxAge;
    QMimeType contentType;
    QByteArray data;
};

QXmppBitsOfBinaryDataPrivate::QXmppBitsOfBinaryDataPrivate()
    : maxAge(-1)
{
}

///
/// \class QXmppBitsOfBinaryData
///
/// QXmppBitsOfBinaryData represents a data element for \xep{0231, Bits of
/// Binary}. It can be used as an extension in other stanzas.
///
/// \see QXmppBitsOfBinaryIq, QXmppBitsOfBinaryDataList
///
/// \since QXmpp 1.2
///

///
/// Creates bits of binary data from a QByteArray.
///
/// This hashes the data to generate a content ID. The MIME type is not set.
///
/// \note This blocks while hashing the data. You may want to run this via QtConcurrent or a
/// QThreadPool to run this on for large amounts of data.
///
/// \since QXmpp 1.5
///
QXmppBitsOfBinaryData QXmppBitsOfBinaryData::fromByteArray(QByteArray data)
{
    QXmppBitsOfBinaryContentId cid;
    cid.setHash(QCryptographicHash::hash(data, QCryptographicHash::Sha1));
    cid.setAlgorithm(QCryptographicHash::Sha1);

    QXmppBitsOfBinaryData bobData;
    bobData.d->cid = std::move(cid);
    bobData.d->data = std::move(data);

    return bobData;
}

///
/// Default constructor
///
QXmppBitsOfBinaryData::QXmppBitsOfBinaryData()
    : d(new QXmppBitsOfBinaryDataPrivate)
{
}

/// Default copy-constructor
QXmppBitsOfBinaryData::QXmppBitsOfBinaryData(const QXmppBitsOfBinaryData &) = default;
/// Default move-constructor
QXmppBitsOfBinaryData::QXmppBitsOfBinaryData(QXmppBitsOfBinaryData &&) = default;
/// Default destructor
QXmppBitsOfBinaryData::~QXmppBitsOfBinaryData() = default;
/// Default assignment operator
QXmppBitsOfBinaryData &QXmppBitsOfBinaryData::operator=(const QXmppBitsOfBinaryData &) = default;
/// Default move-assignment operator
QXmppBitsOfBinaryData &QXmppBitsOfBinaryData::operator=(QXmppBitsOfBinaryData &&) = default;

///
/// Returns the content id of the data
///
QXmppBitsOfBinaryContentId QXmppBitsOfBinaryData::cid() const
{
    return d->cid;
}

///
/// Sets the content id of the data
///
void QXmppBitsOfBinaryData::setCid(const QXmppBitsOfBinaryContentId &cid)
{
    d->cid = cid;
}

///
/// Returns the time in seconds the data should be cached
///
/// A value of 0 means that the data should not be cached, while a value of -1
/// means that nothing was set.
///
/// The default value is -1.
///
int QXmppBitsOfBinaryData::maxAge() const
{
    return d->maxAge;
}

///
/// Sets the time in seconds the data should be cached
///
/// A value of 0 means that the data should not be cached, while a value of -1
/// means that nothing was set.
///
/// The default value is -1.
///
void QXmppBitsOfBinaryData::setMaxAge(int maxAge)
{
    d->maxAge = maxAge;
}

///
/// Returns the content type of the data
///
/// \note This is the advertised content type and may differ from the actual
/// content type of the data.
///
QMimeType QXmppBitsOfBinaryData::contentType() const
{
    return d->contentType;
}

///
/// Sets the content type of the data
///
void QXmppBitsOfBinaryData::setContentType(const QMimeType &contentType)
{
    d->contentType = contentType;
}

///
/// Returns the included data in binary form
///
QByteArray QXmppBitsOfBinaryData::data() const
{
    return d->data;
}

///
/// Sets the data in binary form
///
void QXmppBitsOfBinaryData::setData(const QByteArray &data)
{
    d->data = data;
}

///
/// Returns true, if \c element is a \xep{0231, Bits of Binary} data element
///
bool QXmppBitsOfBinaryData::isBitsOfBinaryData(const QDomElement &element)
{
    return element.tagName() == u"data" && element.namespaceURI() == ns_bob;
}

/// \cond
void QXmppBitsOfBinaryData::parseElementFromChild(const QDomElement &dataElement)
{
    d->cid = QXmppBitsOfBinaryContentId::fromContentId(dataElement.attribute(u"cid"_s));
    d->maxAge = dataElement.attribute(u"max-age"_s, u"-1"_s).toInt();
    d->contentType = QMimeDatabase().mimeTypeForName(dataElement.attribute(u"type"_s));
    d->data = QByteArray::fromBase64(dataElement.text().toUtf8());
}

void QXmppBitsOfBinaryData::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("data"));
    writer->writeDefaultNamespace(toString65(ns_bob));
    writeOptionalXmlAttribute(writer, u"cid", d->cid.toContentId());
    if (d->maxAge > -1) {
        writeOptionalXmlAttribute(writer, u"max-age", QString::number(d->maxAge));
    }
    writeOptionalXmlAttribute(writer, u"type", d->contentType.name());
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    writer->writeCharacters(d->data.toBase64());
#else
    writer->writeCharacters(QString::fromUtf8(d->data.toBase64()));
#endif
    writer->writeEndElement();
}
/// \endcond

///
/// Returns true, if cid, maxAge, contentType and data equal.
///
bool QXmppBitsOfBinaryData::operator==(const QXmppBitsOfBinaryData &other) const
{
    return d->cid == other.cid() &&
        d->maxAge == other.maxAge() &&
        d->contentType == other.contentType() &&
        d->data == other.data();
}

///
/// \class QXmppBitsOfBinaryDataList
///
/// QXmppBitsOfBinaryDataList represents a list of data elements from
/// \xep{0231, Bits of Binary}.
///
/// \since QXmpp 1.2
///

QXmppBitsOfBinaryDataList::QXmppBitsOfBinaryDataList() = default;

QXmppBitsOfBinaryDataList::~QXmppBitsOfBinaryDataList() = default;

/// \cond

///
/// \brief Finds the data for a uri
/// \return Data belonging to the uri
///
/// \since QXmpp 1.5
///
std::optional<QXmppBitsOfBinaryData> QXmppBitsOfBinaryDataList::find(const QXmppBitsOfBinaryContentId &cid) const
{
    const auto thumbnailData = std::find_if(begin(), end(), [&](const auto &bobBlob) {
        return bobBlob.cid() == cid;
    });

    if (thumbnailData != end()) {
        return *thumbnailData;
    }
    return {};
}

void QXmppBitsOfBinaryDataList::parse(const QDomElement &element)
{
    // clear previous data elements
    clear();

    // parse all <data/> elements
    for (const auto &child : iterChildElements(element)) {
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(child)) {
            QXmppBitsOfBinaryData data;
            data.parseElementFromChild(child);
            append(data);
        }
    }
}

void QXmppBitsOfBinaryDataList::toXml(QXmlStreamWriter *writer) const
{
    for (const auto &bitsOfBinaryData : *this) {
        bitsOfBinaryData.toXmlElementFromChild(writer);
    }
}
/// \endcond
