/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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


#ifndef QXMPPBINDIQ_H
#define QXMPPBINDIQ_H

#include "QXmppIq.h"

/// \brief The QXmppBindIq class represents an IQ used for resource
/// binding as defined by RFC 3921.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppBindIq : public QXmppIq
{
public:
    QString jid() const;
    void setJid(const QString&);

    QString resource() const;
    void setResource(const QString&);

    /// \cond
    static bool isBindIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_jid;
    QString m_resource;
};

#endif // QXMPPBIND_H
