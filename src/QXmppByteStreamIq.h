/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
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

#ifndef QXMPPBYTESTREAMIQ_H
#define QXMPPBYTESTREAMIQ_H

#include "QXmppIq.h"

#include <QHostAddress>

class QDomElement;
class QXmlStreamWriter;

class QXmppByteStreamIq : public QXmppIq
{
public:
    enum Mode {
        None = 0,
        Tcp,
        Udp,
    };

    class StreamHost
    {
    public:
        QString jid() const;
        void setJid(const QString &jid);

        QHostAddress host() const;
        void setHost(const QHostAddress &host);

        quint16 port() const;
        void setPort(quint16 port);

        QString zeroconf() const;
        void setZeroconf(const QString &zeroconf);

    private:
        QHostAddress m_host;
        QString m_jid;
        quint16 m_port;
        QString m_zeroconf;
    };

    QXmppByteStreamIq::Mode mode() const;
    void setMode(QXmppByteStreamIq::Mode mode);

    QString sid() const;
    void setSid(const QString &sid);

    QString activate() const;
    void setActivate(const QString &activate);

    QList<QXmppByteStreamIq::StreamHost> streamHosts() const;
    void setStreamHosts(const QList<QXmppByteStreamIq::StreamHost> &streamHosts);

    QString streamHostUsed() const;
    void setStreamHostUsed(const QString &jid);

    static bool isByteStreamIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    Mode m_mode;
    QString m_sid;

    QString m_activate;
    QList<StreamHost> m_streamHosts;
    QString m_streamHostUsed;
};

#endif
