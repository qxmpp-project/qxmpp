// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPEXTERNALSERVICEDISCOVERYIQ_H
#define QXMPPEXTERNALSERVICEDISCOVERYIQ_H

#include "QXmppIq.h"

#include <QDateTime>
#include <QSharedDataPointer>

class QDateTime;
class QXmppExternalServicePrivate;
class QXmppExternalServiceDiscoveryIqPrivate;

class QXMPP_EXPORT QXmppExternalService
{
public:
    ///
    /// Describes the action type of an external service IQ.
    ///
    /// \since QXmpp 1.6
    ///
    enum class Action {
        Add,
        Delete,
        Modify
    };

    ///
    /// Describes the packet type of an external service IQ.
    ///
    /// \since QXmpp 1.6
    ///
    enum class Transport {
		Tcp,
		Udp
    };

    QXmppExternalService();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppExternalService)

    QString host() const;
    void setHost(const QString &);

    QString type() const;
    void setType(const QString &);

    std::optional<Action> action() const;
	void setAction(std::optional<Action>);

    std::optional<QDateTime> expires() const;
	void setExpires(std::optional<QDateTime>);

    std::optional<QString> name() const;
	void setName(std::optional<QString>);

    std::optional<QString> password() const;
	void setPassword(std::optional<QString>);

    std::optional<int> port() const;
	void setPort(std::optional<int>);

    std::optional<bool> restricted() const;
	void setRestricted(std::optional<bool>);

    std::optional<Transport> transport() const;
	void setTransport(std::optional<Transport>);

    std::optional<QString> username() const;
	void setUsername(std::optional<QString>);

    static bool isExternalService(const QDomElement &);

    void parse(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

private:
    QSharedDataPointer<QXmppExternalServicePrivate> d;
};

class QXMPP_EXPORT QXmppExternalServiceDiscoveryIq : public QXmppIq
{
public:
    QXmppExternalServiceDiscoveryIq();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppExternalServiceDiscoveryIq)

    QVector<QXmppExternalService> externalServices();
    void setExternalServices(const QVector<QXmppExternalService> &);
    void addExternalService(const QXmppExternalService &);

    static bool isExternalServiceDiscoveryIq(const QDomElement &);
    static bool checkIqType(const QString &tagName, const QString &xmlNamespace);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppExternalServiceDiscoveryIqPrivate> d;
};

#endif  // QXMPPEXTERNALSERVICEDISCOVERYIQ_H
