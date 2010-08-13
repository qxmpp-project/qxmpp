/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
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

#ifndef QXMPPSTREAMFEATURES_H
#define QXMPPSTREAMFEATURES_H

#include "QXmppConfiguration.h"
#include "QXmppStanza.h"

class QXmppStreamFeatures : public QXmppStanza
{
public:
    QXmppStreamFeatures();

    bool isBindAvailable() const;
    void setBindAvailable(bool available);

    bool isSessionAvailable() const;
    void setSessionAvailable(bool available);

    QList<QXmppConfiguration::SASLAuthMechanism> authMechanisms() const;
    void setAuthMechanisms(QList<QXmppConfiguration::SASLAuthMechanism> &mecanisms);

    QXmppConfiguration::StreamSecurityMode securityMode() const;
    void setSecurityMode(QXmppConfiguration::StreamSecurityMode mode);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    bool m_bindAvailable;
    bool m_sessionAvailable;
    QList<QXmppConfiguration::SASLAuthMechanism> m_authMechanisms;
    QXmppConfiguration::StreamSecurityMode m_securityMode;
};

#endif
