#ifndef FRMSOCKETCONNECTION_H
#define FRMSOCKETCONNECTION_H

#include <QWidget>
#include <QThread>
#include <QTcpSocket>
#include <QFileInfo>

#include "filetransferr.h"


namespace Ui {
class frmSocketConnection;
}

class tcpClient;
class QFileInfo;
//const QMap<QString,int> cmd_map = {{"FREAD",1},{"FWRITE",2},{"RESTART",2}};

class TransferThread:public QThread
{

};

class frmSocketConnection : public QWidget
{
    Q_OBJECT

public:
    explicit frmSocketConnection(QWidget *parent = nullptr);
    ~frmSocketConnection();
    void setClient(tcpClient *c);

private slots:
    void handleSockeRead();
    void handleSockeDisconnect();
    void handleFinished(bool);
    void handleAborted();
    void handleFtWrite(QByteArray b);

    void on_pushButton_clicked();

    void transferFile(QString path);
    void sendFile();
    void recvFile(QByteArray b);

    void handleProgressBase(int value);
    void handleProgress(int value);

    void upgradeBMS(int options);

private:
    Ui::frmSocketConnection *ui;

    tcpClient *m_client = nullptr;

    FileTransferr *m_ft = nullptr;

    int m_filesToTransfer;
    int m_fileTransferred;
    QList<QFileInfo> m_fileInfo;
    QTcpSocket *m_activeSocket = nullptr;
    QThread m_workThread;

    QString m_fileToSend="";
    QString m_fileToReceive = "";
    quint64 m_sendCount;
    quint64 m_fileSize;
    quint64 m_bytesRead;
    quint64 m_recvFileSize = 0;
    quint64 m_recvdFileSize = 0;
    quint32 m_remoteAddress;
};

#endif // FRMSOCKETCONNECTION_H
