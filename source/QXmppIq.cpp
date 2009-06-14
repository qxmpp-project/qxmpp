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


#include "utils.h"
#include "QXmppIq.h"

#include <QTextStream>

QXmppIq::QXmppIq(QXmppIq::Type type)
    : QXmppStanza(), m_type(type)
{
    generateAndSetNextId();
}

QXmppIq::QXmppIq(const QString& type)
    : QXmppStanza()
{
    generateAndSetNextId();
    setTypeFromStr(type);
}

QXmppIq::~QXmppIq()
{

}

QXmppIq::Type QXmppIq::getType() const
{
    return m_type;
}

void QXmppIq::setType(QXmppIq::Type type)
{
    m_type = type;
}

QByteArray QXmppIq::toXml() const
{
    QString data;
    QTextStream stream(&data);

    stream << "<iq";
    helperToXmlAddAttribute(stream, "id", getId());
    helperToXmlAddAttribute(stream, "to", getTo());
    helperToXmlAddAttribute(stream, "from", getFrom());
    if(getTypeStr().isEmpty())
        helperToXmlAddAttribute(stream, "type", "get");
    else
        helperToXmlAddAttribute(stream, "type", getTypeStr());
    stream << ">";
    stream << toXmlElementFromChild();
    stream << getError().toXml();
    stream << "</iq>";

    return data.toAscii();
}

QByteArray QXmppIq::toXmlElementFromChild() const
{
    return "";
}

QString QXmppIq::getTypeStr() const
{
    switch(getType())
    {
    case QXmppIq::Error:
        return "error";
    case QXmppIq::Get:
        return "get";
    case QXmppIq::Set:
        return "set";
    case QXmppIq::Result:
        return "result";
    default:
        qWarning("QXmppIq::getTypeStr() invalid type %d", (int)getType());
        return "";
    }
}

void QXmppIq::setTypeFromStr(const QString& str)
{
    if(str == "error")
    {
        setType(QXmppIq::Error);
        return;
    }
    else if(str == "get")
    {
        setType(QXmppIq::Get);
        return;
    }
    else if(str == "set")
    {
        setType(QXmppIq::Set);
        return;
    }
    else if(str == "result")
    {
        setType(QXmppIq::Result);
        return;
    }
    else
    {
        setType(static_cast<QXmppIq::Type>(-1));
        qWarning("QXmppIq::setTypeFromStr() invalid input string type: %s", qPrintable(str));
        return;
    }
}
