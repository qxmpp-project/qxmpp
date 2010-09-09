#include "profileDialog.h"
#include "ui_profileDialog.h"

profileDialog::profileDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::profileDialog)
{
    ui->setupUi(this);
}

profileDialog::~profileDialog()
{
    delete ui;
}

void profileDialog::setAvatar(const QImage& image)
{
    ui->label_avatar->setPixmap(QPixmap::fromImage(image));
}

void profileDialog::setBareJid(const QString& bareJid)
{
    ui->label_jid->setText(bareJid);
    setWindowTitle(bareJid);
}

void profileDialog::setFullName(const QString& fullName)
{
    ui->label_fullName->setText(fullName);
}

void profileDialog::setStatusText(const QString& status)
{
    ui->label_status->setText(status);
}
