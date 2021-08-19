#ifndef BMSDAEMON_H
#define BMSDAEMON_H

#include <QObject>
#include <QThread>
#include "frmsocketconnection.h"

class QTcpServer;
class QTcpSocket;
class QTimer;
class QUdpSocket;

class dgSocketConnected;

const QMap<QString,int> cmd_map = {{"ls",0},{"read",1},{"write",2},{"restart",3},{"ack",4},{"nak",5},{"upgrade",6},{"txdelay",7}};



class tcpClient:public QObject
{
    Q_OBJECT
public:
    explicit tcpClient(){}
    ~tcpClient(){};
    enum CLIENT_STATE{
        CS_IDLE,
        CS_FILE_READ,
        CS_FILE_WRITE,
    };
    Q_ENUM(CLIENT_STATE)

    QTcpSocket *socket = nullptr;
    QByteArray data;
    CLIENT_STATE state = CS_IDLE;
    QString writeName;
    QString readName;
    qintptr socketDescriptor;
};

class bmsDaemon:public QObject
{
    Q_OBJECT
public:
    bmsDaemon();
    ~bmsDaemon();
    void startServer(int port);

private:
    void debug(QString info);
    void log(QString info);

private slots:
    void handleDataReceived();
    void handleDisconnected();
    void handleNewConnection();
    void broadCast();

private:
    QTcpServer *m_server = nullptr;
    QList<tcpClient*> m_client;
    dgSocketConnected *m_dgSocketConnect = nullptr;
    frmSocketConnection *m_frmSocket = nullptr;
};

#endif // BMSDAEMON_H
