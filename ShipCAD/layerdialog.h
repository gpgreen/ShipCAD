#ifndef LAYERDIALOG_H
#define LAYERDIALOG_H

#include <QDialog>

namespace Ui {
class LayerDialog;
}

class LayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerDialog(QWidget *parent = 0);
    ~LayerDialog();

private:
    Ui::LayerDialog *ui;
};

#endif // LAYERDIALOG_H
