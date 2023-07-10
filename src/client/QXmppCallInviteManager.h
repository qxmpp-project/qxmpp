// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCALLINVITEMANAGER_H
#define QXMPPCALLINVITEMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppError.h"
#include "QXmppJingleIq.h"
#include "QXmppMessageHandler.h"
#include "QXmppSendResult.h"
#include "QXmppTask.h"

class QXmppCallInviteManager;
class QXmppCallInvitePrivate;
class QXmppCallInviteManagerPrivate;

class QXMPP_EXPORT QXmppCallInvite : public QObject
{
    Q_OBJECT
public:
    struct Rejected
    {
    };
    struct Retracted
    {
    };
    struct Left
    {
    };

    using Result = std::variant<Rejected, Retracted, Left, QXmppError>;

    explicit QXmppCallInvite(QXmppCallInviteManager *manager);
    ~QXmppCallInvite();

    QXmppTask<QXmpp::SendResult> accept();
    QXmppTask<QXmpp::SendResult> reject();
    QXmppTask<QXmpp::SendResult> retract();
    QXmppTask<QXmpp::SendResult> leave();

    Q_SIGNAL void invited();
    Q_SIGNAL void accepted(const QString &id, const QString &callPartnerResource);
    Q_SIGNAL void closed(const Result &result);

private:
    QXmppTask<QXmpp::SendResult> invite(
        bool audio = true,
        bool video = false,
        std::optional<QXmppCallInviteElement::Jingle> jingle = std::nullopt,
        std::optional<QVector<QXmppCallInviteElement::External>> external = std::nullopt);

    QString id() const;
    void setId(const QString &id);
    void setCallPartnerJid(const QString &callPartnerJid);
    QString callPartnerJid() const;
    bool isAccepted() const;
    void setIsAccepted(bool isAccepted);

    std::unique_ptr<QXmppCallInvitePrivate> d;

    friend class QXmppCallInviteManager;
    friend class tst_QXmppCallInviteManager;
};

class QXMPP_EXPORT QXmppCallInviteManager : public QXmppClientExtension, public QXmppMessageHandler
{
    Q_OBJECT
public:
    using ProposeResult = std::variant<std::shared_ptr<QXmppCallInvite>, QXmppError>;

    QXmppCallInviteManager();
    ~QXmppCallInviteManager();

    /// \cond
    QStringList discoveryFeatures() const override;
    /// \endcond

    QXmppTask<ProposeResult> invite(
        const QString &callPartnerJid,
        bool audio = true,
        bool video = false,
        std::optional<QXmppCallInviteElement::Jingle> jingle = std::nullopt,
        std::optional<QVector<QXmppCallInviteElement::External>> external = std::nullopt);

    Q_SIGNAL void invited(const std::shared_ptr<QXmppCallInvite> &callInvite, const QString &id);

protected:
    /// \cond
    bool handleMessage(const QXmppMessage &) override;
    void setClient(QXmppClient *client) override;
    /// \endcond

private:
    QXmppTask<QXmpp::SendResult> sendMessage(
        const QXmppCallInviteElement &callInviteElement,
        const QString &callPartnerJid);

    void clear(const std::shared_ptr<QXmppCallInvite> &callInvite);
    void clearAll();

    bool handleCallInviteElement(QXmppCallInviteElement &&callInviteElement, const QString &senderJid);

    bool handleExistingCallInvite(
        const std::shared_ptr<QXmppCallInvite> &existingCallInvite,
        const QXmppCallInviteElement &callInviteElement,
        const QString &callPartnerResource);

    bool handleInviteCallInviteElement(const QXmppCallInviteElement &callInviteElement, const QString &callPartnerJid);

    std::shared_ptr<QXmppCallInvite> addCallInvite(const QString &callPartnerJid);
    const QVector<std::shared_ptr<QXmppCallInvite>> &callInvites() const;

private:
    std::unique_ptr<QXmppCallInviteManagerPrivate> d;

    friend class QXmppCallInvitePrivate;
    friend class tst_QXmppCallInviteManager;
};
Q_DECLARE_METATYPE(QXmppCallInvite::Result)
Q_DECLARE_METATYPE(std::shared_ptr<QXmppCallInvite>)

#endif  // QXMPPJINGLEMESSAGEINITIATIONMANAGER_H
