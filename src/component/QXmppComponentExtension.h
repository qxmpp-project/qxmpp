// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef QXMPPCOMPONENTEXTENSION_H
#define QXMPPCOMPONENTEXTENSION_H

#include <QXmppLogger.h>

class QDomElement;
class QXmppComponent;

class QXmppComponentExtension : public QXmppLoggable
{
public:
    QXmppComponentExtension();
    ~QXmppComponentExtension() override;

    /// \brief You need to implement this method to process incoming XMPP
    /// stanzas.
    ///
    /// You should return true if the stanza was handled and no further
    /// processing should occur, or false to let other extensions process
    /// the stanza.
    virtual bool handleStanza(const QDomElement &stanza) = 0;

protected:
    QXmppComponent *component();
    virtual void setComponent(QXmppComponent *component);

private:
    QXmppComponent *m_component;

    friend class QXmppComponent;
};

#endif // QXMPPCOMPONENTEXTENSION_H
