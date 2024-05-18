// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTARTTLSPACKET_H
#define QXMPPSTARTTLSPACKET_H

#include "QXmppStanza.h"

///
/// \brief The QXmppStartTlsPacket represents packets used for initiating
/// STARTTLS negotiation when connecting.
///
/// \ingroup Stanzas
///
/// \deprecated STARTTLS packets will be removed from the public API.
///
class QXMPP_EXPORT QXmppStartTlsPacket : public QXmppNonza
{
public:
    /// The type of the STARTTLS packet.
    enum Type {
        StartTls,  ///< Used by the client to initiate STARTTLS.
        Proceed,   ///< Used by the server to accept STARTTLS.
        Failure,   ///< Used by the server to reject STARTTLS.
        Invalid,   ///< Invalid type
    };

    [[deprecated]] QXmppStartTlsPacket(Type type = StartTls);
    ~QXmppStartTlsPacket() override;

    Type type() const;
    void setType(Type type);

    /// \cond
    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
    /// \endcond

    [[deprecated]] static bool isStartTlsPacket(const QDomElement &element);
    [[deprecated]] static bool isStartTlsPacket(const QDomElement &element, Type type);

private:
    Type m_type;
};

Q_DECLARE_METATYPE(QXmppStartTlsPacket::Type);

#endif  // QXMPPSTARTTLSPACKET_H
