/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Olivier Goffart
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

#include "QXmppStreamInitiationIq.h"

#include "si.h"
#include "tests.h"

void tst_QXmppStreamInitiationIq::testOffer()
{
    QByteArray xml(
        "<iq id=\"offer1\" to=\"receiver@jabber.org/resource\" type=\"set\">"
          "<si xmlns=\"http://jabber.org/protocol/si\" id=\"a0\" mime-type=\"text/plain\" profile=\"http://jabber.org/protocol/si/profile/file-transfer\">"
            "<file xmlns=\"http://jabber.org/protocol/si/profile/file-transfer\" name=\"test.txt\" size=\"1022\"/>"
            "<feature xmlns=\"http://jabber.org/protocol/feature-neg\">"
              "<x xmlns=\"jabber:x:data\" type=\"form\">"
                "<field type=\"list-single\" var=\"stream-method\">"
                  "<option><value>http://jabber.org/protocol/bytestreams</value></option>"
                  "<option><value>http://jabber.org/protocol/ibb</value></option>"
                "</field>"
              "</x>"
            "</feature>"
          "</si>"
        "</iq>");

    QXmppStreamInitiationIq iq;
    parsePacket(iq, xml);
    serializePacket(iq, xml);
}
