// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "QXmppComponent.h"
#include "QXmppOutgoingComponent.h"
#include "QXmppComponentConfig.h"
#include "QXmppComponentExtension.h"

class QXmppComponentPrivate
{
public:
    QXmppComponentPrivate(QXmppComponent *parent);

    QXmppOutgoingComponent *component;
    QVector<QXmppComponentExtension *> extensions;
    QXmppLogger *logger = nullptr;
};

QXmppComponentPrivate::QXmppComponentPrivate(QXmppComponent *parent)
    : component(new QXmppOutgoingComponent(parent))
{
}

QXmppComponent::QXmppComponent(QObject *parent)
    : QXmppLoggable(parent),
      d(new QXmppComponentPrivate(this))
{
    connect(d->component, &QXmppOutgoingComponent::connected, this, &QXmppComponent::connected);
    connect(d->component, &QXmppOutgoingComponent::disconnected, this, &QXmppComponent::disconnected);

    connect(d->component, &QXmppOutgoingComponent::iqReceived, this, &QXmppComponent::iqReceived);
    connect(d->component, &QXmppOutgoingComponent::messageReceived, this, &QXmppComponent::messageReceived);
    connect(d->component, &QXmppOutgoingComponent::presenceReceived, this, &QXmppComponent::presenceReceived);
    connect(d->component, &QXmppOutgoingComponent::elementReceived, this, &QXmppComponent::onElementReceived);

    // logging
    setLogger(QXmppLogger::getLogger());
}

QXmppComponent::~QXmppComponent()
{
}

///
/// Registers a new \a extension with the component.
///
/// \param extension
///
bool QXmppComponent::addExtension(QXmppComponentExtension* extension)
{
    return insertExtension(d->extensions.size(), extension);
}

///
/// Registers a new \a extension with the component at the given \a index.
///
/// \param index
/// \param extension
///
bool QXmppComponent::insertExtension(int index, QXmppComponentExtension *extension)
{
    if (d->extensions.contains(extension)) {
        qWarning("Cannot add extension, it has already been added");
        return false;
    }

    extension->setParent(this);
    extension->setComponent(this);
    d->extensions.insert(index, extension);
    return true;
}

///
/// Unregisters the given extension from the component. If the extension
/// is found, it will be destroyed.
///
/// \param extension
///
bool QXmppComponent::removeExtension(QXmppComponentExtension *extension)
{
    if (d->extensions.contains(extension)) {
        d->extensions.removeAll(extension);
        extension->deleteLater();
        return true;
    } else {
        warning("Cannot remove extension, it was never added");
        return false;
    }
}

///
/// Returns a list containing all the component's extensions.
///
auto QXmppComponent::extensions() -> QVector<QXmppComponentExtension*>
{
    return d->extensions;
}

///
/// Returns the QXmppLogger associated with the current QXmppComponent.
///
auto QXmppComponent::logger() const -> QXmppLogger*
{
    return d->logger;
}

///
/// Sets the QXmppLogger associated with the current QXmppComponent.
///
void QXmppComponent::setLogger(QXmppLogger *logger)
{
    if (logger != d->logger) {
        if (d->logger) {
            disconnect(this, &QXmppLoggable::logMessage,
                       d->logger, &QXmppLogger::log);
            disconnect(this, &QXmppLoggable::setGauge,
                       d->logger, &QXmppLogger::setGauge);
            disconnect(this, &QXmppLoggable::updateCounter,
                       d->logger, &QXmppLogger::updateCounter);
        }

        d->logger = logger;
        if (d->logger) {
            connect(this, &QXmppLoggable::logMessage,
                    d->logger, &QXmppLogger::log);
            connect(this, &QXmppLoggable::setGauge,
                    d->logger, &QXmppLogger::setGauge);
            connect(this, &QXmppLoggable::updateCounter,
                    d->logger, &QXmppLogger::updateCounter);
        }

        emit loggerChanged();
    }
}

QXmppComponentConfig &QXmppComponent::configuration()
{
    return d->component->config();
}

void QXmppComponent::connectToServer(const QXmppComponentConfig &config)
{
    d->component->config() = config;
    d->component->connectToHost();
}

bool QXmppComponent::sendPacket(const QXmppStanza &packet)
{
    return d->component->sendPacket(packet);
}

bool QXmppComponent::isConnected()
{
    return d->component->isConnected() && d->component->isAuthenticated();
}

void QXmppComponent::onElementReceived(const QDomElement &element, bool &handled)
{
    for (auto *extension : std::as_const(d->extensions)) {
        if (extension->handleStanza(element)) {
            handled = true;
            return;
        }
    }
}
