#ifndef QXMPPMESSAGECARBONSIQ_H
#define QXMPPMESSAGECARBONSIQ_H

#include "QXmppIq.h"

class QXmlStreamWriter;
class QDomElement;


/// \brief Represents an message carbons query as defined by
/// XEP-0280: Message Carbons
///
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppMessageCarbonsIq : public QXmppIq
{
public:
    QXmppMessageCarbonsIq();


    /// \cond

protected:
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

};
#endif // QXMPPMESSAGECARBONSIQ_H
