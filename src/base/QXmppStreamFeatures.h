/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#ifndef QXMPPSTREAMFEATURES_H
#define QXMPPSTREAMFEATURES_H

#include "QXmppStanza.h"

#include <QSharedDataPointer>

class QXmppStreamFeaturesPrivate;

///
/// \brief The QXmppStreamFeatures class represents the features returned by an
/// XMPP server or client.
///
class QXMPP_EXPORT QXmppStreamFeatures : public QXmppStanza
{
public:
    QXmppStreamFeatures();
    QXmppStreamFeatures(const QXmppStreamFeatures &);
    ~QXmppStreamFeatures();

    QXmppStreamFeatures &operator=(const QXmppStreamFeatures &);

    enum Mode {
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

    QStringList authMechanisms() const;
    void setAuthMechanisms(const QStringList &mechanisms);

    QStringList compressionMethods() const;
    void setCompressionMethods(const QStringList &methods);

    Mode tlsMode() const;
    void setTlsMode(Mode mode);

    Mode streamManagementMode() const;
    void setStreamManagementMode(Mode mode);

    Mode clientStateIndicationMode() const;
    void setClientStateIndicationMode(Mode mode);

    Mode registerMode() const;
    void setRegisterMode(const Mode &registerMode);

    bool preApprovedSubscriptionsSupported() const;
    void setPreApprovedSubscriptionsSupported(bool);

    bool rosterVersioningSupported() const;
    void setRosterVersioningSupported(bool);

    /// \cond
    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
    /// \endcond

    static bool isStreamFeatures(const QDomElement &element);

private:
    QSharedDataPointer<QXmppStreamFeaturesPrivate> d;
};

#endif
