// SPDX-FileCopyrightText: 2011 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppIq.h"

/// \brief The QXmppSessionIq class represents an IQ used for session
/// establishment as defined by RFC 3921.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppSessionIq : public QXmppIq
{
public:
    /// \cond
    static bool isSessionIq(const QDomElement &element);
    /// \endcond

private:
    /// \cond
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond
};
