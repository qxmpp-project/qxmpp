// SPDX-FileCopyrightText: 2022 Cochise César <cochisecesar@zoho.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppGeolocItem.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QXmlStreamWriter>

/// \cond
class QXmppGeolocItemPrivate : public QSharedData
{
public:
    std::optional<double> accuracy;
    QString country;
    QString locality;
    std::optional<double> latitude;
    std::optional<double> longitude;
};
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
/// Move-constructor.
QXmppGeolocItem::QXmppGeolocItem(QXmppGeolocItem &&) = default;
QXmppGeolocItem::~QXmppGeolocItem() = default;
/// Assignment operator.
QXmppGeolocItem &QXmppGeolocItem::operator=(const QXmppGeolocItem &other) = default;
/// Move-assignment operator.
QXmppGeolocItem &QXmppGeolocItem::operator=(QXmppGeolocItem &&) = default;

///
/// Returns the horizontal GPS error in meters.
///
std::optional<double> QXmppGeolocItem::accuracy() const
{
    return d->accuracy;
}

///
/// Sets the horizontal GPS error.
///
void QXmppGeolocItem::setAccuracy(std::optional<double> accuracy)
{
    d->accuracy = std::move(accuracy);
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
void QXmppGeolocItem::setCountry(QString country)
{
    d->country = std::move(country);
}

///
/// Returns the latitude in decimal degrees.
///
std::optional<double> QXmppGeolocItem::latitude() const
{
    return d->latitude;
}

///
/// Sets the latitude.
///
void QXmppGeolocItem::setLatitude(std::optional<double> lat)
{
    if (lat && (*lat > 90 || *lat < -90)) {
        d->latitude.reset();
        return;
    }
    d->latitude = std::move(lat);
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
void QXmppGeolocItem::setLocality(QString locality)
{
    d->locality = std::move(locality);
}

///
/// Returns the longitude in decimal degrees.
///
std::optional<double> QXmppGeolocItem::longitude() const
{
    return d->longitude;
}

///
/// Sets the longitude.
///
void QXmppGeolocItem::setLongitude(std::optional<double> lon)
{
    if (lon && (*lon > 180 || *lon < -180)) {
        d->longitude.reset();
        return;
    }
    d->longitude = std::move(lon);
}

///
/// Returns true, if the element is a valid \xep{0080}: User Location PubSub item.
///
bool QXmppGeolocItem::isItem(const QDomElement &itemElement)
{
    auto isPayloadValid = [](const QDomElement &payload) -> bool {
        return payload.tagName() == QStringLiteral("geoloc") &&
            payload.namespaceURI() == ns_geoloc;
    };

    return QXmppPubSubBaseItem::isItem(itemElement, isPayloadValid);
}

/// \cond
std::optional<double> parseOptDouble(const QDomElement &element)
{
    bool ok = false;
    if (auto val = element.text().toDouble(&ok); ok) {
        return val;
    }
    return {};
}

void QXmppGeolocItem::parsePayload(const QDomElement &tune)
{
    for (auto child = tune.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
        const auto tagName = child.tagName();
        if (tagName == QStringLiteral("accuracy")) {
            d->accuracy = parseOptDouble(child);
        } else if (tagName == QStringLiteral("country")) {
            d->country = child.text();
        } else if (tagName == QStringLiteral("lat")) {
            setLatitude(parseOptDouble(child));
        } else if (tagName == QStringLiteral("locality")) {
            d->locality = child.text();
        } else if (tagName == QStringLiteral("lon")) {
            setLongitude(parseOptDouble(child));
        }
    }
}

auto writeTextEl(QXmlStreamWriter *writer, const QString &name, const std::optional<double> &val)
{
    if (val.has_value()) {
        writer->writeTextElement(name, QString::number(*val));
    }
}
auto writeTextEl(QXmlStreamWriter *writer, const QString &name, const QString &val)
{
    helperToXmlAddTextElement(writer, name, val);
}

void QXmppGeolocItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("geoloc"));
    writer->writeDefaultNamespace(ns_geoloc);

    writeTextEl(writer, QStringLiteral("accuracy"), d->accuracy);
    writeTextEl(writer, QStringLiteral("country"), d->country);
    writeTextEl(writer, QStringLiteral("lat"), d->latitude);
    writeTextEl(writer, QStringLiteral("locality"), d->locality);
    writeTextEl(writer, QStringLiteral("lon"), d->longitude);

    writer->writeEndElement();
}
/// \endcond
