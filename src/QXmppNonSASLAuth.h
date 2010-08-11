/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#ifndef QXmppNonSASLAuth_H
#define QXmppNonSASLAuth_H

#include "QXmppIq.h"

class QXmppNonSASLAuthTypesRequestIq : public QXmppIq
{
public:
    QXmppNonSASLAuthTypesRequestIq();
    void setUsername( const QString &username );
    virtual void toXmlElementFromChild(QXmlStreamWriter *writer) const;
private:
    QString m_username;
};

class QXmppNonSASLAuthIq : public QXmppIq
{
public:
    QXmppNonSASLAuthIq();
    virtual void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void setUsername( const QString &username );
    void setPassword( const QString &password );
    void setResource( const QString &resource );
    void setStreamId( const QString &sid );
    void setUsePlainText( bool useplaintext );

private:
    QString m_username;
    QString m_password;
    QString m_resource;
    QString m_sid;
    bool m_useplaintext;
};

#endif // QXmppNonSASLAuth_H
