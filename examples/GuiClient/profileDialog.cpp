#include "profileDialog.h"
#include "ui_profileDialog.h"

profileDialog::profileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::profileDialog)
{
    ui->setupUi(this);
}

profileDialog::~profileDialog()
{
    delete ui;
}
