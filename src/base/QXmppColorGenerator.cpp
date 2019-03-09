/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Linus Jahn <lnj@kaidan.im>
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

#include "QXmppColorGenerator.h"
#include "hsluv.h"
#include <ctgmath>
#include <QCryptographicHash>

/// \brief Generates a color from the input value. This is intended for
/// generating colors for contacts. The generated colors are "consistent", so
/// they are shared between all clients with support for XEP-0392: Consistent
/// Color Generation.
///
/// \param name This should be the (user-specified) nickname of the
/// participant. If there is no nickname set, the bare JID shall be used.
/// \param deficiency Color correction to be done, defaults to
/// ColorVisionDeficiency::NoDeficiency.

QXmppColorGenerator::RGBColor QXmppColorGenerator::generateColor(
        const QString &name, ColorVisionDeficiency deficiency)
{
    QByteArray input = name.toUtf8();

    // hash input through SHA-1
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(input);

    // the first two bytes are used to calculate the angle/hue
    int angle = hash.result().at(0) + hash.result().at(1) * 256;

    double hue = double(angle) / 65536.0 * 360.0;
    double saturation = 100.0;
    double lightness = 50.0;

    // Corrections for Color Vision Deficiencies
    // this uses floating point modulo (fmod)
    if (deficiency == RedGreenBlindness) {
        hue += 90.0;
        hue = fmod(hue, 180);
        hue -= 90.0;
        hue = fmod(hue, 360);
    } else if (deficiency == BlueBlindness) {
        hue = fmod(hue, 180);
    }

    // convert to rgb values (values are between 0.0 and 1.0)
    double red, green, blue = 0.0;
    hsluv2rgb(hue, saturation, lightness, &red, &green, &blue);

    RGBColor color;
    color.red = quint8(red * 255.0);
    color.green = quint8(green * 255.0);
    color.blue = quint8(blue * 255.0);
    return color;
}
