#ifndef UI_QT_DIALOG_H
#define UI_QT_DIALOG_H

#include <QDialog>

namespace Ui {
class ui_qt_dialog;
}

class ui_qt_dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ui_qt_dialog(QWidget *parent, std::string msg);
    ~ui_qt_dialog();
    
private:
    Ui::ui_qt_dialog *ui;
};

#endif // UI_QT_DIALOG_H
