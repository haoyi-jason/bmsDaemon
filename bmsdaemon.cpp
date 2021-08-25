#include "bmsdaemon.h"
#include <QtNetwork>
#include <QObject>
#include "dgsocketconnected.h"

bmsDaemon::bmsDaemon()
{
    // check for search path
    QFile f("/opt/bms/config/searchpath");
    if(!f.exists()){
        f.open(QIODevice::ReadWrite);
        f.write("/mnt/t");
        f.close();
    }

}

bmsDaemon::~bmsDaemon()
{
    if(m_server != nullptr){
        m_server->close();
        m_server->deleteLater();
    }
    foreach (tcpClient *t, m_client) {
        t->deleteLater();
    }

}

void bmsDaemon::startServer(int port)
{
    if(m_server != nullptr){
        m_server->disconnect();
        m_server->deleteLater();
        m_server = nullptr;
    }
    m_server = new QTcpServer();
    m_server->listen(QHostAddress::Any,port);
    if(m_server->isListening()){
        connect(m_server,&QTcpServer::newConnection,this,&bmsDaemon::handleNewConnection);
    }
}

void bmsDaemon::handleNewConnection()
{
    QTcpServer *server = static_cast<QTcpServer*>(sender());
    tcpClient *client = new tcpClient;
    client->socket = server->nextPendingConnection();
    //client->socket->write("Welcome");
    //client->socketDescriptor = server->nextPendingConnection()->socketDescriptor();
    //connect(client->socket,&QTcpSocket::readyRead,this,&bmsDaemon::handleDataReceived);
    //connect(client->socket,&QTcpSocket::disconnected,this,&bmsDaemon::handleDisconnected);
    //m_client.append(client);

    //m_dgSocketConnect = new dgSocketConnected;
   // m_dgSocketConnect->exec();
//    if(m_frmSocket != nullptr){
//        m_frmSocket->close();
//        //delete m_frmSocket;
//        m_frmSocket = nullptr;
//    }
    m_frmSocket = new frmSocketConnection;
    m_frmSocket->setAttribute(Qt::WA_DeleteOnClose,true);
    m_frmSocket->setClient(client);
    m_frmSocket->show();

}

void bmsDaemon::handleDataReceived()
{
    QTcpSocket *s = static_cast<QTcpSocket*>(sender());
    QStringList sl;
    foreach (tcpClient *c, m_client) {
        if(c->socket == s){
            switch(c->state){
            case tcpClient::CS_IDLE:
                sl = QString(s->readAll()).split(" ");
                switch(cmd_map.value(sl[0])){
                case 0:
                    c->readName = sl[1];
                    c->state = tcpClient::CS_FILE_READ;
                    break;
                case 1:
                    c->writeName = sl[1];
                    c->state = tcpClient::CS_FILE_WRITE;
                    break;
                case 2:
                    break;
                default:
                    break;
                }
                break;
            case tcpClient::CS_FILE_READ:
                break;
            case tcpClient::CS_FILE_WRITE:
                break;
            default: break;
            }
        }
    }
}

void bmsDaemon::handleDisconnected()
{
    QTcpSocket *s = static_cast<QTcpSocket*>(sender());
    foreach (tcpClient *c, m_client) {
        if(c->socket == s){
            c->socket->disconnect();
            c->socket->deleteLater();
            c->socket = nullptr;
            c->data.clear();
            m_client.removeOne(c);
        }
    }
}

void bmsDaemon::broadCast()
{

}

void bmsDaemon::debug(QString info)
{

}

void bmsDaemon::log(QString info)
{

}
