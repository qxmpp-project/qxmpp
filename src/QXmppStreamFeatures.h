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

#ifndef QXMPPSTREAMFEATURES_H
#define QXMPPSTREAMFEATURES_H

#include "QXmppConfiguration.h"
#include "QXmppStanza.h"

class QXmppStreamFeatures : public QXmppStanza
{
public:
    QXmppStreamFeatures();

    enum Mode
    {
        Disabled = 0,
        Enabled,
        Required
    };

    Mode bindMode() const;
    void setBindMode(Mode mode);

    Mode sessionMode() const;
    void setSessionMode(Mode mode);

    Mode nonSaslAuthMode() const;
    void setNonSaslAuthMode(Mode mode);

    QList<QXmppConfiguration::SASLAuthMechanism> authMechanisms() const;
    void setAuthMechanisms(QList<QXmppConfiguration::SASLAuthMechanism> &mecanisms);

    QList<QXmppConfiguration::CompressionMethod> compressionMethods() const;
    void setCompressionMethods(QList<QXmppConfiguration::CompressionMethod> &methods);

    Mode tlsMode() const;
    void setTlsMode(Mode mode);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isStreamFeatures(const QDomElement &element);

private:
    Mode m_bindMode;
    Mode m_sessionMode;
    Mode m_nonSaslAuthMode;
    Mode m_tlsMode;
    QList<QXmppConfiguration::SASLAuthMechanism> m_authMechanisms;
    QList<QXmppConfiguration::CompressionMethod> m_compressionMethods;
};

#endif
