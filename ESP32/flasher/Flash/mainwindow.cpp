
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
    m_serialPort(new QSerialPort(this)),
    urlRegex(R"/((([A-Za-z]{3,9}:(?:\/\/)?)(?:[-;:&=\+\$,\w]+@)?[A-Za-z0-9.-]+|(?:www.|[-;:&=\+\$,\w]+@)[A-Za-z0-9.-]+)((?:\/[\+~%\/.\w-_]*)?\??(?:[-\+=&;%@.\w_]*)#?(?:[\w]*))?)/")
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
    if(!checkAndConnectSerial()) {
        return;
    }
    if(m_serialPort->isOpen()) {
        m_serialPort->write(data.toUtf8() + '\n');
        m_serialPort->flush();
        //m_serialPort->waitForBytesWritten();
    } else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Error sending serial data\n") + flashProcess->errorString());
    }
}

bool MainWindow::checkAndConnectSerial()
{
    auto portName = ui->serialSelectorCombobox->currentData().toString();
    if(!m_serialPort->isOpen() || (m_serialPort->isOpen() && m_serialPort->portName() != portName)) {
        closeSerial();
        ui->serialOutputTextBrowser->clear();
        m_serialPort->setPortName(portName);
        if(m_serialPort->open(QIODevice::ReadWrite)) {
            if(!m_serialPort->setBaudRate(QSerialPort::BaudRate::Baud115200)) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Error setting baud:\n115200") +m_serialPort->errorString());
                closeSerial();
                return false;
            }
            // if(!m_serialPort->setDataBits(QSerialPort::DataBits::Data8)) {
            //     QMessageBox::critical(this, tr("Error"),
            //                           tr("Error setting databit:\n8") +m_serialPort->errorString());
            //     closeSerial();
            //     return false;
            // }
            // if(!m_serialPort->setParity(QSerialPort::NoParity)) {
            //     QMessageBox::critical(this, tr("Error"),
            //                           tr("Error setting databit:\n8") +m_serialPort->errorString());
            //     closeSerial();
            //     return false;
            // }
            // if(!m_serialPort->setStopBits(QSerialPort::StopBits::OneStop)) {
            //     QMessageBox::critical(this, tr("Error"),
            //                           tr("Error setting stopbits:\nOneStop") +m_serialPort->errorString());
            //     closeSerial();
            //     return false;
            // }
            // if(!m_serialPort->setFlowControl(QSerialPort::FlowControl::SoftwareControl)) {
            //     QMessageBox::critical(this, tr("Error"),
            //                           tr("Error setting flow control:\nsoftware") +m_serialPort->errorString());
            //     closeSerial();
            //     return false;
            // }
            appendToSerialOutput("Connected to: "+portName +"\n");
            ui->sterialStateInput->setText("Connected to: "+portName);
        } else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Error connecting to:\n") + portName + ": "+m_serialPort->errorString());
            ui->sterialStateInput->setText("Error connecting to: " + portName + ": "+m_serialPort->errorString());
            return false;
        }
    }
    return true;
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
    ui->serialOutputTextBrowser->moveCursor (QTextCursor::End);
    ui->serialOutputTextBrowser->insertPlainText (value);
    ui->serialOutputTextBrowser->moveCursor (QTextCursor::End);
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
    setFlashMode(true);
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
    m_serialPort->flush();
    if(!appendText.isEmpty())
        appendToSerialOutput(appendText);
}

//UI events//////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::on_refreshComports_clicked()
{

    disconnect(ui->serialSelectorCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_serialSelectorCombobox_indexChanged);
    int currentIndex = 0;
    int esp32Index = -1;
    ui->serialSelectorCombobox->clear();

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        QVariant comport(serialPortInfo.portName());
        ui->serialSelectorCombobox->addItem(serialPortInfo.portName() + " - " + serialPortInfo.description(), comport);
        if(serialPortInfo.description().contains("CP210x", Qt::CaseInsensitive) || serialPortInfo.description().contains("CH340", Qt::CaseInsensitive)) {
            esp32Index = currentIndex;
        }
        currentIndex++;
    }
    ui->serialSelectorCombobox->setCurrentIndex(esp32Index > -1 ? esp32Index : 0);
    on_serialSelectorCombobox_indexChanged(esp32Index);
    connect(ui->serialSelectorCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_serialSelectorCombobox_indexChanged);
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
    flashFirmware(esptool, firmwareBinPath, comport);
}

void MainWindow::on_saveWiFiCredsButton_clicked()
{
    auto ssid = ui->wifiSSIDInput->text();
    auto password = ui->wifiPassInput->text();
    bool modified = false;
    if(!checkAndConnectSerial()) {
        return;
    }
    if(ssid.contains(" ")) {
        QMessageBox::critical(this, tr("Invalid form"),
                              tr("Sorry, but in the current version of TCode spaces are not allowed in commands.\nPlease use the web ui in AP mode (see PDF section 'AP configuration')\nto enter your SSID with spaces.\nThis should be fixed in the next version of TCode."));
        return;
    }
//    QRegularExpression ssidRegex;
//    ssidRegex.setPattern(R"(^[^!#;+\]\/"\t][^+\]\/"\t]{0,30}[^ +\]\/"\t]$|^[^ !#;+\]\/"\t]$[ \t]+$)");
//    if(ssidRegex.match(ssid).hasMatch()) {
//        QMessageBox::critical(this, tr("Invalid form"),
//                              tr("SSID has invalid characters"));
//    }
    if(!ssid.isEmpty()) {
        sendSerial("#wifi-ssid:"+ssid);
        modified = true;
    }
    if(!password.isEmpty()) {
        sendSerial("#wifi-pass:"+password);
        modified = true;
    }
    if(modified) {
        sendSerial("$save #restart");
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
        ui->serialOutputTextBrowser->append("Nothing to send!");
}

void MainWindow::on_serialCommandInput_returnPressed()
{
    on_sendSerialButton_clicked();
}

void MainWindow::on_serialSelectorCombobox_indexChanged(int index)
{
    checkAndConnectSerial();
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

