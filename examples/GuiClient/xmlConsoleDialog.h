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
