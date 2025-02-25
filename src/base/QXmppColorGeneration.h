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
    struct QXMPP_EXPORT Rgb {
        quint8 red;
        quint8 green;
        quint8 blue;
    };

    QXMPP_EXPORT static Rgb generateRgb(QStringView str);

#if defined(QT_GUI_LIB) || defined(QXMPP_DOC)
    static inline QColor generateColor(QStringView str)
    {
        auto rgb = generateRgb(str);
        return QColor(rgb.red, rgb.green, rgb.blue);
    }
#endif
};

namespace QXmpp::Private {
double generateColorAngle(QStringView str);
}

#endif  // QXMPPCOLORGENERATION_H
