/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
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

#ifndef QXMPPVERSIONIQ_H
#define QXMPPVERSIONIQ_H

#include "QXmppIq.h"

/// \brief The QXmppVersionIq class represents an IQ for conveying a software
/// version as defined by XEP-0092: Software Version.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppVersionIq : public QXmppIq
{
public:
    QString name() const;
    void setName(const QString &name);

    QString os() const;
    void setOs(const QString &os);

    QString version() const;
    void setVersion(const QString &version);

    /// \cond
    static bool isVersionIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_name;
    QString m_os;
    QString m_version;
};

#endif
