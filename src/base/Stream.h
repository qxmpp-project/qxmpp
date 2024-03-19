// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef STREAM_H
#define STREAM_H

#include <QString>

class QXmlStreamWriter;

namespace QXmpp::Private {

struct StreamOpen {
    void toXml(QXmlStreamWriter *) const;

    QString to;
    QString from;
    QStringView xmlns;
};

}  // namespace QXmpp::Private

#endif  // STREAM_H
