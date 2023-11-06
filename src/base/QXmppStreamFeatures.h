// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppStanza.h"

#include <QSharedDataPointer>

class QXmppStreamFeaturesPrivate;

///
/// \brief The QXmppStreamFeatures class represents the features returned by an
/// XMPP server or client.
///
class QXMPP_EXPORT QXmppStreamFeatures : public QXmppNonza
{
public:
    QXmppStreamFeatures();
    QXmppStreamFeatures(const QXmppStreamFeatures &);
    QXmppStreamFeatures(QXmppStreamFeatures &&);
    ~QXmppStreamFeatures();

    QXmppStreamFeatures &operator=(const QXmppStreamFeatures &);
    QXmppStreamFeatures &operator=(QXmppStreamFeatures &&);

    /// Mode of a feature
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
    void setRegisterMode(const Mode &mode);

    bool preApprovedSubscriptionsSupported() const;
    void setPreApprovedSubscriptionsSupported(bool);

    bool rosterVersioningSupported() const;
    void setRosterVersioningSupported(bool);

    /// \cond
    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

    static bool isStreamFeatures(const QDomElement &element);
    /// \endcond

private:
    QSharedDataPointer<QXmppStreamFeaturesPrivate> d;
};
