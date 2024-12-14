// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppColorGeneration.h"

#include "hsluv/hsluv.h"

#include <QCryptographicHash>

QXmppColorGeneration::Rgb QXmppColorGeneration::generateRgb(QStringView str)
{
    QByteArray input = str.toUtf8();

    // hash input through SHA-1
    auto hashValue = QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Sha1);

    // the first two bytes are used to calculate the angle/hue
    int angle = hashValue.at(0) + hashValue.at(1) * 256;

    double hue = double(angle) / 65536.0 * 360.0;
    double saturation = 100.0;
    double lightness = 50.0;

    // convert to rgb values (values are between 0.0 and 1.0)
    double red, green, blue = 0.0;
    hsluv2rgb(hue, saturation, lightness, &red, &green, &blue);

    return Rgb { quint8(red * 255.0), quint8(green * 255.0), quint8(blue * 255.0) };
}
