#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialBus/QModbusRtuSerialSlave>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QModbusRtuSerialSlave *slave;
    QTimer *timer;
    float value;
    float counter;
    quint16 *pValueLo = ((quint16 *) &value);
    quint16 *pValueHi = ((quint16 *) &value) + 1;
    quint16 *pCounterLo = ((quint16 *) &counter);
    quint16 *pCounterHi = ((quint16 *) &counter) + 1;
    void updatePorts();

private slots:
    void onSlaveStateChanged(QModbusDevice::State state);
    void onSlaveErrorOccured(QModbusDevice::Error error);
    void onTimerTimeout();
    void on_updateAction_triggered();
    void on_runButton_clicked();
    void on_faultBox_stateChanged(int state);
};
#endif // MAINWINDOW_H
