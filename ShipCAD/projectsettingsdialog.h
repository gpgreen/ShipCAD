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

    void initialize(ShipCAD::ProjectSettingsDialogData* data);
    
private:
    Ui::ProjectSettingsDialog *ui;
    ShipCAD::ProjectSettingsDialogData* _data;
};

#endif // PROJECTSETTINGSDIALOG_H
