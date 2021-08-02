#include "dgsocketconnected.h"
#include "ui_dgsocketconnected.h"

dgSocketConnected::dgSocketConnected(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dgSocketConnected)
{
    ui->setupUi(this);
}

dgSocketConnected::~dgSocketConnected()
{
    delete ui;
}
