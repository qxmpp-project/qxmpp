/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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


#ifndef XMLCONSOLEDIALOG_H
#define XMLCONSOLEDIALOG_H

#include <QDialog>
#include "QXmppLogger.h"

namespace Ui {
    class xmlConsoleDialog;
}

class xmlConsoleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit xmlConsoleDialog(QWidget *parent = 0);
    ~xmlConsoleDialog();

public slots:
    void message(QXmppLogger::MessageType, const QString &);

private:
    Ui::xmlConsoleDialog *ui;
};

#endif // XMLCONSOLEDIALOG_H
