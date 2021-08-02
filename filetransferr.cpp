#include "filetransferr.h"
#include <QTcpSocket>
#include <QFile>
#include <QDataStream>
#include <QtCore>

FileTransferr::FileTransferr(QObject *parent) : QObject(parent)
{

    m_socket = new QTcpSocket;
}

void FileTransferr::sendFile(qintptr descriptor, QString fname)
{
    QFile f(fname);
    //m_socket->setSocketDescriptor(descriptor);
    if(f.exists() && f.open(QIODevice::ReadOnly)){
        m_processing = true;
        //connect(s,&QTcpSocket::disconnected,this,&FileTransferr::handleSocketDisconnected);
        quint64 fsize = f.size();
        quint64 byteRead = 0;
        QByteArray b;
        //b.append(QByteArray::number(fsize));
        //emit write(b);
        emit progressBase(fsize);
        m_acked = true;
        while(!f.atEnd() && !m_abort){
            if(m_acked){
                b = f.read(1000);
                byteRead += b.size();
            }
            emit write(b);
            emit progress(byteRead);
            int cntr = 0;
            m_acked = m_nacked = false;
            while(!m_acked || !m_nacked || (cntr < 1000)){
                QEventLoop evLoop;
                QTimer::singleShot(10,&evLoop,&QEventLoop::quit);
                evLoop.exec();
            }
        }
        m_processing = false;
        if(byteRead == fsize){
            emit finished(true);
        }
        else {
            emit finished(false);
        }

        if(m_abort){
            emit aborted();
        }
    }
}

void FileTransferr::abort()
{
    m_abort = true;
}

void FileTransferr::handleSocketDisconnected()
{
    m_abort = true;
}
