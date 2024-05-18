// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBITSOFBINARYIQ_H
#define QXMPPBITSOFBINARYIQ_H

#include "QXmppBitsOfBinaryData.h"
#include "QXmppIq.h"

class QXMPP_EXPORT QXmppBitsOfBinaryIq : public QXmppIq, public QXmppBitsOfBinaryData
{
public:
    QXmppBitsOfBinaryIq();
    ~QXmppBitsOfBinaryIq() override;

    static bool isBitsOfBinaryIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond
};

#endif  // QXMPPBITSOFBINARYIQ_H
