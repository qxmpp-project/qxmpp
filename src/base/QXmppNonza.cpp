/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#include "QXmppNonza.h"

///
/// \class QXmppNonza
///
/// Abstract class for content that can be parsed from DOM and serialized to
/// XML.
///
/// If you want to implement a XMPP stanza (IQ, message or presence) then you
/// should use QXmppStanza. Directly inheriting from this class is useful for
/// other elements like stream management elements in the XML stream.
///
/// \since QXmpp 1.5
///

///
/// \fn QXmppNonza::isXmppStanza
///
/// Indicates if the QXmppStanza is a stanza in the XMPP sense (i. e. a message,
/// iq or presence)
///
/// \since QXmpp 1.0 (moved from QXmppStanza in 1.5)
///

///
/// \fn QXmppNonza::parse
///
/// Parses the object from a DOM element.
///

///
/// \fn QXmppNonza::toXml
///
/// Serializes the object to XML using a QXmlStreamWriter.
///
