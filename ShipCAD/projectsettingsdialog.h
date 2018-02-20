#ifndef PROJECTSETTINGSDIALOG_H
#define PROJECTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectSettingsDialog;
}

namespace ShipCAD {
class ProjectSettingsDialogData;
}

class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ProjectSettingsDialog(QWidget *parent = 0);
    ~ProjectSettingsDialog();

public slots:

    void initialize(ShipCAD::ProjectSettingsDialogData* data);
    void retrieve();
    void unitsChanged(int);
    
private:

    Ui::ProjectSettingsDialog *ui;
    ShipCAD::ProjectSettingsDialogData* _data;
};

#endif // PROJECTSETTINGSDIALOG_H
