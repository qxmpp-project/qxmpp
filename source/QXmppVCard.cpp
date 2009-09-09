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


#include "QXmppVCard.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

#include <QTextStream>
#include <QImage>

QXmppVCard::QXmppVCard(const QString& jid) : QXmppIq(QXmppIq::Get)
{
    // for self jid should be empty
    setTo(jid);
}

QString QXmppVCard::getFullName() const
{
    return m_fullName;
}

void QXmppVCard::setFullName(const QString& str)
{
    m_fullName = str;
}

const QByteArray& QXmppVCard::getPhoto() const
{
    return m_photo;
}

void QXmppVCard::setPhoto(const QByteArray& photo)
{
    m_photo = photo;
}

void QXmppVCard::parse(const QDomElement& nodeRecv)
{
    QString id = nodeRecv.attribute("id");
    QString to = nodeRecv.attribute("to");
    QString from = nodeRecv.attribute("from");
    QString type = nodeRecv.attribute("type");
    setTypeFromStr(type);
    setId(id);
    setTo(to);
    setFrom(from);

    // vCard
    setFullName(nodeRecv.firstChildElement("vCard").
                firstChildElement("FN").text());
    QByteArray base64data = nodeRecv.firstChildElement("vCard").
                            firstChildElement("PHOTO").
                            firstChildElement("BINVAL").text().toAscii();
    setPhoto(QByteArray::fromBase64(base64data));
}

QByteArray QXmppVCard::toXmlElementFromChild() const
{
    QString data;
    QTextStream stream(&data);

    stream << "<vCard";
    helperToXmlAddAttribute(stream, "xmlns", ns_vcard);
    stream << ">";
    helperToXmlAddElement(stream, "FN", getFullName());

    if(!getPhoto().isEmpty())
    {
        stream << "<PHOTO";
        helperToXmlAddElement(stream, "TYPE", getImageType(getPhoto()));
        helperToXmlAddElement(stream, "BINVAL", getPhoto());
        stream << "</PHOTO>";
    }

    stream << "</vCard>";

    return data.toAscii();
}

QImage QXmppVCard::getPhotoAsImage() const
{
    return getImageFromByteArray(getPhoto());
}
