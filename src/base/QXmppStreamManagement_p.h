/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Niels Ole Salscheider
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

#ifndef QXMPPSTREAMMANAGEMENT_P_H
#define QXMPPSTREAMMANAGEMENT_P_H

#include "QXmppGlobal.h"
#include "QXmppStanza.h"

#include <QDomDocument>
#include <QXmlStreamWriter>

//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  It exists for the convenience
// of the QXmppIncomingClient and QXmppOutgoingClient classes.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementEnable
{
public:
    QXmppStreamManagementEnable(const bool resume = false, const unsigned max = 0);

    bool resume() const;
    void setResume(const bool resume);

    unsigned max() const;
    void setMax(const unsigned max);

    static bool isStreamManagementEnable(const QDomElement &element);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    bool m_resume;
    unsigned m_max;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementEnabled
{
public:
    QXmppStreamManagementEnabled(const bool resume = false, const QString id = QString(),
                                 const unsigned max = 0, const QString location = QString());

    bool resume() const;
    void setResume(const bool resume);

    QString id() const;
    void setId(const QString id);

    unsigned max() const;
    void setMax(const unsigned max);

    QString location() const;
    void setLocation(const QString location);

    static bool isStreamManagementEnabled(const QDomElement &element);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    bool m_resume;
    QString m_id;
    unsigned m_max;
    QString m_location;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementResume
{
public:
    QXmppStreamManagementResume(const unsigned h = 0, const QString& previd = QString());

    unsigned h() const;
    void setH(const unsigned h);

    QString prevId() const;
    void setPrevId(const QString& id);

    static bool isStreamManagementResume(const QDomElement &element);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    unsigned m_h;
    QString m_previd;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementResumed
{
public:
    QXmppStreamManagementResumed(const unsigned h = 0, const QString& previd = QString());

    unsigned h() const;
    void setH(const unsigned h);

    QString prevId() const;
    void setPrevId(const QString& id);

    static bool isStreamManagementResumed(const QDomElement &element);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    unsigned m_h;
    QString m_previd;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementFailed
{
public:
    QXmppStreamManagementFailed(const QXmppStanza::Error::Condition error = QXmppStanza::Error::UndefinedCondition);

    QXmppStanza::Error::Condition error() const;
    void setError(const QXmppStanza::Error::Condition error);

    static bool isStreamManagementFailed(const QDomElement &element);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QXmppStanza::Error::Condition m_error;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementAck
{
public:
    QXmppStreamManagementAck(const unsigned seqNo = 0);

    unsigned seqNo() const;
    void setSeqNo(const unsigned seqNo);

    static bool isStreamManagementAck(const QDomElement &element);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    unsigned m_seqNo;
};

class QXMPP_AUTOTEST_EXPORT QXmppStreamManagementReq
{
public:
    static bool isStreamManagementReq(const QDomElement &element);

    /// \cond
    static void toXml(QXmlStreamWriter *writer);
    /// \endcond
};

#endif
