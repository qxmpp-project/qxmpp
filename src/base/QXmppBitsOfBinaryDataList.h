/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#ifndef QXMPPBITSOFBINARYDATACONTAINER_H
#define QXMPPBITSOFBINARYDATACONTAINER_H

#include "QXmppBitsOfBinaryData.h"

#include <QVector>

class QDomElement;
class QXmlStreamWriter;

/// \class QXmppBitsOfBinaryDataList represents a list of data elements from
/// \xep{0231}: Bits of Binary.
///
/// \since QXmpp 1.2

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
