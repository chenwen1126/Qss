#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QssMainWindow(parent),//step 3
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/QssDemo/app.png"));
    setWindowTitle("QssDemo");

    statusBar()->addWidget(new QLabel("  Ready  "));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    warnBox("<br><h4>this is a warnBox custom by Qss<\\h4><br>");
}
