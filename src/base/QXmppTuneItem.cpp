/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#include "QXmppTuneItem.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QUrl>
#include <QXmlStreamWriter>

/// \cond
class QXmppTuneItemPrivate : public QSharedData
{
public:
    QXmppTuneItemPrivate();

    QString artist;
    quint16 length;
    quint8 rating;
    QString source;
    QString title;
    QString track;
    QUrl uri;
};

QXmppTuneItemPrivate::QXmppTuneItemPrivate()
    : length(0),
      rating(0)
{
}
/// \endcond

///
/// \class QXmppTuneItem
///
/// This class represents a PubSub item for \xep{0118, User Tune}.
///
/// \since QXmpp 1.5
///

///
/// Default constructor
///
QXmppTuneItem::QXmppTuneItem()
    : d(new QXmppTuneItemPrivate)
{
}

/// Copy-constructor.
QXmppTuneItem::QXmppTuneItem(const QXmppTuneItem &other) = default;

QXmppTuneItem::~QXmppTuneItem() = default;

/// Assignment operator.
QXmppTuneItem &QXmppTuneItem::operator=(const QXmppTuneItem &other) = default;

///
/// Returns the artist of the piece or song.
///
QString QXmppTuneItem::artist() const
{
    return d->artist;
}

///
/// Sets the artist of the piece or song.
///
void QXmppTuneItem::setArtist(const QString &artist)
{
    d->artist = artist;
}

///
/// Returns the length of the piece in seconds (0 means unknown).
///
quint16 QXmppTuneItem::length() const
{
    return d->length;
}

///
/// Sets the length of the piece in seconds (0 means unknown).
///
void QXmppTuneItem::setLength(quint16 length)
{
    d->length = length;
}

///
/// Returns the user's rating of the song or piece (from 1 to 10), 0 means
/// invalid or unknown.
///
quint8 QXmppTuneItem::rating() const
{
    return d->rating;
}

///
/// Sets the user's rating of the song or piece (from 1 to 10), 0 means invalid
/// or unknown.
///
void QXmppTuneItem::setRating(quint8 rating)
{
    if (rating > 10)
        d->rating = 0;
    else
        d->rating = rating;
}

///
/// Returns the album, other collection or other source (e.g. website) of the
/// piece.
///
QString QXmppTuneItem::source() const
{
    return d->source;
}

///
/// Sets the album, other collection or other source (e.g. website) of the
/// piece.
///
void QXmppTuneItem::setSource(const QString &source)
{
    d->source = source;
}

///
/// Returns the title of the piece.
///
QString QXmppTuneItem::title() const
{
    return d->title;
}

///
/// Sets the title of the piece.
///
void QXmppTuneItem::setTitle(const QString &title)
{
    d->title = title;
}

///
/// Returns the track number or other identifier in the collection or source.
///
QString QXmppTuneItem::track() const
{
    return d->track;
}

///
/// Sets the track number or other identifier in the collection or source.
///
void QXmppTuneItem::setTrack(const QString &track)
{
    d->track = track;
}

///
/// Returns an URI or URL pointing to information about the song, collection or
/// artist.
///
QUrl QXmppTuneItem::uri() const
{
    return d->uri;
}

///
/// Sets an URI or URL pointing to information about the song, collection or
/// artist.
///
void QXmppTuneItem::setUri(const QUrl &uri)
{
    d->uri = uri;
}

///
/// Returns true, if the element is a valid \xep{0118}: User Tune PubSub item.
///
bool QXmppTuneItem::isItem(const QDomElement &itemElement)
{
    auto isPayloadValid = [](const QDomElement &payload) -> bool {
        return payload.tagName() == QStringLiteral("tune") &&
            payload.namespaceURI() == ns_tune;
    };

    return QXmppPubSubItem::isItem(itemElement, isPayloadValid);
}

/// \cond
void QXmppTuneItem::parsePayload(const QDomElement &tune)
{
    auto child = tune.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QStringLiteral("artist")) {
            d->artist = child.text();
        } else if (child.tagName() == QStringLiteral("length")) {
            bool ok = false;
            d->length = child.text().toUShort(&ok);

            if (!ok) {
                d->length = 0;
            }
        } else if (child.tagName() == QStringLiteral("rating")) {
            bool ok = false;
            d->rating = child.text().toUShort(&ok);

            if (!ok || d->rating > 10) {
                d->rating = 0;
            }
        } else if (child.tagName() == QStringLiteral("source")) {
            d->source = child.text();
        } else if (child.tagName() == QStringLiteral("title")) {
            d->title = child.text();
        } else if (child.tagName() == QStringLiteral("track")) {
            d->track = child.text();
        } else if (child.tagName() == QStringLiteral("uri")) {
            d->uri = QUrl(child.text());
        }
        child = child.nextSiblingElement();
    }
}

void QXmppTuneItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("tune"));
    writer->writeDefaultNamespace(ns_tune);

    helperToXmlAddTextElement(writer, QStringLiteral("artist"), d->artist);
    if (d->length != 0)
        writer->writeTextElement(QStringLiteral("length"), QString::number(d->length));
    if (d->rating != 0)
        writer->writeTextElement(QStringLiteral("rating"), QString::number(d->rating));
    helperToXmlAddTextElement(writer, QStringLiteral("source"), d->source);
    helperToXmlAddTextElement(writer, QStringLiteral("title"), d->title);
    helperToXmlAddTextElement(writer, QStringLiteral("track"), d->track);
    helperToXmlAddTextElement(writer, QStringLiteral("uri"), d->uri.toString(QUrl::FullyEncoded));

    writer->writeEndElement();
}
/// \endcond
