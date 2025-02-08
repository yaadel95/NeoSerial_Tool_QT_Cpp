// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void readData();

    void on_btn_refresh_clicked();

    void on_btn_connect_clicked();

    void on_btn_send_clicked();

    void on_btn_clear_clicked();

    void on_btn_save_csv_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    bool connected = false;
    QList<QStringList> dataLog;

    void updatePorts();
    void logData(const QString &type, const QString &data);
    void updateIndicator();
};

#endif // MAINWINDOW_H
