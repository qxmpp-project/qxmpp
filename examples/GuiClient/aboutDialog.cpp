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


#include "aboutDialog.h"
#include "ui_aboutDialog.h"

#include "QXmppGlobal.h"

#include <QtGlobal>

aboutDialog::aboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("About %1").arg(qApp->applicationName()));

    ui->textEdit->append(QString("Copyright (C) 2008-2014 The QXmpp developers\n"));
    ui->textEdit->append(qApp->applicationName() + " " + qApp->applicationVersion());
    ui->textEdit->append(QString("\nBased on:"));
    ui->textEdit->append(QString("QXmpp %1").arg(QXmppVersion()));
    ui->textEdit->append(QString("Qt %1 [built-with]").arg(qVersion()));
    ui->textEdit->append(QString("Qt %1 [running-with]").arg(QT_VERSION_STR));
}

aboutDialog::~aboutDialog()
{
    delete ui;
}
