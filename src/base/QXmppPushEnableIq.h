// SPDX-FileCopyrightText: 2020 Robert Märkisch <zatrox@kaidan.im>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUSHENABLEIQ_H
#define QXMPPPUSHENABLEIQ_H

#include <QXmppIq.h>

class QXmppPushEnableIqPrivate;
class QXmppDataForm;

///
/// \brief This class represents an IQ to enable or disablepush notifications
/// on the user server.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.3
///
class QXMPP_EXPORT QXmppPushEnableIq : public QXmppIq
{
public:
    QXmppPushEnableIq();
    QXmppPushEnableIq(const QXmppPushEnableIq &);
    QXmppPushEnableIq(QXmppPushEnableIq &&);
    ~QXmppPushEnableIq();
    QXmppPushEnableIq &operator=(const QXmppPushEnableIq &);
    QXmppPushEnableIq &operator=(QXmppPushEnableIq &&);

    ///
    /// \brief The Mode enum describes whether the IQ should enable or disable
    /// push notifications
    ///
    enum Mode : bool {
        Enable = true,
        Disable = false
    };

    QString jid() const;
    void setJid(const QString &jid);

    QString node() const;
    void setNode(const QString &node);

    void setMode(Mode mode);
    Mode mode();

    QXmppDataForm dataForm() const;
    void setDataForm(const QXmppDataForm &form);

    static bool isPushEnableIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppPushEnableIqPrivate> d;
};

#endif  // QXMPPPUSHENABLEIQ_H
