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

#ifndef QXMPPBITSOFBINARYIQ_H
#define QXMPPBITSOFBINARYIQ_H

#include "QXmppBitsOfBinaryData.h"
#include "QXmppIq.h"

/// \class QXmppBitsOfBinaryIq represents a \xep{0231}: Bits of Binary IQ to
/// request and transmit Bits of Binary data elements.
///
/// \since QXmpp 1.2

class QXMPP_EXPORT QXmppBitsOfBinaryIq : public QXmppIq, public QXmppBitsOfBinaryData
{
public:
    QXmppBitsOfBinaryIq();
    ~QXmppBitsOfBinaryIq();

    static bool isBitsOfBinaryIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond
};

#endif  // QXMPPBITSOFBINARYIQ_H
