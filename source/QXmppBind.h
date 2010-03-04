/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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
    QXmppBind(QXmppIq::Type type);
    QXmppBind(const QString& type);
    ~QXmppBind();
    
    QString jid() const;
    QString resource() const;
    void setJid(const QString&);
    void setResource(const QString&);

// deprecated accessors, use the form without "get" instead
// obsolete start
    QString Q_DECL_DEPRECATED getJid() const;
    QString Q_DECL_DEPRECATED getResource() const;
// obsolete end

private:
    QString m_jid;
    QString m_resource;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
};

#endif // QXMPPBIND_H
