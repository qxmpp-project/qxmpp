// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStickerPackItem.h"

#include "QXmppConstants_p.h"
#include "QXmppEncryptedFileSource.h"
#include "QXmppFileMetadata.h"
#include "qdebug.h"

#include <QXmlStreamWriter>

class QXmppStickerItemPrivate : public QSharedData
{
public:
    QXmppFileMetadata metadata;
    QVector<QXmppHttpFileSource> httpSources;
    QVector<QXmppEncryptedFileSource> encryptedSources;
    QVector<QString> suggest;
};

QXmppStickerItem::QXmppStickerItem()
    : d(new QXmppStickerItemPrivate())
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppStickerItem)

///
/// \class QXmppStickerItem
///
/// This class represents a single sticker when publishing or retrieving it.
///
/// \since QXmpp 1.5
///

///
/// \brief Returns metadata about the sticker file
///
const QXmppFileMetadata &QXmppStickerItem::metadata() const
{
    return d->metadata;
}

///
/// \brief Sets metadata of this sticker file
///
void QXmppStickerItem::setMetadata(const QXmppFileMetadata &metadata)
{
    d->metadata = metadata;
}

///
/// \brief Returns HTTP sources for the sticker file
///
const QVector<QXmppHttpFileSource> &QXmppStickerItem::httpSource() const
{
    return d->httpSources;
}

///
/// \brief Sets the list of HTTP sources for this sticker file
///
void QXmppStickerItem::setHttpSources(const QVector<QXmppHttpFileSource> &httpSources)
{
    d->httpSources = httpSources;
}

///
/// \brief Returns the list of encrypted sources for this sticker file
///
const QVector<QXmppEncryptedFileSource> &QXmppStickerItem::encryptedSources() const
{
    return d->encryptedSources;
}

///
/// \brief Set the list of encrypted sources for this sticker file
///
void QXmppStickerItem::setEncryptedSources(const QVector<QXmppEncryptedFileSource> &encryptedSources)
{
    d->encryptedSources = encryptedSources;
}

///
/// \brief Returns words for which apps can suggest the use of this sticker
///
const QVector<QString> &QXmppStickerItem::suggestedWords() const
{
    return d->suggest;
}

///
/// \brief Sets the words for which apps can suggest the use of this sticker
///
void QXmppStickerItem::setSuggestedWords(const QVector<QString> &suggest)
{
    d->suggest = suggest;
}

/// \cond
void QXmppStickerItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    d->metadata.toXml(writer);
    writer->writeStartElement("sources");
    writer->writeDefaultNamespace(ns_sfs);
    for (const auto &httpSource : d->httpSources) {
        httpSource.toXml(writer);
    }
    for (const auto &encryptedSource : d->encryptedSources) {
        encryptedSource.toXml(writer);
    }
    writer->writeEndElement();
    for (const auto &word : d->suggest) {
        writer->writeTextElement("suggest", word);
    }
    writer->writeEndElement();
}

bool QXmppStickerItem::parse(const QDomElement &element)
{
    auto fileElement = element.firstChildElement("file");
    d->metadata.parse(fileElement);

    auto sources = element.firstChildElement("sources");
    for (auto sourceEl = sources.firstChildElement();
         !sourceEl.isNull();
         sourceEl = sourceEl.nextSiblingElement()) {
        if (sourceEl.tagName() == QStringLiteral("url-data")) {
            QXmppHttpFileSource source;
            if (source.parse(sourceEl)) {
                d->httpSources.push_back(std::move(source));
            }
        } else if (sourceEl.tagName() == QStringLiteral("encrypted")) {
            QXmppEncryptedFileSource source;
            if (source.parse(sourceEl)) {
                d->encryptedSources.push_back(std::move(source));
            }
        }
    }

    for (auto suggestEl = element.firstChildElement("suggest");
         !suggestEl.isNull();
         suggestEl = suggestEl.nextSiblingElement("suggest")) {
        d->suggest.push_back(suggestEl.text());
    }

    return true;
}
/// \endcond

class QXmppStickerPackItemPrivate : public QSharedData
{
public:
    QString name;
    QString summary;
    QVector<QXmppStickerItem> items;
    bool restricted = false;
    QXmppHash hash;
};

QXmppStickerPackItem::QXmppStickerPackItem()
    : d(new QXmppStickerPackItemPrivate())
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppStickerPackItem)

///
/// \class QXmppStickerPackitem
///
/// A pubsub item that represents a sticker pack.
///
/// \since QXmpp 1.5
///

///
/// \brief Returns the name of the sticker pack
///
const QString &QXmppStickerPackItem::name() const
{
    return d->name;
}

///
/// \brief Sets the name of the sticker pack
///
void QXmppStickerPackItem::setName(const QString &name)
{
    d->name = name;
}

///
/// \brief Returns the summary of this sticker pack
///
const QString &QXmppStickerPackItem::summary() const
{
    return d->summary;
}

///
/// \brief Sets the summary of the sticker pack
///
void QXmppStickerPackItem::setSummary(const QString &summary)
{
    d->summary = summary;
}

///
/// \brief Returns the list of stickers of this pack
///
const QVector<QXmppStickerItem> &QXmppStickerPackItem::items() const
{
    return d->items;
}

///
/// \brief Set the list of stickers for this pack
///
void QXmppStickerPackItem::setItems(const QVector<QXmppStickerItem> &items)
{
    d->items = items;
}

///
/// \brief Returns whether this sticker pack can be freely imported
///
bool QXmppStickerPackItem::restricted() const
{
    return d->restricted;
}

///
/// \brief Set whether this sticker pack should be importable by others
///
void QXmppStickerPackItem::setRestricted(bool restricted)
{
    d->restricted = restricted;
}

void QXmppStickerPackItem::parsePayload(const QDomElement &payloadElement)
{
    d->name = payloadElement.firstChildElement("name").text();
    d->summary = payloadElement.firstChildElement("summary").text();

    for (auto firstChild = payloadElement.firstChildElement("item");
         !firstChild.isNull();
         firstChild = firstChild.nextSiblingElement("item")) {
        QXmppStickerItem stickerItem;
        stickerItem.parse(firstChild);

        d->items.push_back(std::move(stickerItem));
    }

    d->hash.parse(payloadElement.firstChildElement("hash"));
    d->restricted = !payloadElement.firstChildElement("restricted").isNull();
}

void QXmppStickerPackItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("pack");
    writer->writeDefaultNamespace(ns_stickers);

    writer->writeTextElement("name", d->name);
    writer->writeTextElement("summary", d->summary);

    for (const auto &item : d->items) {
        item.toXml(writer);
    }

    d->hash.toXml(writer);

    if (d->restricted) {
        writer->writeEmptyElement("restricted");
    }
    writer->writeEndElement();
}
