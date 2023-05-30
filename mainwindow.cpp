#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QDateTime>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    slave = new QModbusRtuSerialSlave();
    timer = new QTimer();
    updatePorts();
    slave->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud19200);
    slave->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    slave->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::TwoStop);
    slave->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::EvenParity);
    connect(slave, &QModbusDevice::stateChanged, this, &MainWindow::onSlaveStateChanged);

    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
    QModbusDataUnitMap reg;
    reg.insert(QModbusDataUnit::HoldingRegisters, {QModbusDataUnit::HoldingRegisters, 0, 46000});
    slave->setMap(reg);
    on_faultBox_stateChanged(ui->faultBox->checkState());
}

MainWindow::~MainWindow()
{
    delete ui;
    slave->deleteLater();
    timer->deleteLater();
}

void MainWindow::updatePorts()
{
    while (ui->portBox->count()) ui->portBox->removeItem(0);
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();

    for (int i=0; i<portList.count(); i++)
    {
        ui->portBox->addItem(portList[i].portName());
    }
    ui->portBox->model()->sort(0);
    ui->portBox->setCurrentIndex(0);
}

void MainWindow::on_updateAction_triggered()
{
    updatePorts();
}


void MainWindow::on_runButton_clicked()
{
    if (slave->state() == QModbusDevice::UnconnectedState) {
        slave->setConnectionParameter(QModbusDevice::SerialPortNameParameter, ui->portBox->currentText());
        slave->setServerAddress(ui->addressBox->value());
        slave->connectDevice();
    } else if (slave->state() == QModbusDevice::ConnectedState) {
        slave->disconnectDevice();
    }
}

void MainWindow::onSlaveStateChanged(QModbusDevice::State state)
{
    if (state == QModbusDevice::UnconnectedState) {
        timer->stop();
        ui->runButton->setText("Запустить");
        ui->intervalEdit->setEnabled(true);
        ui->portBox->setEnabled(true);
        ui->addressBox->setEnabled(true);
        ui->runButton->setEnabled(true);
        statusBar()->showMessage("Устройство остановлено", 5000);
    }
    else if (state == QModbusDevice::ConnectedState) {
        counter = 0;
        timer->setInterval(ui->intervalEdit->text().toInt()*1000);
        timer->start();
        ui->runButton->setText("Остановить");
        ui->intervalEdit->setEnabled(false);
        ui->portBox->setEnabled(false);
        ui->addressBox->setEnabled(false);
        ui->runButton->setEnabled(true);
        statusBar()->showMessage("Устройство запущено", 5000);
    }
    else {
        ui->runButton->setEnabled(false);
    }
}

void MainWindow::onSlaveErrorOccured(QModbusDevice::Error error)
{
    if (error == QModbusDevice::NoError) return;
    statusBar()->showMessage("Ошибка: " + slave->errorString(), 5000);
}

void MainWindow::onTimerTimeout()
{
    counter++;
    ui->counterLabel->setText(QString("%1").arg(counter));
    float base = ui->valueEdit->text().toFloat();
    float deviation = (base/100) * ui->deviationEdit->text().toInt();
    qsrand(QDateTime::currentMSecsSinceEpoch());
    float rand = (float) qrand()/(float) RAND_MAX;
    float sign = (float) qrand()/(float) RAND_MAX;
    deviation = deviation * rand;
    if (sign < 0.5) deviation = -deviation;
    value = base + deviation;
    ui->valueLabel->setText(QString("%1").arg(value, 0, 'E', 3));
    slave->setData(QModbusDataUnit::HoldingRegisters, 10011, *pValueLo);
    slave->setData(QModbusDataUnit::HoldingRegisters, 10012, *pValueHi);
    slave->setData(QModbusDataUnit::HoldingRegisters, 45008, *pCounterLo);
    slave->setData(QModbusDataUnit::HoldingRegisters, 45009, *pCounterHi);
}

void MainWindow::on_faultBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        slave->setData(QModbusDataUnit::HoldingRegisters, 2000, 0xffff);
        slave->setData(QModbusDataUnit::HoldingRegisters, 2001, 0);
    }
    else {
        slave->setData(QModbusDataUnit::HoldingRegisters, 2000, 0);
        slave->setData(QModbusDataUnit::HoldingRegisters, 2001, 0xffff);
    }
}
