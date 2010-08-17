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


#ifndef QXMPPBIND_H
#define QXMPPBIND_H

#include "QXmppIq.h"

class QXmppBind : public QXmppIq
{
public:
    QXmppBind();
    QXmppBind(QXmppIq::Type type);
    
    QString jid() const;
    QString resource() const;

    void setJid(const QString&);
    void setResource(const QString&);

    static bool isBind(const QDomElement &element);

    // deprecated accessors, use the form without "get" instead
    /// \cond
    QString Q_DECL_DEPRECATED getJid() const;
    QString Q_DECL_DEPRECATED getResource() const;
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
