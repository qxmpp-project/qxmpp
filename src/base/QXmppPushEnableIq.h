/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Robert Märkisch
 *  Linus Jahn
 *  Jonah Brüchert
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

#pragma once

#include <QXmppIq.h>

class QXmppPushEnableIqPrivate;
class QXmppDataForm;

///
/// \brief This class represents an IQ to enable or disablepush notifications
/// on the user server.
///
/// \since QXmpp 1.3
///
class QXMPP_EXPORT QXmppPushEnableIq : public QXmppIq
{
public:
    QXmppPushEnableIq();
    QXmppPushEnableIq(const QXmppPushEnableIq &);
    ~QXmppPushEnableIq();
    QXmppPushEnableIq &operator=(const QXmppPushEnableIq &);

    ///
    /// \brief The Mode enum describes whether the IQ should enable or disable
    /// push notifications
    ///
    enum Mode : bool {
        Enable = true,
        Disable = false
    };

    QString jid() const;
    void setJid(const QString &jid);

    QString node() const;
    void setNode(const QString &node);

    void setMode(Mode mode);
    Mode mode();

    QXmppDataForm dataForm() const;
    void setDataForm(const QXmppDataForm &form);

    static bool isPushEnableIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppPushEnableIqPrivate> d;
};
