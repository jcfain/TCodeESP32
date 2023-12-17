
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVariant>
#include <QMessageBox>
#include <QStringList>
#include <QSerialPortInfo>
#include <QProcess>
#include <QTreeWidgetItem>
#include <QFileInfo>
#include <QDir>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    flashProcess(new QProcess(this)),
    m_serialPort(new QSerialPort(this))
{
    ui->setupUi(this);
    on_refreshComports_clicked();

//    m_finder = new QTimer(this);
//    m_finder->setInterval(1000);
//    connect(m_finder,&QTimer::timeout,this,&MainWindow::findActiveWirelesses);
//    m_finder->start();
//    foundCount = 0;
//    ui->availableWifiNetworks->setColumnWidth(0,50);
//    ui->availableWifiNetworks->setColumnWidth(1,200);
    //menuBar()->hide();
    //statusBar()->hide();
    //flashProcess = new QProcess(this);
    connect(flashProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::on_flashFinished);
    connect(flashProcess, &QProcess::readyRead, this,  &MainWindow::on_flashUpdate);

    //m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::readyRead, this, &MainWindow::on_serailReadyRead);
    ui->binFilePath->setText(QApplication::applicationDirPath() + QDir::separator() + "release.bin");
    ui->espToolInput->setText(QApplication::applicationDirPath() + QDir::separator() + "esptool" + QDir::separator() + "esptool.exe");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendSerial(QString data)
{
    auto currentCom = ui->serialSelectorCombobox->currentData().toString();
    checkAndConnectSerial(currentCom);
    if(m_serialPort->isOpen()) {
        m_serialPort->write(data.toUtf8() + '\n');
        //m_serialPort->waitForBytesWritten();
    } else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Error sending serial data\n") + flashProcess->errorString());
    }
}

void MainWindow::checkAndConnectSerial(QString portName)
{
    if(!m_serialPort->isOpen() || (m_serialPort->isOpen() && m_serialPort->portName() != portName)) {
        closeSerial();
        ui->serialOutputTextEdit->clear();
        m_serialPort->setPortName(portName);
        m_serialPort->setBaudRate(115200);
        //        m_serialPort->setDataBits(8);
        //        m_serialPort->setParity(p.parity);
        //        m_serialPort->setStopBits(p.stopBits);
        //        m_serialPort->setFlowControl(p.flowControl);
        if(m_serialPort->open(QIODevice::ReadWrite)) {
            appendToSerialOutput("Connected to: "+portName +"\n");
            ui->sterialStateInput->setText("Connected to: "+portName);
        } else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Error connecting to:\n") + portName + ": "+m_serialPort->errorString());
            ui->sterialStateInput->setText("Error connecting to: " + portName + ": "+m_serialPort->errorString());
        }
    }
}

void MainWindow::closeSerial()
{
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
        ui->sterialStateInput->setText("Not connected");
    }
}

void MainWindow::appendToSerialOutput(QString value)
{
    ui->serialOutputTextEdit->moveCursor (QTextCursor::End);
    ui->serialOutputTextEdit->insertPlainText (value);
    ui->serialOutputTextEdit->moveCursor (QTextCursor::End);
}

void MainWindow::appendToFlashOutput(QString value)
{
    ui->flashOutputTestEdit->moveCursor (QTextCursor::End);
    ui->flashOutputTestEdit->insertPlainText (value);
    ui->flashOutputTestEdit->moveCursor (QTextCursor::End);
}

void MainWindow::flashFirmware(QString esptoolPath, QString firmwarePath, QString comport)
{
    if(comport.isEmpty()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Invalid comport :empty\n"));
        return;
    }
    if(!QFileInfo::exists(esptoolPath)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Invalie esptool path\n") + esptoolPath);
        return;
    }
    if(!QFileInfo::exists(firmwarePath)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Invalie firmware path\n") + firmwarePath);
        return;
    }
    const QStringList args = {
        "--chip","esp32",
        "--port", comport,
        "--baud","921600",
        "--before","default_reset",
        "--after","hard_reset",
        "write_flash",
        "-z",
        "--flash_mode","dio",
        "--flash_freq","40m",
        "--flash_size","4MB",
        "--erase-all",
        "0x0",
        firmwarePath
    };
    closeSerial();
    flashProcess->start(esptoolPath, args);
    //QFileInfo espToolInfo(esptool);
    //process.setWorkingDirectory(QApplication::applicationDirPath());

    //.\esptool\esptool.exe --chip esp32 --port %comport% --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB --erase-all 0x0 release.bin

    //    ui->wifiPasswordInput->setEnabled(true);
    //    ui->availableWifiNetworks->setEnabled(true);
}

void MainWindow::setFlashMode(bool flashing)
{
    ui->tabWidget->setEnabled(!flashing);
    ui->serialSelectorCombobox->setEnabled(!flashing);
    ui->flashNowButton->setEnabled(!flashing);
    ui->refreshComports->setEnabled(!flashing);
}

void MainWindow::showAbout()
{
    QMessageBox::information(this, tr("About"),
                             tr("Version:") + QString::number(VERSION) + "\nLicense: GPLV3");

}

//void MainWindow::findActiveWirelesses()
//{
//    QNetworkConfigurationManager ncm;
//    netcfgList = ncm.allConfigurations();
//    WiFisList.clear();
//    for (auto &x : netcfgList)
//    {
//        if (x.bearerType() == QNetworkConfiguration::BearerWLAN)
//        {
//            if(x.name() == "")
//                WiFisList << "Unknown(Other Network)";
//            else
//                WiFisList << x.name();

//            qDebug() << x.type();
//        }
//    }
//    for(int i=0; i<WiFisList.size(); i++)
//    {
//        bool exist = false;
//        for(int j=0; j<ui->availableWifiNetworks->topLevelItemCount(); j++)
//        {
//            QTreeWidgetItem *index = ui->availableWifiNetworks->topLevelItem(j);
//            QString str = index->text(1);
//            if(str == WiFisList[i])
//            {
//                exist = true;
//                break;
//            }
//        }
//        if(!exist)
//        {
//            QTreeWidgetItem * item = new QTreeWidgetItem();
//            item->setTextAlignment(0,Qt::AlignVCenter);
//            item->setTextAlignment(1,Qt::AlignHCenter);
//            item->setText(0,QString::number(++foundCount));
//            item->setText(1,WiFisList[i]);
//            ui->availableWifiNetworks->addTopLevelItem(item);
//        }
//    }
//}

//Other events///////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::on_flashFinished(int exitcode, QProcess::ExitStatus status) {

    if(exitcode != 0) {
        on_flashError();
        return;
    }
    // ui->flashOutputTestEdit->toPlainText().contains("Hash of data verified", Qt::CaseInsensitive)
    setFlashMode(false);
    QMessageBox::information(this, tr("Success"),
                             tr("Flashing finished!"));
}

void MainWindow::on_flashError() {
    appendToFlashOutput( tr("Error flashing\n") + flashProcess->errorString());
    setFlashMode(false);
}

void MainWindow::on_flashUpdate()
{
    QString appendText(flashProcess->readAll());
    if(!appendText.isEmpty())
        appendToFlashOutput(appendText);
}

void MainWindow::on_serailReadyRead()
{
    QString appendText(m_serialPort->readAll());
    if(!appendText.isEmpty())
        appendToSerialOutput(appendText);
}

//UI events//////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::on_refreshComports_clicked()
{
    ui->serialSelectorCombobox->clear();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        QVariant comport(serialPortInfo.portName());
        ui->serialSelectorCombobox->addItem(serialPortInfo.portName() + " - " + serialPortInfo.description(), comport);
    }
}


void MainWindow::on_flashNowButton_clicked()
{
    //    ui->wifiPasswordInput->setEnabled(false);
    //    ui->availableWifiNetworks->setEnabled(false);
    ui->flashOutputTestEdit->clear();
    auto esptool = ui->espToolInput->text();
    auto comport = ui->serialSelectorCombobox->currentData().toString();
    auto firmwareBinPath = ui->binFilePath->text();
    if(esptool.isEmpty() || comport.isEmpty()) {
        auto message = esptool.isEmpty() ? "Invalid esptool" : "Invalid comport";
        QMessageBox::critical(this, tr("Invalid form"),
                              tr(message));
        return;
    }
    setFlashMode(true);
    flashFirmware(esptool, firmwareBinPath, comport);
}

void MainWindow::on_saveWiFiCredsButton_clicked()
{
    auto ssid = ui->wifiSSIDInput->text();
    auto password = ui->wifiPassInput->text();
    bool modified = false;
    if(!ssid.isEmpty()) {
        sendSerial("#wifi-ssid:"+ssid);
        modified = true;
    }
    if(!password.isEmpty()) {
        sendSerial("#wifi-pass:"+password);
        modified = true;
    }
    if(modified) {
        sendSerial("$save");
        sendSerial("#restart");
        ui->tabWidget->setCurrentIndex(3);
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Nothing to modify"));
    }
}

void MainWindow::on_sendSerialButton_clicked()
{
    auto command = ui->serialCommandInput->text();
    if(!command.isEmpty()) {
        sendSerial(command);
        ui->serialCommandInput->clear();
    } else
        ui->serialOutputTextEdit->append("Nothing to send!");
}

void MainWindow::on_serialCommandInput_returnPressed()
{
    on_sendSerialButton_clicked();
}

void MainWindow::on_serialSelectorCombobox_currentIndexChanged(int index)
{
    auto currentCom = ui->serialSelectorCombobox->currentData().toString();
    checkAndConnectSerial(currentCom);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch(index) {
        case 0:
            ui->serialSelectorCombobox->setFocus();
            break;
        case 1:
            ui->flashNowButton->setFocus();
            break;
        case 2:
            ui->wifiSSIDInput->setFocus();
            break;
        case 3:
            ui->serialCommandInput->setFocus();
            break;
    }
}

void MainWindow::on_actionAbout_triggered()
{
    showAbout();
}


void MainWindow::on_showPasswordCheckbox_clicked(bool checked)
{
    ui->wifiPassInput->setEchoMode(checked ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password);
}

