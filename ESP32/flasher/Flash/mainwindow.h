
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSerialPort>
//#include <QTimer>
//#include <QStandardItemModel>
//#include <QNetworkSession>
//#include <QStandardItemModel>
//#include <QNetworkConfiguration>
//#include <QNetworkConfigurationManager>
//#include <QNetworkSession>

#define VERSION 0.11f

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow

{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

//    int foundCount;
//    QNetworkConfiguration netcfg;
//    QStringList WiFisList;
//    QList<QNetworkConfiguration> netcfgList;

//public slots:
//    void findActiveWirelesses();

private slots:
    void on_flashNowButton_clicked();
    void on_refreshComports_clicked();
    void on_flashFinished(int exitcode, QProcess::ExitStatus status);
    void on_flashError();
    void on_flashUpdate();
    void on_serailReadyRead();

    void on_saveWiFiCredsButton_clicked();

    void on_serialSelectorCombobox_currentIndexChanged(int index);

    void on_sendSerialButton_clicked();

    void on_serialCommandInput_returnPressed();

    void on_tabWidget_currentChanged(int index);

    void on_actionAbout_triggered();

    void on_showPasswordCheckbox_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    QProcess* flashProcess = 0;
    QSerialPort* m_serialPort = 0;

    void sendSerial(QString data);
    void checkAndConnectSerial(QString portName);
    void closeSerial();
    void appendToSerialOutput(QString value);
    void appendToFlashOutput(QString value);
    void flashFirmware(QString esptoolPath, QString firmwarePath, QString comport);
    void setFlashMode(bool flashing);
    void showAbout();
//    QTimer* m_finder;
//    QStandardItemModel* listModel;
//    QNetworkSession *session;
};

#endif // MAINWINDOW_H
