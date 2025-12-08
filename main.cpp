#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("课程管理系统");
    app.setApplicationVersion("3.0");
    app.setOrganizationName("EducationSoft");

    // 设置全局字体
    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    MainWindow window;
    window.setWindowTitle("课程管理系统 v3.0 - 完整功能版");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
