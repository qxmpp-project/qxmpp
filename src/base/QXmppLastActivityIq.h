#ifndef QXMPPLASTACTIVITYIQ_H
#define QXMPPLASTACTIVITYIQ_H

#include "QXmppIq.h"

class QXmppLastActivityIqPrivate;

/// \brief The QXmppLastActivityIq class represents an IQ for conveying a
/// last activity as defined by:
/// \li XEP-0012: Last Activity.
/// \li XEP-0256: Last Activity in Presence.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppLastActivityIq : public QXmppIq
{
public:
    QXmppLastActivityIq(const QString& to = "");
    QXmppLastActivityIq(const QXmppLastActivityIq& other);
    ~QXmppLastActivityIq();

    QXmppLastActivityIq& operator=(const QXmppLastActivityIq& other);

    quint64 seconds() const;
    void setSeconds(quint64 seconds);

    QString status() const;
    void setStatus(const QString& status);

    /// \cond
    static bool isLastActivityIq(const QDomElement& element);
    /// \endcond

protected:
    /// \cond
    virtual void parseElementFromChild(const QDomElement& element);
    virtual void toXmlElementFromChild(QXmlStreamWriter* writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppLastActivityIqPrivate> d;
};

#endif // QXMPPLASTACTIVITYIQ_H
