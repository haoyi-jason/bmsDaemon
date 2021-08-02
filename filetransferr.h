#ifndef FILETRANSFERR_H
#define FILETRANSFERR_H

#include <QObject>
#include <QDebug>
class QTcpSocket;

class FileTransferr : public QObject
{
    Q_OBJECT
public:
    explicit FileTransferr(QObject *parent = nullptr);
    void sendFile(qintptr descriptor, QString fname);
    void abort();

    void acked(){qDebug()<<Q_FUNC_INFO;m_acked = true;}
    void nacked(){m_nacked = true;}
    bool isProcessing(){return m_processing;}
private slots:
    void handleSocketDisconnected();
signals:
    void finished(bool);
    void aborted();
    void progressBase(int);
    void progress(int);
    void write(QByteArray b);

public slots:

private:
    bool m_abort = false;
    QTcpSocket *m_socket = nullptr;
    bool m_acked = false;
    bool m_nacked = false;
    bool m_processing = false;
};

#endif // FILETRANSFERR_H
