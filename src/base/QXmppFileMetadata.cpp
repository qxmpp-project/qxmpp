// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppFileMetadata.h"

#include "QXmppConstants_p.h"
#include "QXmppHash.h"
#include "QXmppThumbnail.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Algorithms.h"
#include "StringLiterals.h"

#include <utility>

#include <QDateTime>
#include <QDomElement>
#include <QFileInfo>
#include <QMimeDatabase>

using namespace QXmpp::Private;

class QXmppFileMetadataPrivate : public QSharedData
{
public:
    std::optional<QDateTime> date;
    std::optional<QString> desc;
    QVector<QXmppHash> hashes;
    std::optional<uint32_t> height;
    std::optional<uint32_t> length;
    std::optional<QMimeType> mediaType;
    std::optional<QString> name;
    std::optional<uint64_t> size;
    QVector<QXmppThumbnail> thumbnails;
    std::optional<uint32_t> width;
};

///
/// \class QXmppFileMetadata
///
/// File metadata from \xep{0446, File metadata element}.
///
/// \since QXmpp 1.5
///

///
/// \brief Creates a QXmppFileMetadata object from information from QFileInfo.
///
/// Sets the filename, file size, media type and the last modification date.
///
QXmppFileMetadata QXmppFileMetadata::fromFileInfo(const QFileInfo &info)
{
    QXmppFileMetadata metadata;
    metadata.setFilename(info.fileName());
    metadata.setSize(info.size());
    metadata.setMediaType(QMimeDatabase().mimeTypeForFile(info));
    metadata.setLastModified(info.lastModified());
    return metadata;
}

QXmppFileMetadata::QXmppFileMetadata()
    : d(new QXmppFileMetadataPrivate())
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppFileMetadata)

/// \cond
template<typename ElementType>
QVector<ElementType> parseChildElements(const QDomElement &el, QStringView tagName, QStringView namespaceUri)
{
    QVector<ElementType> vec;
    for (const auto &subEl : iterChildElements(el, tagName, namespaceUri)) {
        ElementType parsedElement;
        parsedElement.parse(subEl);
        vec.push_back(std::move(parsedElement));
    }
    return vec;
}

bool QXmppFileMetadata::parse(const QDomElement &el)
{
    if (el.isNull()) {
        return false;
    }

    if (auto dateEl = firstChildElement(el, u"date"); !dateEl.isNull()) {
        d->date = QXmppUtils::datetimeFromString(dateEl.text());
    }

    if (auto descEl = firstChildElement(el, u"desc"); !descEl.isNull()) {
        d->desc = descEl.text();
    }

    d->hashes = parseChildElements<QXmppHash>(el, u"hash", ns_hashes);

    if (auto heightEl = firstChildElement(el, u"height"); !heightEl.isNull()) {
        d->height = firstChildElement(el, u"height").text().toUInt();
    }
    if (auto lengthEl = firstChildElement(el, u"length"); !lengthEl.isNull()) {
        d->length = lengthEl.text().toUInt();
    }
    if (auto mediaTypeEl = firstChildElement(el, u"media-type"); !mediaTypeEl.isNull()) {
        d->mediaType = QMimeDatabase().mimeTypeForName(mediaTypeEl.text());
    }
    if (auto nameEl = firstChildElement(el, u"name"); !nameEl.isNull()) {
        d->name = nameEl.text();
    }
    if (auto sizeEl = firstChildElement(el, u"size"); !sizeEl.isNull()) {
        d->size = sizeEl.text().toULong();
    }
    for (const auto &thumbEl : iterChildElements(el, u"thumbnail")) {
        QXmppThumbnail thumbnail;
        if (thumbnail.parse(thumbEl)) {
            d->thumbnails.append(std::move(thumbnail));
        }
    }
    if (auto widthEl = firstChildElement(el, u"width"); !widthEl.isNull()) {
        d->width = widthEl.text().toUInt();
    }
    return true;
}

void QXmppFileMetadata::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("file"));
    writer->writeDefaultNamespace(toString65(ns_file_metadata));
    if (d->date) {
        writer->writeTextElement(QSL65("date"), QXmppUtils::datetimeToString(*d->date));
    }

    if (d->desc) {
        writer->writeTextElement(QSL65("desc"), *d->desc);
    }

    for (const auto &hash : d->hashes) {
        hash.toXml(writer);
    }

    if (d->height) {
        writer->writeTextElement(QSL65("height"), QString::number(*d->height));
    }
    if (d->length) {
        writer->writeTextElement(QSL65("length"), QString::number(*d->length));
    }
    if (d->mediaType) {
        writer->writeTextElement(QSL65("media-type"), d->mediaType->name());
    }
    if (d->name) {
        writer->writeTextElement(QSL65("name"), *d->name);
    }
    if (d->size) {
        writer->writeTextElement(QSL65("size"), QString::number(*d->size));
    }
    for (const auto &thumbnail : d->thumbnails) {
        thumbnail.toXml(writer);
    }
    if (d->width) {
        writer->writeTextElement(QSL65("width"), QString::number(*d->width));
    }
    writer->writeEndElement();
}
/// \endcond

/// Returns when the file was last modified
const std::optional<QDateTime> &QXmppFileMetadata::lastModified() const
{
    return d->date;
}

/// Sets when the file was last modified
void QXmppFileMetadata::setLastModified(const std::optional<QDateTime> &date)
{
    d->date = date;
}

/// Returns the description of the file
const std::optional<QString> &QXmppFileMetadata::description() const
{
    return d->desc;
}

/// Sets the description of the file
void QXmppFileMetadata::setDescription(const std::optional<QString> &description)
{
    d->desc = description;
}

/// Returns the hashes of the file
const QVector<QXmppHash> &QXmppFileMetadata::hashes() const
{
    return d->hashes;
}

/// Sets the hashes of the file
void QXmppFileMetadata::setHashes(const QVector<QXmppHash> &hashes)
{
    d->hashes = hashes;
}

/// Returns the height of the image
std::optional<uint32_t> QXmppFileMetadata::height() const
{
    return d->height;
}

/// Sets the height of the image
void QXmppFileMetadata::setHeight(std::optional<uint32_t> height)
{
    d->height = height;
}

/// Returns the length of a video or audio file
std::optional<uint32_t> QXmppFileMetadata::length() const
{
    return d->length;
}

/// Sets the length of a video or audio file
void QXmppFileMetadata::setLength(std::optional<uint32_t> length)
{
    d->length = length;
}

/// Returns the media type of the file
const std::optional<QMimeType> &QXmppFileMetadata::mediaType() const
{
    return d->mediaType;
}

/// Sets the media type of the file
void QXmppFileMetadata::setMediaType(std::optional<QMimeType> mediaType)
{
    d->mediaType = std::move(mediaType);
}

/// Returns the filename
std::optional<QString> QXmppFileMetadata::filename() const
{
    return d->name;
}

/// Sets the filename
void QXmppFileMetadata::setFilename(std::optional<QString> name)
{
    d->name = std::move(name);
}

/// Returns the size of the file in bytes
std::optional<uint64_t> QXmppFileMetadata::size() const
{
    return d->size;
}

/// Sets the size of the file in bytes
void QXmppFileMetadata::setSize(std::optional<uint64_t> size)
{
    d->size = size;
}

/// Returns the thumbnail references.
const QVector<QXmppThumbnail> &QXmppFileMetadata::thumbnails() const
{
    return d->thumbnails;
}

/// Sets the thumbnail references.
void QXmppFileMetadata::setThumbnails(const QVector<QXmppThumbnail> &thumbnail)
{
    d->thumbnails = thumbnail;
}

/// Returns the width of the image or video.
std::optional<uint32_t> QXmppFileMetadata::width() const
{
    return d->width;
}

/// Sets the width of the image or video.
void QXmppFileMetadata::setWidth(std::optional<uint32_t> width)
{
    d->width = width;
}
