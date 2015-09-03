#include "pointdialog.h"
#include "ui_pointdialog.h"

PointDialog::PointDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PointDialog)
{
    ui->setupUi(this);
    setModal(false);
}

PointDialog::~PointDialog()
{
    delete ui;
}

void PointDialog::setActive(bool active)
{
    if (active)
        show();
    else
        hide();
}
