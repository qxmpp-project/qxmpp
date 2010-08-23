/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

class QXmppBindIq : public QXmppIq
{
public:
    QXmppBindIq();
    QXmppBindIq(QXmppIq::Type type);
    
    QString jid() const;
    QString resource() const;

    void setJid(const QString&);
    void setResource(const QString&);

    static bool isBindIq(const QDomElement &element);

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
