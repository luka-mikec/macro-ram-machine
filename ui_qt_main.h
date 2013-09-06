#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

void alert(std::string ttl, std::string msg, QWidget *parent_wnd, bool fullblown = true);

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();
    
private slots:
    void on_actionNew_triggered();
    void on_actionLoad_triggered();
    void on_actionInspect_stdlib_triggered();
    void on_actionExit_triggered();
    void on_actionCompile_triggered();
    void on_actionRun_triggered();
    void on_actionCompile_Run_triggered();
    void on_actionAbout_triggered();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
