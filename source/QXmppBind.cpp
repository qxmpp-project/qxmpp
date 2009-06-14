/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
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


#include "QXmppBind.h"
#include "utils.h"
#include "QXmppConstants.h"

#include <QTextStream>

QXmppBind::QXmppBind(QXmppIq::Type type)
    : QXmppIq(type)
{
}
QXmppBind::QXmppBind(const QString& type)
    : QXmppIq(type)
{
}

QXmppBind::~QXmppBind()
{
}

QString QXmppBind::getJid() const
{
    return m_jid;
}

QString QXmppBind::getResource() const
{
    return m_resource;
}

void QXmppBind::setJid(const QString& str)
{
    m_jid = str;
}

void QXmppBind::setResource(const QString& str)
{
    m_resource = str;
}

QByteArray QXmppBind::toXmlElementFromChild() const
{
    QString data;
    QTextStream stream(&data);

    stream << "<bind";
    helperToXmlAddAttribute(stream, "xmlns", ns_bind);
    stream << ">";
    helperToXmlAddElement(stream, "jid", getJid());
    helperToXmlAddElement(stream, "resource", getResource());
    stream << "</bind>";

    return data.toAscii();
}

