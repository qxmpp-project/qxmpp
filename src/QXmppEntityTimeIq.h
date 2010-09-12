#ifndef QXMPPENTITYTIMEIQ_H
#define QXMPPENTITYTIMEIQ_H

#include "QXmppIq.h"

/// \ingroup Stanzas

class QXmppEntityTimeIq : public QXmppIq
{
public:
    QString tzo() const;
    void setTzo(const QString &tzo);

    QString utc() const;
    void setUtc(const QString &utc);

    static bool isEntityTimeIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_tzo;
    QString m_utc;
};

#endif //QXMPPENTITYTIMEIQ_H
