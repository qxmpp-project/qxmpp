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

#include "QXmppGeolocItem.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QFloat16>
#include <QUrl>
#include <QXmlStreamWriter>

/// \cond
class QXmppGeolocItemPrivate : public QSharedData
{
public:
    QXmppGeolocItemPrivate();

    quint8 accuracy;
    QString country;
    qfloat16 lat;
    QString locality;
    qfloat16 lon;
};

QXmppGeolocItemPrivate::QXmppGeolocItemPrivate()
{
}
/// \endcond

///
/// \class QXmppGeolocItem
///
/// This class represents a PubSub item for \xep{0080, User Location}.
///
/// \since QXmpp 1.5
///

///
/// Default constructor
///
QXmppGeolocItem::QXmppGeolocItem()
    : d(new QXmppGeolocItemPrivate)
{
}

/// Copy-constructor.
QXmppGeolocItem::QXmppGeolocItem(const QXmppGeolocItem &other) = default;

QXmppGeolocItem::~QXmppGeolocItem() = default;

/// Assignment operator.
QXmppGeolocItem &QXmppGeolocItem::operator=(const QXmppGeolocItem &other) = default;

///
/// Returns the horizontal GPS error in meters.
///
quint8 QXmppGeolocItem::accuracy() const
{
    return d->accuracy;
}

///
/// Sets the horizontal GPS error.
///
void QXmppGeolocItem::setAccuracy(const quint8 &accuracy)
{
    d->accuracy = accuracy;
}

///
/// Returns the country.
///
QString QXmppGeolocItem::country() const
{
    return d->country;
}

///
/// Sets the country.
///
void QXmppGeolocItem::setCountry(const QString &country)
{
    d->country = country;
}

///
/// Returns the latitude in decimal degrees.
///
qfloat16 QXmppGeolocItem::lat() const
{
    return d->lat;
}

///
/// Sets the latitude.
///
void QXmppGeolocItem::setLat(qfloat16 &lat)
{
    if (lat > 90 || lat < -90)
        d->lat = 0;
    else
        d->lat = lat;
}

///
/// Returns the locality such as a town or a city.
///
QString QXmppGeolocItem::locality() const
{
    return d->locality;
}

///
/// Sets the locality.
///
void QXmppGeolocItem::setLocality(const QString &locality)
{
    d->locality = locality;
}

///
/// Returns the longitude in decimal degrees.
///
qfloat16 QXmppGeolocItem::lon() const
{
    return d->lon;
}

///
/// Sets the longitude.
///
void QXmppGeolocItem::setLon(qfloat16 &lon)
{
    if (lon > 180 || lon < -180)
        d->lon = 0;
    else
        d->lon = lon;
}

///
/// Returns true, if the element is a valid \xep{0080}: User Location PubSub item.
///
bool QXmppGeolocItem::isItem(const QDomElement &itemElement)
{
    auto isPayloadValid = [](const QDomElement &payload) -> bool {
        return payload.tagName() == QStringLiteral("tune") &&
            payload.namespaceURI() == ns_geoloc;
    };

    return QXmppPubSubItem::isItem(itemElement, isPayloadValid);
}

/// \cond
void QXmppGeolocItem::parsePayload(const QDomElement &tune)
{
    auto child = tune.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QStringLiteral("accuracy")) {
            d->accuracy = child.text().toUShort();
        } else if (child.tagName() == QStringLiteral("country")) {
            d->country = child.text();
        } else if (child.tagName() == QStringLiteral("lat")) {
            bool ok = false;
            d->lat = child.text().toLong(&ok);
            if (!ok || d->lat > 90 || d->lat < -90) {
                d->lat = 0;
            }
        } else if (child.tagName() == QStringLiteral("locality")) {
            d->locality = child.text();
        } else if (child.tagName() == QStringLiteral("lon")) {
            bool ok = false;
            d->lon = child.text().toLong(&ok);
            if (!ok || d->lon > 180 || d->lon < -180) {
                d->lon = 0;
            }
        }

        child = child.nextSiblingElement();
    }
}

void QXmppGeolocItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("geoloc"));
    writer->writeDefaultNamespace(ns_geoloc);

    helperToXmlAddTextElement(writer, QStringLiteral("accuracy"), QString::number(d->accuracy));
    writer->writeTextElement(QStringLiteral("country"), d->country);
    writer->writeTextElement(QStringLiteral("lat"), QString::number(d->lat));
    helperToXmlAddTextElement(writer, QStringLiteral("locality"), d->locality);
    helperToXmlAddTextElement(writer, QStringLiteral("lon"), QString::number(d->lon));

    writer->writeEndElement();
}
/// \endcond
