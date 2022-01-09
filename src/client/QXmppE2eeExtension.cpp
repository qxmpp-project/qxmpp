/*
 * Copyright (C) 2008-2022 The QXmpp developers
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

#include "QXmppE2eeExtension.h"

///
/// \class QXmppE2eeExtension
///
/// Abstract client extension for end-to-end-encryption protocols.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

///
/// \struct QXmppE2eeExtension::NotEncrypted
///
/// Indicates that the input was not encrypted and so nothing could be decrypted.
///
/// \since QXmpp 1.5
///

///
/// \typedef QXmppE2eeExtension::EncryptMessageResult
///
/// Contains the XML serialized message stanza with encrypted contents or a
/// QXmpp::SendError in case the message couldn't be encrypted.
///

///
/// \typedef QXmppE2eeExtension::IqEncryptResult
///
/// Contains the XML serialized IQ stanza with encrypted contents or a
/// QXmpp::SendError in case the IQ couldn't be encrypted.
///

///
/// \typedef QXmppE2eeExtension::IqDecryptResult
///
/// Contains a deserialized IQ stanza in form of a DOM element with decrypted
/// contents or a QXmpp::SendError in case the IQ couldn't be decrypted.
///

///
/// \fn QXmppE2eeExtension::encryptMessage
///
/// Encrypts a QXmppMessage and returns the serialized XML stanza with encrypted
/// contents via QFuture.
///
/// If the message cannot be encrypted for whatever reason you can either
/// serialize the message unencrypted and return that or return a SendError with
/// an error message.
///

///
/// \fn QXmppE2eeExtension::encryptIq
///
/// Encrypts a QXmppIq and returns the serialized XML stanza with encrypted
/// contents via QFuture.
///
/// If the IQ cannot be encrypted for whatever reason you can either serialize
/// the IQ unencrypted and return that or return a SendError with an error
/// message.
///

///
/// \fn QXmppE2eeExtension::decryptIq
///
/// Decrypts an IQ from a DOM element and returns a fully decrypted IQ as a DOM
/// element via QFuture. If the input was not encrypted,
/// QXmppE2eeExtension::NotEncrypted should be returned.
///
