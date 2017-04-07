#include "layerdialog.h"
#include "ui_layerdialog.h"

LayerDialog::LayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerDialog)
{
    ui->setupUi(this);
}

LayerDialog::~LayerDialog()
{
    delete ui;
}
