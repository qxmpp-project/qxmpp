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
#include "QXmppUtils.h"
#include "QXmppConstants.h"

#include <QTextStream>
#include <QXmlStreamWriter>

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

void QXmppBind::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    QString data;
    QTextStream stream(&data);

    writer->writeStartElement("bind");
    helperToXmlAddAttribute(writer, "xmlns", ns_bind);
    helperToXmlAddTextElement(writer, "jid", getJid() );
    helperToXmlAddTextElement(writer, "resource", getResource());
    writer->writeEndElement();
}

