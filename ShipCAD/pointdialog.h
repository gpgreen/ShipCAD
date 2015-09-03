#ifndef POINTDIALOG_H
#define POINTDIALOG_H

#include <QDialog>

namespace Ui {
class PointDialog;
}

/*! \brief Dialog to edit coordinates of a Control Point
 */
class PointDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PointDialog(QWidget *parent = 0);
    ~PointDialog();

    /*! \brief show or hide the dialog
     *
     * \param active true if dialog is to be shown, false if hidden
     */
    void setActive(bool active);

private:
    Ui::PointDialog *ui;
};

#endif // POINTDIALOG_H
