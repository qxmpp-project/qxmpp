/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#ifndef QXMPPMUCIQ_H
#define QXMPPMUCIQ_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"

/// \brief The QXmppClient class represents a chat room configuration IQ as
/// defined by XEP-0045: Multi-User Chat.
///
/// \sa QXmppDataForm
///

class QXmppMucOwnerIq : public QXmppIq
{
public:
    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);

    static bool isMucOwnerIq(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QXmppDataForm m_form;
};

#endif
