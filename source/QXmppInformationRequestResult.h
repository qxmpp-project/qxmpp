#ifndef QXMPPINFORMATIONREQUESTRESULT_H
#define QXMPPINFORMATIONREQUESTRESULT_H

#include "QXmppIq.h"

class QXmppInformationRequestResult : public QXmppIq
{
public:
    QXmppInformationRequestResult();
    virtual QByteArray toXmlElementFromChild() const;
};

#endif // QXMPPINFORMATIONREQUESTRESULT_H
