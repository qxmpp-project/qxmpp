// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef QXMPPCOMPONENT_H
#define QXMPPCOMPONENT_H

#include <memory>
#include <QVector>
#include <QXmppLogger.h>

class QXmppComponentPrivate;
class QXmppComponentConfig;
class QXmppComponentExtension;
class QDomElement;
class QXmppStanza;
class QXmppPresence;
class QXmppMessage;
class QXmppIq;

class QXmppComponent : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppComponent(QObject *parent = nullptr);
    ~QXmppComponent();

    bool addExtension(QXmppComponentExtension *extension);
    bool insertExtension(int index, QXmppComponentExtension *extension);
    bool removeExtension(QXmppComponentExtension *extension);
    auto extensions() -> QVector<QXmppComponentExtension*>;
    template<typename T>
    auto findExtension() -> T *;

    auto logger() const -> QXmppLogger*;
    void setLogger(QXmppLogger* logger);
    Q_SIGNAL void loggerChanged();

    QXmppComponentConfig &configuration();

    Q_SLOT void connectToServer(const QXmppComponentConfig &config);
    Q_SLOT bool sendPacket(const QXmppStanza &);

    bool isConnected();
    Q_SIGNAL void connected();
    Q_SIGNAL void disconnected();

    /// This signal is emitted when a presence is received.
    Q_SIGNAL void presenceReceived(const QXmppPresence &);

    /// This signal is emitted when a message is received.
    Q_SIGNAL void messageReceived(const QXmppMessage &);

    /// This signal is emitted when an IQ response (type result or error) has
    /// been received that was not handled by elementReceived().
    Q_SIGNAL void iqReceived(const QXmppIq &);

private:
    void onElementReceived(const QDomElement &element, bool &handled);

    std::unique_ptr<QXmppComponentPrivate> d;
};

///
/// \brief Returns the extension which can be cast into type T*, or 0
/// if there is no such extension.
///
/// Usage example:
/// \code
/// QXmppDiscoveryManager* ext = client->findExtension<QXmppDiscoveryManager>();
/// if(ext)
/// {
///     //extension found, do stuff...
/// }
/// \endcode
///
template<typename T>
T *QXmppComponent::findExtension()
{
    const auto list = extensions();
    for (auto ext : list) {
        if (auto *extension = qobject_cast<T *>(ext)) {
            return extension;
        }
    }
    return nullptr;
}

#endif // QXMPPCOMPONENT_H
