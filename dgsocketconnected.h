#ifndef DGSOCKETCONNECTED_H
#define DGSOCKETCONNECTED_H

#include <QDialog>

namespace Ui {
class dgSocketConnected;
}

class dgSocketConnected : public QDialog
{
    Q_OBJECT

public:
    explicit dgSocketConnected(QWidget *parent = nullptr);
    ~dgSocketConnected();

private:
    Ui::dgSocketConnected *ui;
};

#endif // DGSOCKETCONNECTED_H
