// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCOLORGENERATION_H
#define QXMPPCOLORGENERATION_H

#include "QXmppGlobal.h"

#ifdef QT_GUI_LIB
#include <QColor>
#endif

class QXmppColorGeneration
{
public:
    struct Rgb {
        quint8 red;
        quint8 green;
        quint8 blue;
    };

    static Rgb generateRgb(QStringView str);

#ifdef QT_GUI_LIB
    static QColor generateColor(QStringView str)
    {
        auto rgb = generateRgb(str);
        return QColor(rgb.red, rgb.green, rgb.blue);
    }
#endif
};

#endif  // QXMPPCOLORGENERATION_H
