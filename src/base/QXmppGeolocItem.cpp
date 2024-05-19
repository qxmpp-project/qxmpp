// SPDX-FileCopyrightText: 2022 Cochise César <cochisecesar@zoho.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppGeolocItem.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

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
        return payload.tagName() == u"geoloc" &&
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
    for (const auto &child : iterChildElements(tune)) {
        const auto tagName = child.tagName();
        if (tagName == u"accuracy") {
            d->accuracy = parseOptDouble(child);
        } else if (tagName == u"country") {
            d->country = child.text();
        } else if (tagName == u"lat") {
            setLatitude(parseOptDouble(child));
        } else if (tagName == u"locality") {
            d->locality = child.text();
        } else if (tagName == u"lon") {
            setLongitude(parseOptDouble(child));
        }
    }
}

auto writeTextEl(QXmlStreamWriter *writer, const QString &name, std::optional<double> val)
{
    if (val.has_value()) {
        writer->writeTextElement(name, QString::number(*val));
    }
}
auto writeTextEl(QXmlStreamWriter *writer, const QString &name, const QString &val)
{
    writeXmlTextElement(writer, name, val);
}

void QXmppGeolocItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("geoloc"));
    writer->writeDefaultNamespace(toString65(ns_geoloc));

    writeTextEl(writer, u"accuracy"_s, d->accuracy);
    writeTextEl(writer, u"country"_s, d->country);
    writeTextEl(writer, u"lat"_s, d->latitude);
    writeTextEl(writer, u"locality"_s, d->locality);
    writeTextEl(writer, u"lon"_s, d->longitude);

    writer->writeEndElement();
}
/// \endcond
