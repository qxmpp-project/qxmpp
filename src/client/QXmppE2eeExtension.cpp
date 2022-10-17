// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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

///
/// \typedef QXmppE2eeExtension::MessageEncryptResult
///
/// Contains the XML serialized message stanza with encrypted contents or a
/// QXmpp::SendError in case the message couldn't be encrypted.
///

///
/// \typedef QXmppE2eeExtension::MessageDecryptResult
///
/// Contains the decrypted QXmppMessage, NotEncrypted or an QXmppError.
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
/// If the message cannot be encrypted for whatever reason, you can either
/// serialize the message unencrypted and return that or return a SendError with
/// an error message.
///

///
/// \fn QXmppE2eeExtension::decryptMessage
///
/// Decrypts a QXmppMessage and returns the decrypted QXmppMessage. In case the message was not
/// encrypted, QXmppE2eeExtension::NotEncrypted should be returned.
///

///
/// \fn QXmppE2eeExtension::encryptIq
///
/// Encrypts a QXmppIq and returns the serialized XML stanza with encrypted
/// contents via QFuture.
///
/// If the IQ cannot be encrypted for whatever reason, you can either serialize
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

///
/// \fn QXmppE2eeExtension::isEncrypted(const QDomElement &)
///
/// Returns whether the DOM element of an IQ or message stanza is encrypted with this encryption.
///

///
/// \fn QXmppE2eeExtension::isEncrypted(const QXmppMessage &)
///
/// Returns whether the message is encrypted with this encryption.
///
