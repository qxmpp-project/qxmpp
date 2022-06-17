// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppSendStanzaParams.h"

#include <QVector>

///
/// \class QXmppSendStanzaParams
///
/// Contains additional parameters for sending stanzas.
///
/// \since QXmpp 1.5
///

class QXmppSendStanzaParamsPrivate : public QSharedData
{
public:
    QVector<QString> encryptionJids;
};

QXmppSendStanzaParams::QXmppSendStanzaParams()
    : d(new QXmppSendStanzaParamsPrivate)
{
}

/// Copy-constructor
QXmppSendStanzaParams::QXmppSendStanzaParams(const QXmppSendStanzaParams &other) = default;
/// Move-constructor
QXmppSendStanzaParams::QXmppSendStanzaParams(QXmppSendStanzaParams &&) = default;
QXmppSendStanzaParams::~QXmppSendStanzaParams() = default;
/// Assignment operator
QXmppSendStanzaParams &QXmppSendStanzaParams::operator=(const QXmppSendStanzaParams &) = default;
/// Move-assignment operator
QXmppSendStanzaParams &QXmppSendStanzaParams::operator=(QXmppSendStanzaParams &&) = default;

///
/// Returns the list of JIDs that the stanza should be encrypted for.
///
/// If this is empty, the stanza should be encrypted for the recipient.
/// This option is useful for groupchats.
///
QVector<QString> QXmppSendStanzaParams::encryptionJids() const
{
    return d->encryptionJids;
}

///
/// Sets the list of JIDs that the stanza should be encrypted for.
///
/// If this is empty, the stanza should be encrypted for the recipient.
/// This option is useful for groupchats.
///
void QXmppSendStanzaParams::setEncryptionJids(QVector<QString> encryptionJids)
{
    d->encryptionJids = std::move(encryptionJids);
}
