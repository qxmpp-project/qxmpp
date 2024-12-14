// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppColorGeneration.h"

#include "hsluv/hsluv.h"

#include <QCryptographicHash>

using namespace QXmpp::Private;

///
/// \class QXmppColorGeneration
///
/// \brief Generates colors from strings as defined in \xep{0392, Consistent Color Generation}.
///
/// This way the colors are unique between different clients.
///
/// \since QXmpp 1.10
///

///
/// Generates an RGB color triple for a string.
///
QXmppColorGeneration::Rgb QXmppColorGeneration::generateRgb(QStringView str)
{
    double hue = generateColorAngle(str);
    double saturation = 100.0;
    double lightness = 50.0;

    // convert to rgb values (values are between 0.0 and 1.0)
    double red, green, blue = 0.0;
    hsluv2rgb(hue, saturation, lightness, &red, &green, &blue);

    return Rgb { quint8(red * 255.0), quint8(green * 255.0), quint8(blue * 255.0) };
}

///
/// \fn QXmppColorGeneration::generateColor
///
/// Generates a QColor for a string.
///
/// \note This is a header-only function and it is only available when building the application
/// using QXmpp with the Qt GUI module.
///

double QXmpp::Private::generateColorAngle(QStringView str)
{
    QByteArray input = str.toUtf8();

    // hash input through SHA-1
    auto hashValue = QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Sha1);

    // the first two bytes are used to calculate the angle/hue
    auto hashData = reinterpret_cast<quint8 *>(hashValue.data());
    int angle = hashData[0] + hashData[1] * 256;

    return double(angle) / 65536.0 * 360.0;
}
