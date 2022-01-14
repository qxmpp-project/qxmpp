// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPIBBIQ_H
#define QXMPPIBBIQ_H

#include "QXmppIq.h"

class QXmppIbbOpenIq : public QXmppIq
{
public:
    QXmppIbbOpenIq();

    long blockSize() const;
    void setBlockSize(long block_size);

    QString sid() const;
    void setSid(const QString &sid);

    /// \cond
    static bool isIbbOpenIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    long m_block_size;
    QString m_sid;
};

class QXmppIbbCloseIq : public QXmppIq
{
public:
    QXmppIbbCloseIq();

    QString sid() const;
    void setSid(const QString &sid);

    /// \cond
    static bool isIbbCloseIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QString m_sid;
};

class QXMPP_EXPORT QXmppIbbDataIq : public QXmppIq
{
public:
    QXmppIbbDataIq();

    quint16 sequence() const;
    void setSequence(quint16 seq);

    QString sid() const;
    void setSid(const QString &sid);

    QByteArray payload() const;
    void setPayload(const QByteArray &data);

    /// \cond
    static bool isIbbDataIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    quint16 m_seq;
    QString m_sid;
    QByteArray m_payload;
};

#endif  // QXMPPIBBIQS_H
