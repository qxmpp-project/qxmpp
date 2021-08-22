// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef QXMPPOUTGOINGCOMPONENT_H
#define QXMPPOUTGOINGCOMPONENT_H

#include <memory>

#include <QXmppStream.h>

class QXmppOutgoingComponentPrivate;
class QXmppComponentConfig;
class QXmppIq;
class QXmppMessage;
class QXmppPresence;

class QXmppOutgoingComponent : public QXmppStream
{
    Q_OBJECT

public:
    QXmppOutgoingComponent(QObject *parent = nullptr);
    ~QXmppOutgoingComponent() override;

    QXmppComponentConfig &config();
    void connectToHost();
    bool isAuthenticated() const;

    /// This signal is emitted when an element is received.
    Q_SIGNAL void elementReceived(const QDomElement &element, bool &handled);

    /// This signal is emitted when a presence is received.
    Q_SIGNAL void presenceReceived(const QXmppPresence &);

    /// This signal is emitted when a message is received.
    Q_SIGNAL void messageReceived(const QXmppMessage &);

    /// This signal is emitted when an IQ response (type result or error) has
    /// been received that was not handled by elementReceived().
    Q_SIGNAL void iqReceived(const QXmppIq &);

protected:
    void handleStart() override;
    void handleStanza(const QDomElement &element) override;
    void handleStream(const QDomElement &element) override;

private:
    std::unique_ptr<QXmppOutgoingComponentPrivate> d;
};

#endif // QXMPPOUTGOINGCOMPONENT_H
