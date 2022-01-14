// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBYTESTREAMIQ_H
#define QXMPPBYTESTREAMIQ_H

#include "QXmppIq.h"

#include <QHostAddress>

class QXMPP_EXPORT QXmppByteStreamIq : public QXmppIq
{
public:
    enum Mode {
        None = 0,
        Tcp,
        Udp
    };

    class QXMPP_EXPORT StreamHost
    {
    public:
        QString jid() const;
        void setJid(const QString &jid);

        QString host() const;
        void setHost(const QString &host);

        quint16 port() const;
        void setPort(quint16 port);

        QString zeroconf() const;
        void setZeroconf(const QString &zeroconf);

    private:
        QString m_host;
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
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    Mode m_mode;
    QString m_sid;

    QString m_activate;
    QList<StreamHost> m_streamHosts;
    QString m_streamHostUsed;
};

#endif
