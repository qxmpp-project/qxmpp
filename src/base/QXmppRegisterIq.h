// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPREGISTERIQ_H
#define QXMPPREGISTERIQ_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"

class QXmppBitsOfBinaryDataList;
class QXmppRegisterIqPrivate;

/// \brief The QXmppRegisterIq class represents a registration IQ
/// as defined by \xep{0077}: In-Band Registration.
///
/// It is used to create an account on the server.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppRegisterIq : public QXmppIq
{
public:
    QXmppRegisterIq();
    QXmppRegisterIq(const QXmppRegisterIq &other);
    QXmppRegisterIq(QXmppRegisterIq &&);
    ~QXmppRegisterIq() override;

    QXmppRegisterIq &operator=(const QXmppRegisterIq &other);
    QXmppRegisterIq &operator=(QXmppRegisterIq &&);

    static QXmppRegisterIq createChangePasswordRequest(const QString &username, const QString &newPassword, const QString &to = {});
    static QXmppRegisterIq createUnregistrationRequest(const QString &to = {});

    QString email() const;
    void setEmail(const QString &email);

    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);

    QString instructions() const;
    void setInstructions(const QString &instructions);

    QString password() const;
    void setPassword(const QString &username);

    QString username() const;
    void setUsername(const QString &username);

    bool isRegistered() const;
    void setIsRegistered(bool isRegistered);

    bool isRemove() const;
    void setIsRemove(bool isRemove);

    QXmppBitsOfBinaryDataList bitsOfBinaryData() const;
    QXmppBitsOfBinaryDataList &bitsOfBinaryData();
    void setBitsOfBinaryData(const QXmppBitsOfBinaryDataList &bitsOfBinaryData);

    QString outOfBandUrl() const;
    void setOutOfBandUrl(const QString &outOfBandUrl);

    /// \cond
    static bool isRegisterIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppRegisterIqPrivate> d;
};

#endif
