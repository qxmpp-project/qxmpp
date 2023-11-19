// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPIQ_H
#define QXMPPIQ_H

#include "QXmppStanza.h"

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include <QXmlStreamWriter>

class QXmppIqPrivate;

/// \brief The QXmppIq class is the base class for all IQs.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppIq : public QXmppStanza
{
public:
    /// This enum describes the type of IQ.
    enum Type {
        Error = 0,  ///< Error response.
        Get,        ///< Get request.
        Set,        ///< Set request.
        Result      ///< Result.
    };

    QXmppIq(QXmppIq::Type type = QXmppIq::Get);
    QXmppIq(const QXmppIq &other);
    QXmppIq(QXmppIq &&);
    ~QXmppIq() override;

    QXmppIq &operator=(const QXmppIq &other);
    QXmppIq &operator=(QXmppIq &&);

    QXmppIq::Type type() const;
    void setType(QXmppIq::Type);

    bool isXmppStanza() const override;

    /// \cond
    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;

    virtual void parseElementFromChild(const QDomElement &element);
    virtual void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppIqPrivate> d;
};

Q_DECLARE_METATYPE(QXmppIq::Type)

#endif  // QXMPPIQ_H
