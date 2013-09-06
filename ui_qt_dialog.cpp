#include "ui_qt_dialog.h"
#include "ui_ui_qt_dialog.h"


ui_qt_dialog::ui_qt_dialog(QWidget *parent, std::string msg) :
    QDialog(parent),
    ui(new Ui::ui_qt_dialog)
{
    ui->setupUi(this);
    ui->plainTextEdit->appendPlainText(QString::fromStdString(msg));
    auto v = ui->plainTextEdit->textCursor();
    v.setPosition(0);
    ui->plainTextEdit->setTextCursor(v);
}

ui_qt_dialog::~ui_qt_dialog()
{
    delete ui;
}
