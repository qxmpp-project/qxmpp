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

#ifndef QXMPPIBBIQ_H
#define QXMPPIBBIQ_H

#include "QXmppIq.h"

class QXmppIbbOpenIq: public QXmppIq
{
public:
    QXmppIbbOpenIq();

    long blockSize() const;
    void setBlockSize( long block_size );

    QString sid() const;
    void setSid( const QString &sid );

    static bool isIbbOpenIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    long m_block_size;
    QString m_sid;
};

class QXmppIbbCloseIq: public QXmppIq
{
public:
    QXmppIbbCloseIq();

    QString sid() const;
    void setSid( const QString &sid );

    static bool isIbbCloseIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_sid;
};

class QXMPP_EXPORT QXmppIbbDataIq : public QXmppIq
{
public:
    QXmppIbbDataIq();

    quint16 sequence() const;
    void setSequence( quint16 seq );

    QString sid() const;
    void setSid( const QString &sid );

    QByteArray payload() const;
    void setPayload( const QByteArray &data );

    static bool isIbbDataIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    quint16 m_seq;
    QString m_sid;
    QByteArray m_payload;
};

#endif // QXMPPIBBIQS_H
