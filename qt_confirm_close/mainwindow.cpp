#include "mainwindow.h"

#include <QCloseEvent>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 16, 16, 16);

    auto *label = new QLabel(tr("点击窗口的关闭按钮时，会弹出确认对话框。"), central);
    label->setWordWrap(true);
    layout->addWidget(label);

    setCentralWidget(central);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    const QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        tr("确认退出"),
        tr("确定要退出应用吗？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (choice == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}
