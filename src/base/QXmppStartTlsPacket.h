/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
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

#ifndef QXMPPSTARTTLSPACKET_H
#define QXMPPSTARTTLSPACKET_H

#include "QXmppStanza.h"

///
/// \brief The QXmppStartTlsPacket represents packets used for initiating
/// STARTTLS negotiation when connecting.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppStartTlsPacket : public QXmppStanza
{
public:
    /// The type of the STARTTLS packet.
    enum Type {
        StartTls,  ///< Used by the client to initiate STARTTLS.
        Proceed,   ///< Used by the server to accept STARTTLS.
        Failure    ///< Used by the server to reject STARTTLS.
    };

    QXmppStartTlsPacket(Type type = StartTls);
    ~QXmppStartTlsPacket() override;

    Type type() const;
    void setType(Type type);

    /// \cond
    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
    /// \endcond

    static bool isStartTlsPacket(const QDomElement &element);
    static bool isStartTlsPacket(const QDomElement &element, Type type);

private:
    Type m_type;
};

Q_DECLARE_METATYPE(QXmppStartTlsPacket::Type);

#endif  // QXMPPSTARTTLSPACKET_H
