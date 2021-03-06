#include "frmsocketconnection.h"
#include "ui_frmsocketconnection.h"
#include "bmsdaemon.h"

#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QHostAddress>
#include <QProcess>
#include <QSysInfo>
#include <QStringList>


frmSocketConnection::frmSocketConnection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmSocketConnection)
{
    ui->setupUi(this);
    m_ft = new FileTransferr();

    //QThread workThread;
   // connect(&m_workThread,&QThread::finished,m_ft,&FileTransferr::deleteLater);
//    connect(m_ft,&FileTransferr::finished,this,&frmSocketConnection::handleFinished);
//    connect(m_ft,&FileTransferr::aborted,this,&frmSocketConnection::handleAborted);
//    connect(m_ft,&FileTransferr::write,this,&frmSocketConnection::handleFtWrite);
//    connect(m_ft,&FileTransferr::progressBase,this,&frmSocketConnection::handleProgressBase);
//    connect(m_ft,&FileTransferr::progress,this,&frmSocketConnection::handleProgress);
//    m_ft->moveToThread(&m_workThread);
    if(QSysInfo::productType().contains("win")){
        m_isLinux = false;
    }
    else{
        m_isLinux = true;
    }


}

frmSocketConnection::~frmSocketConnection()
{
    m_workThread.terminate();
    m_workThread.wait();
    delete ui;
}

void frmSocketConnection::setClient(tcpClient *c)
{
    m_filesToTransfer = 1;
    m_fileTransferred = 0;
    m_client = c;
    m_client->state = tcpClient::CS_IDLE;

    connect(m_client->socket,&QTcpSocket::readyRead,this,&frmSocketConnection::handleSockeRead);
    connect(m_client->socket,&QTcpSocket::disconnected,this,&frmSocketConnection::handleSockeDisconnect);

    // loop through each file
    //m_ft->start(m_activeSocket, "D:/temp/bms/log/S_01_20210714_12.csv");
    //m_client->socket->write("Welcome 2");

    m_remoteAddress = m_client->socket->peerAddress().toIPv4Address();
//    qDebug()<<"Remote Address:"<<QString::number(m_remoteAddress,16);
    QStringList sl = m_client->socket->peerAddress().toString().split(":");
    QString msg = "Incoming Connection from:\n" + sl[sl.size()-1]; //m_client->socket->peerAddress().toString();
    ui->lbInfo->setText(msg);
}

void frmSocketConnection::handleFtWrite(QByteArray b)
{
   m_client->socket->write(b);
}

void frmSocketConnection::handleSockeRead()
{
    QStringList sl;
    //m_client->socket->write("Hello");
    QByteArray recv = m_client->socket->readAll();
    //QString logPath = "";
    switch(m_client->state){
    case tcpClient::CS_IDLE:
        sl = QString(recv).split(" ");
        //qDebug()<<Q_FUNC_INFO<<sl;
        switch(cmd_map.value(sl[0])){
        case 0: // ls
        {
            QFile f("/opt/bms/config/searchpath");
            if(f.open(QIODevice::ReadWrite)){
                m_logPath = QString(f.readLine());
                f.close();
                m_logPath = m_logPath.trimmed();
            }
            if(m_logPath == ""){
                m_logPath = "/mnt/t";
            }


            //qDebug()<<"Search Path:"<<logPath;
            QFileInfoList flist;
            if(m_isLinux){
                flist = QDir(m_logPath).entryInfoList(QStringList()<<"*.csv",QDir::Files | QDir::NoDotAndDotDot, QDir::Reversed);
                //flist = QDir("/opt/bms/temp/log").entryInfoList(QStringList()<<"*.csv",QDir::Files | QDir::NoDotAndDotDot, QDir::Reversed);
            }else{
                flist = QDir("d:/temp/bms/log").entryInfoList(QStringList()<<"*.csv",QDir::Files | QDir::NoDotAndDotDot, QDir::Reversed);
            }

            //qDebug()<<"File list:"<<flist.size();

            if(flist.size() > 0){
                foreach (QFileInfo fi, flist) {
                    QString msg = QString("%1;%2\n").arg(fi.fileName()).arg(fi.size());
                    m_client->socket->write(msg.toUtf8());
                }
                // copy events.log to temporary file
                QProcess proc;
                proc.execute("/bin/sh -c \"cp /opt/bms/log/sys/events.log /opt/bms/log/sys/event_t.log\"");
                proc.waitForFinished();
                // send event log
                QFileInfo finfo("/opt/bms/log/sys/event_t.log");
                QString msg = QString("%1;%2\n").arg("events.log").arg(finfo.size());
                m_client->socket->write(msg.toUtf8());

                proc.execute("/bin/sh -c \"cp /opt/bms/log/record/eventLog.csv /opt/bms/log/record/eventLog_t.csv\"");
                proc.waitForFinished();
                QFileInfo eventLog("/opt/bms/log/record/eventLog_t.csv");
                msg = QString("%1;%2\n").arg("eventLog.csv").arg(eventLog.size());
                m_client->socket->write(msg.toUtf8());

                QFileInfo config("/opt/bms/config/controller.json");
                msg = QString("%1;%2\n").arg(config.fileName()).arg(config.size());
                m_client->socket->write(msg.toUtf8());
            }
            else{
                QString msg = "NULL;0\n";
                m_client->socket->write(msg.toUtf8());
            }
        }
            break;
        case 1:
            //qDebug()<<"Read File:"+sl[1];
            // check if request for event log file
            if(sl[1] == "eventLog.csv"){
                if(m_isLinux){
                    m_fileToSend = "/opt/bms/log/record/eventLog_t.csv";
                }
                else{
                    m_fileToSend = "d:/temp/bms/log/record/eventLog.csv";
                }
            }
            else if(sl[1] == "events.log"){
                if(m_isLinux){
                    m_fileToSend = "/opt/bms/log/sys/event_t.log";
                }
                else{
                    m_fileToSend = "d:/temp/bms/log/record/sys/event_t.log";
                }
            }
            else if(sl[1] == "controller.json"){
                m_fileToSend = "/opt/bms/config/controller.json";
            }
            else{
                if(m_isLinux){
                    m_fileToSend = m_logPath+"/"+sl[1];
                }
                else{
                    m_fileToSend = "d:/temp/bms/log/"+sl[1];
                }
            }
            m_sendCount = 0;
            m_bytesRead = 0;
            ui->lbFileName->setText("Sending File:"+sl[1]);
            qDebug()<<"Send file:"<<m_fileToSend;
            sendFile();
            break;
        case 2: // write
            m_fileToReceive = "";
            recvFile(recv);
            //m_client->writeName = sl[1];
           // m_client->state = tcpClient::CS_FILE_WRITE;
            break;
        case 3: // restart
            break;
        case 4: // ack
            m_sendCount += m_bytesRead;
            if(m_sendCount < m_fileSize){
                sendFile();
            }
            else{
                ui->pbCurrentFile->setValue(m_sendCount);
            }
            break;
        case 5: // nak

            break;
        case 6: // upgrade
            upgradeBMS(3);
            break;
        case 7: // txdelay
            if(sl.size() == 2){
                int delay = sl[1].toInt();
                if(delay < 0) delay = 10;
                if(delay > 50) delay = 50;
                if(delay == 0){
                    m_limitTransfer = false;
                }
                else{
                    m_limitTransfer = true;
                    m_transferDelay = delay;
                }
            }
            break;
        default:
            break;
        }
        break;
    case tcpClient::CS_FILE_READ:
        break;
    case tcpClient::CS_FILE_WRITE:
        recvFile(recv);
        break;
    default: break;
    }
}

void frmSocketConnection::handleSockeDisconnect()
{
    if(m_remoteAddress != 0x7f000001){
        this->deleteLater();
    }
}

void frmSocketConnection::handleFinished(bool result)
{
    qDebug()<<Q_FUNC_INFO;
    m_client->state == tcpClient::CS_IDLE;
}

void frmSocketConnection::handleAborted()
{
    qDebug()<<Q_FUNC_INFO;

}



void frmSocketConnection::on_pushButton_clicked()
{
    //m_client->socket->write("test");
    handleFtWrite(QByteArray::fromStdString("test2"));
}

void frmSocketConnection::transferFile(QString path)
{
    QFile f(path);
    if(f.exists() && f.open(QIODevice::ReadOnly)){
        quint64 fsize = f.size();
        quint64 byteRead = 0;
        QByteArray b;
        //b.append(QByteArray::number(fsize));
        //handleFtWrite(b);
        ui->pbCurrentFile->setMaximum(fsize);
        ui->pbCurrentFile->setValue(0);
        while(!f.atEnd()){
            b = f.read(1500);
            byteRead += b.size();
            handleFtWrite(b);
            ui->pbCurrentFile->setValue(byteRead);
            //progress(byteRead/fsize);
            QEventLoop evLoop;
            QTimer::singleShot(1,&evLoop,&QEventLoop::quit);
            evLoop.exec();

        }
        qDebug()<<"Finish file transfer";
        m_client->state = tcpClient::CS_IDLE;
        if(byteRead == fsize){
            //emit finished(true);
            m_client->state = tcpClient::CS_IDLE;
        }
        else {
            //emit finished(false);
        }


    }
}

void frmSocketConnection::sendFile()
{
    QFile f(m_fileToSend);
    if(f.exists() && f.open(QIODevice::ReadOnly)){
        if(m_sendCount == 0){
            m_fileSize = f.size();
            ui->pbCurrentFile->setMaximum(m_fileSize);
        }
        f.seek(m_sendCount);
        quint64 fsize = f.size();
        quint64 byteRead = 0;
        QByteArray b;
        //b.append(QByteArray::number(fsize));
        //handleFtWrite(b);
        ui->pbCurrentFile->setValue(m_sendCount);
        m_bytesRead = (m_fileSize-m_sendCount)<1500?(m_fileSize - m_sendCount):1500;
        b = f.read(m_bytesRead);
        handleFtWrite(b);
        if(m_limitTransfer)
            QThread::msleep(m_transferDelay);
    }
    f.close();
}

void frmSocketConnection::recvFile(QByteArray b)
{
    if(m_fileToReceive == ""){
        QStringList sl = QString(b).split(" ");
        if(sl.size() == 3){
            qDebug()<<"Start file receive";
            if(m_isLinux){
                m_fileToReceive = "/opt/bms/temp/"+sl[1];
                QDir dir;
                if(!dir.exists("/opt/bms/temp/")){
                    dir.mkpath("/opt/bms/temp/");
                }
            }
            else{
                m_fileToReceive = "d:/temp/bms/temp/"+sl[1];
                QDir dir;
                if(!dir.exists("d:/temp/bms/temp/")){
                    dir.mkpath("d:/temp/bms/temp/");
                }
            }
            m_recvFileSize = sl[2].toInt();
            m_recvdFileSize = 0;
            m_client->state = tcpClient::CS_FILE_WRITE;
            handleFtWrite(QString("ack").toUtf8());
            ui->pbCurrentFile->setMaximum(m_recvFileSize);
        }
    }
    else{
        QFile f(m_fileToReceive);
        if(f.open(QIODevice::ReadWrite | QIODevice::Append)){
            f.write(b);
            f.close();
            m_recvdFileSize += b.size();
            qDebug()<<"\rByte Received:"<<m_recvdFileSize<<"/"<<m_recvFileSize;
            ui->pbCurrentFile->setValue(m_recvdFileSize);
            if(m_recvdFileSize == m_recvFileSize){
                qDebug()<<"Finish file transfer";
                m_client->state = tcpClient::CS_IDLE;
            }
            QThread::msleep(5);
            handleFtWrite(QString("ack").toUtf8());
        }
    }
}

void frmSocketConnection::handleProgressBase(int value)
{
    ui->pbCurrentFile->setMaximum(value);
}

void frmSocketConnection::handleProgress(int value)
{
    ui->pbCurrentFile->setValue(value);
}

void frmSocketConnection::replaceConfigFile()
{
    QProcess proc;

    QString src, dst;
    if(QSysInfo::productType().contains("win")){
        src = "d:/temp/bms/temp/controller.json";
        dst = "d:/temp/bms/temp/controller_new.json";
    }
    else{
        src = "/opt/bms/temp/controller.json";
        dst = "/opt/bms/config/controller.json";
    }

    QString cmd;
    // check file existance
    if(QFile(src).exists()){
        // remove old file
        cmd = QString("rm %1").arg(dst);
        proc.execute(cmd);
        proc.waitForFinished();
        // copy file
        cmd = QString("mv %1 %2").arg(src).arg(dst);
        proc.execute(cmd);
        proc.waitForFinished();
    }
}

void frmSocketConnection::upgradeBMS(int options)
{
    QProcess proc;

    QString c_src, u_src;
    QString c_dst, u_dst;
    if(QSysInfo::productType().contains("win")){
        c_src = "d:/temp/bms/temp/BMS_Controller";
        u_src = "d:/temp/bms/temp/BMS_HY01";
        c_dst = "d:/temp/bms/bin/BMS_Controller";
        u_dst = "d:/temp/bms/bin/BMS_HY01";
    }
    else{
        c_src = "/opt/bms/temp/BMS_Controller";
        u_src = "/opt/bms/temp/BMS_HY01";
        c_dst = "/opt/BMS_Controller/bin/BMS_Controller";
        u_dst = "/opt/BMS_HY01/bin/BMS_HY01";
    }

    // stop programs
    if(QSysInfo::productType().contains("win")){
        qDebug()<<"systemctl stop bms_controller";
        qDebug()<<"systemctl stop bms_ui";
    }
    else{
//        proc.execute("systemctl stop bms_controller");
//        proc.waitForFinished();
//        proc.execute("systemctl stop bms_ui");
//        proc.waitForFinished();
    }
    QString cmd;
    if(options == 1){ // bms controller only
        // check file existance
        if(QFile(c_src).exists()){
            proc.execute("systemctl stop bms_controller");
            proc.waitForFinished();
            // remove BMS_Controlle
            cmd = QString("rm %1").arg(c_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // copy file
            cmd = QString("mv %1 %2").arg(c_src).arg(c_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // change executable
            cmd = QString("chmod +x %1").arg(c_dst);
            proc.execute(cmd);
            proc.waitForFinished();
        }

    }
    else if(options == 2){ // bms ui only
        if(QFile(u_src).exists()){
            proc.execute("systemctl stop bms_ui");
            proc.waitForFinished();
            // remove BMS_UI
            cmd = QString("rm %1").arg(u_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // cop file
            cmd = QString("mv %1 %2").arg(u_src).arg(u_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // change executable
            cmd = QString("chmod +x %1").arg(u_dst);
            proc.execute(cmd);
            proc.waitForFinished();
        }

    }
    else if(options == 3){ // both
        if(QFile(c_src).exists()){
            proc.execute("systemctl stop bms_controller");
            proc.waitForFinished();
            // remove BMS_Controlle
            cmd = QString("rm %1").arg(c_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // cop file
            cmd = QString("mv %1 %2").arg(c_src).arg(c_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // change executable
            cmd = QString("chmod +x %1").arg(c_dst);
            proc.execute(cmd);
            proc.waitForFinished();
        }
        if(QFile(u_src).exists()){
            proc.execute("systemctl stop bms_ui");
            proc.waitForFinished();
            // remove BMS_UI
            cmd = QString("rm %1").arg(u_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // cop file
            cmd = QString("mv %1 %2").arg(u_src).arg(u_dst);
            proc.execute(cmd);
            proc.waitForFinished();
            // change executable
            cmd = QString("chmod +x %1").arg(u_dst);
            proc.execute(cmd);
            proc.waitForFinished();
        }
    }

//    // unlink source
//    cmd = QString("rm %1").arg(c_src);
//    proc.execute(cmd);
//    proc.waitForFinished();
//    cmd = QString("rm %1").arg(u_src);
//    proc.execute(cmd);
//    proc.waitForFinished();

    // start programs
    if(QSysInfo::productType().contains("win")){
        qDebug()<<"systemctl start bms_controller";
        qDebug()<<"systemctl start bms_ui";
    }
    else{
        proc.execute("systemctl start bms_controller");
        proc.waitForFinished();
        proc.execute("systemctl start bms_ui");
        proc.waitForFinished();
    }
}

void frmSocketConnection::on_pushButton_2_clicked()
{

}

void frmSocketConnection::on_pbUpdateController_clicked()
{
    upgradeBMS(1);
}

void frmSocketConnection::on_pbRestartController_clicked()
{
    QProcess proc;
    // start programs
    if(QSysInfo::productType().contains("win")){
        qDebug()<<"systemctl restart bms_controller";
//        qDebug()<<"systemctl restart bms_ui";
    }
    else{
        proc.execute("systemctl restart bms_controller");
        proc.waitForFinished();
//        proc.execute("systemctl restart bms_ui");
//        proc.waitForFinished();
    }
}

void frmSocketConnection::on_pbUpdateUI_clicked()
{
    upgradeBMS(2);
}

void frmSocketConnection::on_pbRestartUI_clicked()
{
    QProcess proc;
    // start programs
    if(QSysInfo::productType().contains("win")){
//        qDebug()<<"systemctl restart bms_controller";
        qDebug()<<"systemctl restart bms_ui";
    }
    else{
//        proc.execute("systemctl restart bms_controller");
//        proc.waitForFinished();
        proc.execute("systemctl restart bms_ui");
        proc.waitForFinished();
    }
}

void frmSocketConnection::on_pbUpdateConfig_clicked()
{
    replaceConfigFile();
}
