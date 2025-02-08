
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFont>
#include <QFontDatabase>
#include <QFile>
#include <QGraphicsEffect>
#include <QPropertyAnimation>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new QSerialPort(this))
{
    ui->setupUi(this);

    // Load custom font
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Orbitron-VariableFont_wght.ttf");
    if (fontId == -1) {
        qDebug() << "Failed to load font!";
    } else {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont font(family, 10);
        qApp->setFont(font);
    }

    // Apply stylesheet
    QFile file(":/styles.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    } else {
        qDebug() << "Failed to open stylesheet file!";
    }

    // Add to MainWindow constructor
    QGraphicsDropShadowEffect *indicatorEffect = new QGraphicsDropShadowEffect(this);
    indicatorEffect->setBlurRadius(20);
    indicatorEffect->setOffset(0);
    ui->indicator->setGraphicsEffect(indicatorEffect);

    // Add hover animations to buttons
    for (QPushButton* btn : findChildren<QPushButton*>()) {
        btn->setCursor(Qt::PointingHandCursor);

        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(10);
        effect->setOffset(0);
        btn->setGraphicsEffect(effect);

        QObject::connect(btn, &QPushButton::pressed, [=]() {
            QPropertyAnimation *anim = new QPropertyAnimation(btn, "geometry");
            anim->setDuration(100);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->setStartValue(btn->geometry());
            anim->setEndValue(btn->geometry().adjusted(-2, -2, 2, 2));
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });

        QObject::connect(btn, &QPushButton::released, [=]() {
            QPropertyAnimation *anim = new QPropertyAnimation(btn, "geometry");
            anim->setDuration(100);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->setStartValue(btn->geometry());
            anim->setEndValue(btn->geometry().adjusted(2, 2, -2, -2));
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

    // Initialize indicator
    ui->indicator->setProperty("connected", false);
    ui->indicator->setFixedSize(20, 20);


    // Setup baud rates
    ui->cb_baudrate->addItems({"9600", "19200", "38400", "57600", "115200"});
    ui->cb_baudrate->setCurrentText("115200");

    // Connect signals
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(ui->txt_send, &QLineEdit::returnPressed, this, &MainWindow::on_btn_connect_clicked);

    updatePorts();
}

MainWindow::~MainWindow()
{
    if(serial->isOpen()) serial->close();
    delete ui;
}

void MainWindow::updatePorts()
{
    ui->cb_port->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &port : ports)
        ui->cb_port->addItem(port.portName());
}


void MainWindow::readData()
{
    const QByteArray data = serial->readAll();
    QString received = QString::fromUtf8(data).trimmed();

    if(!received.isEmpty()) {
        ui->txt_receive->append("RECV: " + received);
        logData("RECEIVED", received);
    }
}

void MainWindow::logData(const QString &type, const QString &data)
{
    QStringList entry;
    entry << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    entry << type;
    entry << data;
    dataLog.append(entry);
}


void MainWindow::on_btn_refresh_clicked()
{
        updatePorts();
}


void MainWindow::on_btn_connect_clicked()
{
    if(!connected) {
        serial->setPortName(ui->cb_port->currentText());
        serial->setBaudRate(ui->cb_baudrate->currentText().toInt());

        if(serial->open(QIODevice::ReadWrite)) {
            connected = true;
            ui->btn_connect->setText("Disconnect");
            updateIndicator();
            QMessageBox::information(this, "Connected",
                                     "Successfully connected to " + serial->portName());
        } else {
            QMessageBox::critical(this, "Error", serial->errorString());
        }
    } else {
        serial->close();
        connected = false;
        ui->btn_connect->setText("Connect");
        updateIndicator();
        QMessageBox::information(this, "Disconnected", "Connection closed");
    }
}


void MainWindow::on_btn_send_clicked()
{
    if(!connected) {
        QMessageBox::warning(this, "Warning", "Not connected to any device!");
        return;
    }

    const QString data = ui->txt_send->text() + "\n";
    if(serial->write(data.toUtf8()) > 0) {
        logData("SENT", ui->txt_send->text());
        ui->txt_send->clear();
    } else {
        QMessageBox::critical(this, "Error", "Failed to send data");
    }
}


void MainWindow::on_btn_clear_clicked()
{
    ui->txt_receive->clear();
    dataLog.clear();
}


void MainWindow::on_btn_save_csv_clicked()
{
    if(dataLog.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No data to save!");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save CSV File", "", "CSV Files (*.csv)");

    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << "Timestamp,Type,Data\n";
            for(const QStringList &entry : dataLog) {
                stream << entry.join(',') << '\n';
            }
            file.close();
            QMessageBox::information(this, "Success", "Data saved successfully");
        } else {
            QMessageBox::critical(this, "Error", "Failed to save file");
        }
    }
}

void MainWindow::updateIndicator()
{
    if(connected) {
        ui->indicator->setStyleSheet(
            "background-color: #00ff00;"
            "border-radius: 10px;"
            "border: 2px solid #00ff00;"
            );
    } else {
        ui->indicator->setStyleSheet(
            "background-color: red;"
            "border-radius: 10px;"
            "border: 2px solid #2a2a4a;"
            );
    }
}

