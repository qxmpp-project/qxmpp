/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


#ifndef QXMPPSESSIONIQ_H
#define QXMPPSESSIONIQ_H

#include "QXmppIq.h"

/// \brief The QXmppSessionIq class represents an IQ used for session
/// establishment as defined by RFC 5921.
///
/// \ingroup Stanzas

class QXmppSessionIq : public QXmppIq
{
public:
    /// \cond
    static bool isSessionIq(const QDomElement &element);
    /// \endcond

private:
    /// \cond
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond
};

#endif // QXMPPSESSION_H
