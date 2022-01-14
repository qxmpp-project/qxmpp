// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBITSOFBINARYDATACONTAINER_H
#define QXMPPBITSOFBINARYDATACONTAINER_H

#include "QXmppBitsOfBinaryData.h"

#include <QVector>

class QDomElement;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppBitsOfBinaryDataList : public QVector<QXmppBitsOfBinaryData>
{
public:
    QXmppBitsOfBinaryDataList();
    ~QXmppBitsOfBinaryDataList();

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond
};

#endif  // QXMPPBITSOFBINARYDATACONTAINER_H
