
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QTextEdit>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QEasingCurve>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QFrame>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_courseManager(new CourseManager(this))
    , m_clockTimer(new QTimer(this))
    , m_courseTable(nullptr)
    , m_weekLabel(nullptr)
    , m_clockLabel(nullptr)
    , m_searchEdit(nullptr)
    , m_searchBtn(nullptr)
    , m_exportBtn(nullptr)
    , m_backupBtn(nullptr)
{
    ui->setupUi(this);

    // 设置当前周为当前日期所在的周
    QDate today = QDate::currentDate();
    m_currentWeekStart = today.addDays(1 - today.dayOfWeek()); // 周一开始

    setupUI();
    applyStyles();
    updateWeekDisplay();
    populateCourseTable();

    // 连接时钟定时器
    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start(1000); // 每秒更新一次
    updateClock(); // 立即更新一次

    // 初始淡入效果
    fadeInWidget(ui->centralwidget);
    onRefresh();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 使用现有的centralwidget
    QWidget *centralWidget = ui->centralwidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 标题栏
    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("课程管理系统", this);
    titleLabel->setObjectName("titleLabel");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    // 时钟标签
    m_clockLabel = new QLabel(this);
    m_clockLabel->setObjectName("clockLabel");
    titleLayout->addWidget(m_clockLabel);

    mainLayout->addLayout(titleLayout);

    // 搜索栏
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索课程名称、教师或地点...");
    m_searchEdit->setObjectName("searchEdit");

    m_searchBtn = new QPushButton("搜索", this);
    m_searchBtn->setObjectName("searchButton");
    // 设置圆角样式
    m_searchBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);"
        "    color: white;"
        "    border: none;"
        "    padding: 10px 20px;"
        "    border-radius: 20px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2980b9, stop:1 #2471a3);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2471a3, stop:1 #1f618d);"
        "}"
        );
    connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::onSearch);

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);

    mainLayout->addLayout(searchLayout);

    // 周导航
    QHBoxLayout *weekNavLayout = new QHBoxLayout();

    // 导航按钮通用样式
    QString navButtonStyle =
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 20px;"
        "    border-radius: 20px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2980b9, stop:1 #2471a3);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2471a3, stop:1 #1f618d);"
        "}";

    QString todayButtonStyle =
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b);"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 20px;"
        "    border-radius: 20px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #c0392b, stop:1 #a93226);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #a93226, stop:1 #922b21);"
        "}";

    QPushButton *prevBtn = new QPushButton("← 上一周", this);
    prevBtn->setObjectName("navButton");
    prevBtn->setStyleSheet(navButtonStyle);
    connect(prevBtn, &QPushButton::clicked, this, &MainWindow::prevWeek);

    m_weekLabel = new QLabel(this);
    m_weekLabel->setObjectName("weekLabel");
    m_weekLabel->setAlignment(Qt::AlignCenter);

    QPushButton *nextBtn = new QPushButton("下一周 →", this);
    nextBtn->setObjectName("navButton");
    nextBtn->setStyleSheet(navButtonStyle);
    connect(nextBtn, &QPushButton::clicked, this, &MainWindow::nextWeek);

    QPushButton *todayBtn = new QPushButton("本周", this);
    todayBtn->setObjectName("todayButton");
    todayBtn->setStyleSheet(todayButtonStyle);
    connect(todayBtn, &QPushButton::clicked, this, &MainWindow::goToThisWeek);

    weekNavLayout->addWidget(prevBtn);
    weekNavLayout->addWidget(m_weekLabel);
    weekNavLayout->addWidget(nextBtn);
    weekNavLayout->addWidget(todayBtn);

    mainLayout->addLayout(weekNavLayout);

    // 课程表格
    m_courseTable = new QTableWidget(this);
    m_courseTable->setObjectName("courseTable");
    m_courseTable->setColumnCount(8); // 时间 + 周一至周日
    m_courseTable->setRowCount(10);   // 10节课

    QStringList headers;
    headers << "时间" << "周一" << "周二" << "周三" << "周四" << "周五" << "周六" << "周日";
    m_courseTable->setHorizontalHeaderLabels(headers);

    // 设置时间列
    QStringList timeSlots = {
        "08:00-08:45", "08:55-09:40", "09:50-10:35", "10:45-11:30",
        "14:00-14:45", "14:55-15:40", "15:50-16:35", "16:45-17:30",
        "19:00-19:45", "19:55-20:40"
    };

    for (int i = 0; i < timeSlots.size(); ++i) {
        QTableWidgetItem *timeItem = new QTableWidgetItem(timeSlots[i]);
        timeItem->setTextAlignment(Qt::AlignCenter);
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
        m_courseTable->setItem(i, 0, timeItem);
    }

    // 关键修改：取消 Stretch 模式，使用 Interactive
    m_courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_courseTable->verticalHeader()->setVisible(false);
    m_courseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_courseTable->setSelectionBehavior(QAbstractItemView::SelectItems);

    // 设置足够宽的列宽，确保内容完全显示
    m_courseTable->setColumnWidth(0, 120);   // 时间列
    for (int col = 1; col < m_courseTable->columnCount(); ++col) {
        m_courseTable->setColumnWidth(col, 160);  // 课程列 - 增加到200像素
    }

    // 设置足够的行高
    for (int row = 0; row < m_courseTable->rowCount(); ++row) {
        m_courseTable->setRowHeight(row, 80);  // 增加到100像素
    }

    // 关键设置：确保内容完全显示
    m_courseTable->setWordWrap(true);              // 允许文本换行
    m_courseTable->setTextElideMode(Qt::ElideNone); // 禁止文本截断
    m_courseTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 允许水平滚动

    connect(m_courseTable, &QTableWidget::cellClicked, this, &MainWindow::showCourseDetails);

    mainLayout->addWidget(m_courseTable);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 操作按钮通用样式
    QString actionButtonStyle =
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ecc71, stop:1 #27ae60);"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 20px;"
        "    border-radius: 20px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    min-width: 100px;"
        "    margin: 5px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #27ae60, stop:1 #229954);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #229954, stop:1 #1e8449);"
        "}";

    QPushButton *addBtn = new QPushButton("添加课程", this);
    addBtn->setObjectName("actionButton");
    addBtn->setStyleSheet(actionButtonStyle);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddCourse);

    QPushButton *editTimeBtn = new QPushButton("编辑时间段", this);
    editTimeBtn->setObjectName("actionButton");
    editTimeBtn->setStyleSheet(actionButtonStyle);
    connect(editTimeBtn, &QPushButton::clicked, this, &MainWindow::onEditTimeSlots);

    QPushButton *refreshBtn = new QPushButton("刷新", this);
    refreshBtn->setObjectName("actionButton");
    refreshBtn->setStyleSheet(actionButtonStyle);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefresh);

    m_exportBtn = new QPushButton("导出数据", this);
    m_exportBtn->setObjectName("actionButton");
    m_exportBtn->setStyleSheet(actionButtonStyle);
    connect(m_exportBtn, &QPushButton::clicked, this, &MainWindow::onExport);

    m_backupBtn = new QPushButton("备份数据", this);
    m_backupBtn->setObjectName("actionButton");
    m_backupBtn->setStyleSheet(actionButtonStyle);
    connect(m_backupBtn, &QPushButton::clicked, this, &MainWindow::onBackup);

    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(editTimeBtn);
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addWidget(m_exportBtn);
    buttonLayout->addWidget(m_backupBtn);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
}

void MainWindow::applyStyles()
{
    // 应用CSS样式
    QString styleSheet = R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #f5f7fa, stop:1 #c3cfe2);
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }

        #titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #2c3e50;
            padding: 10px;
        }

        #clockLabel {
            font-size: 16px;
            color: #7f8c8d;
            padding: 10px;
            background: rgba(255,255,255,0.8);
            border-radius: 10px;
        }

        #searchEdit {
            padding: 8px 12px;
            border: 2px solid #bdc3c7;
            border-radius: 20px;
            font-size: 14px;
            background: white;
        }

        #searchEdit:focus {
            border-color: #3498db;
            outline: none;
        }

        #searchButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            padding: 8px 20px;
            border-radius: 20px;
            font-weight: bold;
        }

        #searchButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2980b9, stop:1 #2471a3);
        }

        #weekLabel {
            font-size: 18px;
            font-weight: bold;
            color: #34495e;
            padding: 15px;
            background: white;
            border-radius: 15px;
            margin: 5px;
        }

        #navButton, #todayButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 20px;
            font-weight: bold;
            min-width: 100px;
        }

        #navButton:hover, #todayButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2980b9, stop:1 #2471a3);
        }

        #todayButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
        }

        #todayButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #c0392b, stop:1 #a93226);
        }

        #actionButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2ecc71, stop:1 #27ae60);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 20px;
            font-weight: bold;
            font-size: 14px;
            margin: 5px;
        }

        #actionButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #27ae60, stop:1 #229954);
        }

        #courseTable {
            background: white;
            border-radius: 15px;
            gridline-color: #bdc3c7;
            font-size: 12px;
        }

        #courseTable::item {
            padding: 8px;
            border: none;
        }

        #courseTable::item:selected {
            background: rgba(52, 152, 219, 0.3);
        }

        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #34495e, stop:1 #2c3e50);
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
        }

        .course-cell {
            border-radius: 8px;
            margin: 2px;
            font-weight: bold;
            color: white;
        }
    )";

    setStyleSheet(styleSheet);
}

void MainWindow::updateClock()
{
    if (!m_clockLabel) return;

    QDateTime current = QDateTime::currentDateTime();
    QString timeStr = current.toString("yyyy-MM-dd hh:mm:ss dddd");
    m_clockLabel->setText(timeStr);
}

void MainWindow::updateWeekDisplay()
{
    if (!m_weekLabel) return;

    QDate weekEnd = m_currentWeekStart.addDays(6);

    // 计算当前是第几周
    QDate semesterStart = m_courseManager->getSemesterStartDate();
    int weekNumber = (semesterStart.daysTo(m_currentWeekStart) / 7) + 1;

    // 确保周数在合理范围内
    weekNumber = qMax(1, qMin(25, weekNumber));

    QString weekText = QString("第%1周 %2 ~ %3")
                           .arg(weekNumber)
                           .arg(m_currentWeekStart.toString("MM.dd"))
                           .arg(weekEnd.toString("MM.dd"));

    m_weekLabel->setText(weekText);
    populateCourseTable();
}

QColor MainWindow::getCourseColor(const QString &courseType)
{
    if (courseType == "必修") {
        return QColor(231, 76, 60); // 红色
    } else if (courseType == "选修") {
        return QColor(52, 152, 219); // 蓝色
    } else if (courseType == "实验") {
        return QColor(46, 204, 113); // 绿色
    } else {
        return QColor(155, 89, 182); // 紫色
    }
}

void MainWindow::populateCourseTable()
{
    if (!m_courseTable) return;

    // 开始批量更新 - 禁用信号
    m_courseTable->setUpdatesEnabled(true);
    m_courseTable->blockSignals(true);

    // 清除现有课程内容（更高效的方式）
    for (int row = 0; row < m_courseTable->rowCount(); ++row) {
        for (int col = 1; col < m_courseTable->columnCount(); ++col) {
            QTableWidgetItem *item = m_courseTable->item(row, col);
            if (item) {
                item->setText("");
                item->setBackground(QBrush());
                item->setToolTip("");
            }
        }
    }

    try {
        // 获取当前周的课程
        QList<CourseData> courses = m_courseManager->getCoursesByWeek(m_currentWeekStart);

        for (const CourseData &course : courses) {
            int day = course.dayOfWeek - 1;
            if (day < 0 || day > 6) continue;

            // 只处理课程时间段
            for (int slot = course.startSlot - 1; slot < course.endSlot && slot < m_courseTable->rowCount(); ++slot) {
                if (slot < 0) continue;

                QTableWidgetItem *item = m_courseTable->item(slot, day + 1);
                if (!item) {
                    item = new QTableWidgetItem();
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    m_courseTable->setItem(slot, day + 1, item);
                }

                // 只在第一节课显示完整信息
                if (slot == course.startSlot - 1) {
                    QString courseText = QString("%1\n@%2").arg(course.name).arg(course.location);
                    item->setText(courseText);
                    item->setData(Qt::UserRole, course.id);
                }

                // 设置颜色和样式
                QColor courseColor = getCourseColor(course.courseType);
                item->setBackground(courseColor);
                item->setForeground(Qt::black);
                item->setTextAlignment(Qt::AlignCenter);

                QString tooltip = QString("课程: %1\n地点: %2\n时间: 第%3-%4节\n教师: %5\n类型: %6\n学分: %7")
                                      .arg(course.name).arg(course.location)
                                      .arg(course.startSlot).arg(course.endSlot)
                                      .arg(course.teacher.isEmpty() ? "未设置" : course.teacher)
                                      .arg(course.courseType).arg(course.credits);
                item->setToolTip(tooltip);
            }
        }
    } catch (...) {
        qDebug() << "Error populating course table";
    }

    // 恢复信号和更新
    m_courseTable->blockSignals(false);
    m_courseTable->setUpdatesEnabled(true);
    m_courseTable->viewport()->update();
}
void MainWindow::onAddCourse()
{
    animateButton(qobject_cast<QPushButton*>(sender()));
    showAddCourseDialog();
}

void MainWindow::onEditTimeSlots()
{
    animateButton(qobject_cast<QPushButton*>(sender()));
    QMessageBox::information(this, "编辑时间段", "编辑时间段功能开发中...");
}

void MainWindow::onRefresh()
{
    animateButton(qobject_cast<QPushButton*>(sender()));

    // 先刷新数据，再执行动画
    populateCourseTable();

    // 使用QTimer避免阻塞
    QTimer::singleShot(100, [this]() {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(m_courseTable);
        m_courseTable->setGraphicsEffect(effect);

        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", this);
        animation->setDuration(400);
        animation->setKeyValueAt(0, 1.0);
        animation->setKeyValueAt(0.3, 0.6);
        animation->setKeyValueAt(1, 1.0);
        animation->setEasingCurve(QEasingCurve::InOutQuad);

        connect(animation, &QPropertyAnimation::finished, [effect, animation]() {
            animation->deleteLater();
            effect->deleteLater();
        });

        animation->start();
    });

    QMessageBox::information(this, "刷新", "课程表已刷新！");
}

void MainWindow::onSearch()
{
    animateButton(qobject_cast<QPushButton*>(sender()));

    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        populateCourseTable(); // 显示所有课程
        return;
    }

    QList<CourseData> courses = m_courseManager->searchCourses(keyword);

    // 清除表格
    for (int row = 0; row < m_courseTable->rowCount(); ++row) {
        for (int col = 1; col < m_courseTable->columnCount(); ++col) {
            QTableWidgetItem *item = m_courseTable->item(row, col);
            if (item) {
                item->setText("");
                item->setBackground(QBrush());
                item->setToolTip("");
            }
        }
    }

    // 显示搜索结果
    for (const CourseData &course : courses) {
        int day = course.dayOfWeek - 1;
        if (day < 0 || day > 6) continue;

        for (int slot = course.startSlot - 1; slot < course.endSlot; ++slot) {
            if (slot < 0 || slot >= m_courseTable->rowCount()) continue;

            QTableWidgetItem *item = m_courseTable->item(slot, day + 1);
            if (!item) {
                item = new QTableWidgetItem();
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                m_courseTable->setItem(slot, day + 1, item);
            }

            if (slot == course.startSlot - 1) {
                QString courseText = QString("%1\n@%2").arg(course.name).arg(course.location);
                item->setText(courseText);
                item->setData(Qt::UserRole, course.id);
            }

            QColor courseColor = getCourseColor(course.courseType);
            item->setBackground(courseColor);
            item->setForeground(Qt::white);
            item->setTextAlignment(Qt::AlignCenter);

            QString tooltip = QString("课程: %1\n地点: %2\n时间: 第%3-%4节\n教师: %5\n类型: %6\n学分: %7")
                                  .arg(course.name).arg(course.location)
                                  .arg(course.startSlot).arg(course.endSlot)
                                  .arg(course.teacher.isEmpty() ? "未设置" : course.teacher)
                                  .arg(course.courseType).arg(course.credits);
            item->setToolTip(tooltip);
        }
    }
}

void MainWindow::onExport()
{
    animateButton(qobject_cast<QPushButton*>(sender()));

    QString filePath = QFileDialog::getSaveFileName(this, "导出课程数据",
                                                    QDir::homePath() + "/课程表.csv",
                                                    "CSV文件 (*.csv)");
    if (!filePath.isEmpty()) {
        if (m_courseManager->exportToCsv(filePath)) {
            QMessageBox::information(this, "导出成功", "课程数据已成功导出到: " + filePath);
        } else {
            QMessageBox::warning(this, "导出失败", "导出课程数据失败");
        }
    }
}

void MainWindow::onBackup()
{
    animateButton(qobject_cast<QPushButton*>(sender()));

    if (m_courseManager->createBackup()) {
        QMessageBox::information(this, "备份成功", "数据库备份已创建");
    } else {
        QMessageBox::warning(this, "备份失败", "创建数据库备份失败");
    }
}

void MainWindow::prevWeek()
{
    animateButton(qobject_cast<QPushButton*>(sender()));
    m_currentWeekStart = m_currentWeekStart.addDays(-7);
    updateWeekDisplay();
}

void MainWindow::nextWeek()
{
    animateButton(qobject_cast<QPushButton*>(sender()));
    m_currentWeekStart = m_currentWeekStart.addDays(7);
    updateWeekDisplay();
}

void MainWindow::goToThisWeek()
{
    animateButton(qobject_cast<QPushButton*>(sender()));
    QDate today = QDate::currentDate();
    m_currentWeekStart = today.addDays(1 - today.dayOfWeek());
    updateWeekDisplay();
}void MainWindow::showCourseDetails(int row, int column)
{
    if (column == 0) return; // 时间列不处理

    QTableWidgetItem *item = m_courseTable->item(row, column);
    if (!item || item->text().isEmpty()) return;

    // 添加点击动画效果（使用异步避免阻塞）
    QTimer::singleShot(0, [this, row]() {
        animateTableRow(row);
    });

    int courseId = item->data(Qt::UserRole).toInt();
    CourseData course = m_courseManager->getCourseById(courseId);

    if (course.id == -1) return;

    // 显示课程详情和操作选项 - 移除动画避免卡死
    QDialog dialog(this);
    dialog.setWindowTitle("课程详情");
    dialog.setFixedSize(500, 450);

    // 直接显示对话框，不添加动画
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // 课程信息
    QGroupBox *infoGroup = new QGroupBox("课程信息");
    QGridLayout *gridLayout = new QGridLayout(infoGroup);

    // 第一行：课程名称和地点
    QLabel *nameLabel = new QLabel("<b>课程名称:</b>");
    QLabel *nameValue = new QLabel(course.name);
    nameValue->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: bold;");

    QLabel *locationLabel = new QLabel("<b>上课地点:</b>");
    QLabel *locationValue = new QLabel(course.location);
    locationValue->setStyleSheet("color: #2c3e50; font-size: 14px;");

    gridLayout->addWidget(nameLabel, 0, 0);
    gridLayout->addWidget(nameValue, 0, 1);
    gridLayout->addWidget(locationLabel, 0, 2);
    gridLayout->addWidget(locationValue, 0, 3);

    // 第二行：上课时间和教师
    QLabel *timeLabel = new QLabel("<b>上课时间:</b>");
    QString timeText = QString("周%1 第%2-%3节").arg(course.dayOfWeek).arg(course.startSlot).arg(course.endSlot);
    QLabel *timeValue = new QLabel(timeText);
    timeValue->setStyleSheet("color: #e74c3c; font-size: 14px; font-weight: bold;");

    QLabel *teacherLabel = new QLabel("<b>授课教师:</b>");
    QLabel *teacherValue = new QLabel(course.teacher.isEmpty() ? "未设置" : course.teacher);
    teacherValue->setStyleSheet("color: #2c3e50; font-size: 14px;");

    gridLayout->addWidget(timeLabel, 1, 0);
    gridLayout->addWidget(timeValue, 1, 1);
    gridLayout->addWidget(teacherLabel, 1, 2);
    gridLayout->addWidget(teacherValue, 1, 3);

    // 第三行：课程类型和学分
    QLabel *typeLabel = new QLabel("<b>课程类型:</b>");
    QLabel *typeValue = new QLabel(course.courseType);
    typeValue->setStyleSheet("color: #2c3e50; font-size: 14px;");

    QLabel *creditLabel = new QLabel("<b>学分:</b>");
    QLabel *creditValue = new QLabel(QString::number(course.credits));
    creditValue->setStyleSheet("color: #2c3e50; font-size: 14px;");

    gridLayout->addWidget(typeLabel, 2, 0);
    gridLayout->addWidget(typeValue, 2, 1);
    gridLayout->addWidget(creditLabel, 2, 2);
    gridLayout->addWidget(creditValue, 2, 3);

    // 第四行：课程周期
    QLabel *periodLabel = new QLabel("<b>课程周期:</b>");
    QString periodText = QString("%1 ~ %2").arg(course.startDate.toString("yyyy年MM月dd日")).arg(course.endDate.toString("yyyy年MM月dd日"));
    QLabel *periodValue = new QLabel(periodText);
    periodValue->setStyleSheet("color: #7f8c8d; font-size: 13px;");

    gridLayout->addWidget(periodLabel, 3, 0);
    gridLayout->addWidget(periodValue, 3, 1, 1, 3);

    // 设置布局属性
    gridLayout->setHorizontalSpacing(15);
    gridLayout->setVerticalSpacing(12);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(3, 1);

    mainLayout->addWidget(infoGroup);

    // 考试倒计时区域
    if (course.examDate.isValid()) {
        QGroupBox *examGroup = new QGroupBox("📝 考试信息");
        QVBoxLayout *examLayout = new QVBoxLayout(examGroup);

        // 考试日期
        QHBoxLayout *examDateLayout = new QHBoxLayout();
        QLabel *examDateLabel = new QLabel("<b>考试日期:</b>");
        QLabel *examDateValue = new QLabel(course.examDate.toString("yyyy年MM月dd日 dddd"));
        examDateValue->setStyleSheet("color: #e74c3c; font-size: 14px; font-weight: bold;");

        examDateLayout->addWidget(examDateLabel);
        examDateLayout->addWidget(examDateValue);
        examDateLayout->addStretch();

        examLayout->addLayout(examDateLayout);

        // 考试倒计时
        QDate today = QDate::currentDate();
        int daysToExam = today.daysTo(course.examDate);

        QHBoxLayout *countdownLayout = new QHBoxLayout();
        QLabel *countdownLabel = new QLabel("<b>距离考试:</b>");

        QString countdownText;
        QString countdownStyle;

        if (daysToExam < 0) {
            countdownText = "考试已结束";
            countdownStyle = "color: #95a5a6; font-size: 14px;";
        } else if (daysToExam == 0) {
            countdownText = "⏰ 今天考试！";
            countdownStyle = "color: #e74c3c; font-size: 16px; font-weight: bold;";
        } else if (daysToExam <= 7) {
            countdownText = QString("🚨 还剩 %1 天！").arg(daysToExam);
            countdownStyle = "color: #e74c3c; font-size: 15px; font-weight: bold;";
        } else if (daysToExam <= 30) {
            countdownText = QString("📚 还剩 %1 天").arg(daysToExam);
            countdownStyle = "color: #f39c12; font-size: 14px; font-weight: bold;";
        } else {
            countdownText = QString("📖 还剩 %1 天").arg(daysToExam);
            countdownStyle = "color: #27ae60; font-size: 14px;";
        }

        QLabel *countdownValue = new QLabel(countdownText);
        countdownValue->setStyleSheet(countdownStyle);

        countdownLayout->addWidget(countdownLabel);
        countdownLayout->addWidget(countdownValue);
        countdownLayout->addStretch();

        examLayout->addLayout(countdownLayout);

        // 考试进度条
        if (daysToExam >= 0) {
            QHBoxLayout *progressLayout = new QHBoxLayout();
            QLabel *progressLabel = new QLabel("<b>备考进度:</b>");

            int totalDays = course.startDate.daysTo(course.examDate);
            int passedDays = course.startDate.daysTo(today);
            int progress = 0;

            if (totalDays > 0) {
                progress = qMin(100, (passedDays * 100) / totalDays);
            }

            QProgressBar *progressBar = new QProgressBar();
            progressBar->setRange(0, 100);
            progressBar->setValue(progress);
            progressBar->setTextVisible(true);
            progressBar->setFormat(QString("%1%").arg(progress));

            if (progress < 30) {
                progressBar->setStyleSheet(R"(
                    QProgressBar {
                        border: 2px solid #bdc3c7;
                        border-radius: 5px;
                        text-align: center;
                        color: #2c3e50;
                    }
                    QProgressBar::chunk {
                        background-color: #27ae60;
                        border-radius: 3px;
                    }
                )");
            } else if (progress < 70) {
                progressBar->setStyleSheet(R"(
                    QProgressBar {
                        border: 2px solid #bdc3c7;
                        border-radius: 5px;
                        text-align: center;
                        color: #2c3e50;
                    }
                    QProgressBar::chunk {
                        background-color: #f39c12;
                        border-radius: 3px;
                    }
                )");
            } else {
                progressBar->setStyleSheet(R"(
                    QProgressBar {
                        border: 2px solid #bdc3c7;
                        border-radius: 5px;
                        text-align: center;
                        color: #2c3e50;
                    }
                    QProgressBar::chunk {
                        background-color: #e74c3c;
                        border-radius: 3px;
                    }
                )");
            }

            progressLayout->addWidget(progressLabel);
            progressLayout->addWidget(progressBar, 1);

            examLayout->addLayout(progressLayout);
        }

        mainLayout->addWidget(examGroup);
    }

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #bdc3c7;");
    mainLayout->addWidget(line);

    // 操作按钮 - 简化按钮点击处理
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *editBtn = new QPushButton("✏️ 编辑课程");
    QPushButton *deleteBtn = new QPushButton("🗑️ 删除课程");
    QPushButton *closeBtn = new QPushButton("❌ 关闭");

    // 简化按钮样式
    editBtn->setStyleSheet("QPushButton { background: #f39c12; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: bold; }");
    deleteBtn->setStyleSheet("QPushButton { background: #e74c3c; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: bold; }");
    closeBtn->setStyleSheet("QPushButton { background: #95a5a6; color: white; border: none; padding: 10px 25px; border-radius: 8px; font-weight: bold; }");

    // 简化按钮连接
    connect(editBtn, &QPushButton::clicked, [&, courseId]() {
        dialog.accept();
        showEditCourseDialog(courseId);
    });

    connect(deleteBtn, &QPushButton::clicked, [&, courseId]() {
        dialog.accept();
        showDeleteConfirmation(courseId);
    });

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);

    // 直接显示对话框，不使用动画
    dialog.exec();
}void MainWindow::showAddCourseDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("添加新课程");
    dialog.setFixedSize(550, 700);

    // 设置对话框样式 - 更清新的白色系
    dialog.setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #f8fafc, stop:1 #e2e8f0);
            font-family: 'Microsoft YaHei', 'Segoe UI';
        }
        QGroupBox {
            background: white;
            border: 1.5px solid #e2e8f0;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            font-weight: 600;
            color: #1e293b;
            font-size: 14px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px 0 8px;
            color: #475569;
            font-weight: 600;
            font-size: 13px;
        }
        /* 日历控件样式 */
        QCalendarWidget {
            background: white;
            border: 1.5px solid #e1e5e9;
            border-radius: 10px;
        }
        QCalendarWidget QToolButton {
            background: #f8f9fa;
            color: #2c3e50;
            font-weight: bold;
            border-radius: 6px;
            padding: 5px;
            margin: 2px;
        }
        QCalendarWidget QToolButton:hover {
            background: #e3f2fd;
        }
        QCalendarWidget QMenu {
            background: white;
            border: 1.5px solid #e1e5e9;
            border-radius: 6px;
        }
        QCalendarWidget QSpinBox {
            padding: 5px;
            border: 1.5px solid #e1e5e9;
            border-radius: 6px;
            background: white;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: #f8f9fa;
            border-bottom: 1px solid #e1e5e9;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
        QCalendarWidget QAbstractItemView:enabled {
            background: white;
            color: #2c3e50;
            selection-background-color: #90caf9;
            selection-color: white;
            border: none;
            outline: none;
        }
        QCalendarWidget QAbstractItemView:disabled {
            color: #bdc3c7;
        }
        QCalendarWidget QHeaderView::section {
            background: #f1f5f9;
            color: #475569;
            font-weight: bold;
            padding: 8px;
            border: none;
        }
        QCalendarWidget QAbstractItemView:enabled:selected {
            background: #2196f3;
            color: white;
            border-radius: 4px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // === 顶部标题区域 ===
    QWidget *headerWidget = new QWidget();
    headerWidget->setStyleSheet(R"(
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
            stop:0 #f0f9ff, stop:1 #f8fafc);
        border: 1.5px solid #e0f2fe;
        border-radius: 12px;
        margin: 8px;
        padding: 5px;
    )");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);

    // 图标标签
    QLabel *iconLabel = new QLabel("📚");
    iconLabel->setStyleSheet(R"(
        font-size: 28px;
        margin: 8px;
        padding: 8px;
        background: #f1f5f9;
        border-radius: 8px;
    )");

    // 标题
    QLabel *titleLabel = new QLabel("添加新课程");
    titleLabel->setStyleSheet(R"(
        font-size: 22px;
        font-weight: 600;
        color: #1e293b;
        background: transparent;
        margin: 10px;
    )");

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    mainLayout->addWidget(headerWidget);

    // 学期信息
    QLabel *semesterLabel = new QLabel(QString("🎓 当前学期: %1").arg(m_courseManager->getCurrentSemester()));
    semesterLabel->setStyleSheet(R"(
        color: #475569;
        font-size: 13px;
        font-weight: 500;
        margin: 5px;
        padding: 10px;
        background: white;
        border: 1.5px solid #f1f5f9;
        border-radius: 10px;
    )");
    semesterLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(semesterLabel);

    // === 表单区域 ===
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background: transparent;
            border: none;
            margin: 5px;
        }
        QScrollBar:vertical {
            background: #f8fafc;
            width: 8px;
            margin: 0px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #cbd5e1;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #94a3b8;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            border: none;
            background: none;
        }
    )");

    QWidget *formContainer = new QWidget();
    formContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *formContainerLayout = new QVBoxLayout(formContainer);
    formContainerLayout->setSpacing(12);
    formContainerLayout->setContentsMargins(8, 8, 8, 8);

    // 基本信息分组
    QGroupBox *basicInfoGroup = new QGroupBox("📝 基本信息");
    QFormLayout *basicFormLayout = new QFormLayout(basicInfoGroup);
    basicFormLayout->setVerticalSpacing(15);
    basicFormLayout->setHorizontalSpacing(20);
    basicFormLayout->setContentsMargins(15, 20, 15, 20);

    // 课程名称
    QLineEdit *nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("请输入课程名称...");
    nameEdit->setStyleSheet(getInputStyle());
    nameEdit->setMinimumHeight(40);

    // 上课星期
    QComboBox *dayCombo = new QComboBox();
    dayCombo->addItems({"📅 周一", "📅 周二", "📅 周三", "📅 周四", "📅 周五", "📅 周六", "📅 周日"});
    dayCombo->setStyleSheet(getComboBoxStyle());

    // 时间设置
    QHBoxLayout *timeLayout = new QHBoxLayout();
    QSpinBox *startSlotSpin = new QSpinBox();
    startSlotSpin->setRange(1, 10);
    startSlotSpin->setValue(1);
    startSlotSpin->setStyleSheet(getSpinBoxStyle());

    QLabel *toLabel = new QLabel("至");
    toLabel->setStyleSheet(R"(
        color: #64748b;
        font-weight: 500;
        margin: 0 12px;
        font-size: 13px;
    )");

    QSpinBox *endSlotSpin = new QSpinBox();
    endSlotSpin->setRange(1, 10);
    endSlotSpin->setValue(2);
    endSlotSpin->setStyleSheet(getSpinBoxStyle());

    timeLayout->addWidget(startSlotSpin);
    timeLayout->addWidget(toLabel);
    timeLayout->addWidget(endSlotSpin);
    timeLayout->addStretch();

    // 教室地点
    QLineEdit *locationEdit = new QLineEdit();
    locationEdit->setPlaceholderText("例如: A101教室");
    locationEdit->setStyleSheet(getInputStyle());

    basicFormLayout->addRow("🎯 课程名称:", nameEdit);
    basicFormLayout->addRow("📅 上课星期:", dayCombo);
    basicFormLayout->addRow("⏰ 上课节次:", timeLayout);
    basicFormLayout->addRow("📍 教室地点:", locationEdit);

    // 详细信息分组
    QGroupBox *detailInfoGroup = new QGroupBox("📋 详细信息");
    QFormLayout *detailFormLayout = new QFormLayout(detailInfoGroup);
    detailFormLayout->setVerticalSpacing(15);
    detailFormLayout->setHorizontalSpacing(20);
    detailFormLayout->setContentsMargins(15, 20, 15, 20);

    QLineEdit *teacherEdit = new QLineEdit();
    teacherEdit->setPlaceholderText("请输入授课教师姓名...");
    teacherEdit->setStyleSheet(getInputStyle());

    QComboBox *typeCombo = new QComboBox();
    typeCombo->addItems({"📘 必修", "📗 选修", "🔬 实验", "📙 其他"});
    typeCombo->setStyleSheet(getComboBoxStyle());

    QDoubleSpinBox *creditSpin = new QDoubleSpinBox();
    creditSpin->setRange(0, 10);
    creditSpin->setValue(2.0);
    creditSpin->setSingleStep(0.5);
    creditSpin->setStyleSheet(getSpinBoxStyle());

    detailFormLayout->addRow("👨‍🏫 授课教师:", teacherEdit);
    detailFormLayout->addRow("📊 课程类型:", typeCombo);
    detailFormLayout->addRow("⭐ 学分:", creditSpin);

    // 时间安排分组
    QGroupBox *timeInfoGroup = new QGroupBox("📅 时间安排");
    QFormLayout *timeFormLayout = new QFormLayout(timeInfoGroup);
    timeFormLayout->setVerticalSpacing(15);
    timeFormLayout->setHorizontalSpacing(20);
    timeFormLayout->setContentsMargins(15, 20, 15, 20);

    QDateEdit *startDateEdit = new QDateEdit();
    startDateEdit->setDate(m_courseManager->getSemesterStartDate());
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy年MM月dd日");
    startDateEdit->setStyleSheet(getDateEditStyle());

    QDateEdit *endDateEdit = new QDateEdit();
    endDateEdit->setDate(m_courseManager->getSemesterEndDate());
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setDisplayFormat("yyyy年MM月dd日");
    endDateEdit->setStyleSheet(getDateEditStyle());

    QDateEdit *examDateEdit = new QDateEdit();
    examDateEdit->setDate(m_courseManager->getSemesterEndDate());
    examDateEdit->setCalendarPopup(true);
    examDateEdit->setDisplayFormat("yyyy年MM月dd日");
    examDateEdit->setStyleSheet(getDateEditStyle());

    timeFormLayout->addRow("🚀 开始日期:", startDateEdit);
    timeFormLayout->addRow("🏁 结束日期:", endDateEdit);
    timeFormLayout->addRow("📝 考试日期:", examDateEdit);

    formContainerLayout->addWidget(basicInfoGroup);
    formContainerLayout->addWidget(detailInfoGroup);
    formContainerLayout->addWidget(timeInfoGroup);
    formContainerLayout->addStretch();

    scrollArea->setWidget(formContainer);
    mainLayout->addWidget(scrollArea);

    // === 按钮区域 ===
    QWidget *buttonWidget = new QWidget();
    buttonWidget->setStyleSheet(R"(
        background: white;
        border: 1.5px solid #f1f5f9;
        border-radius: 12px;
        margin: 8px;
        padding: 15px;
    )");
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);

    QPushButton *cancelBtn = new QPushButton("❌ 取消");
    QPushButton *saveBtn = new QPushButton("💾 保存课程");

    cancelBtn->setStyleSheet(getButtonStyle("#ef4444"));  // 红色
    saveBtn->setStyleSheet(getButtonStyle("#10b981"));    // 绿色

    cancelBtn->setMinimumSize(120, 45);
    saveBtn->setMinimumSize(120, 45);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    mainLayout->addWidget(buttonWidget);

    // 连接信号
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&, nameEdit, locationEdit, startSlotSpin, endSlotSpin, teacherEdit, typeCombo, creditSpin, startDateEdit, endDateEdit, examDateEdit]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "请输入课程名称！");
            return;
        }

        if (locationEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "请输入教室地点！");
            return;
        }

        if (startSlotSpin->value() > endSlotSpin->value()) {
            QMessageBox::warning(&dialog, "输入错误", "开始节次不能大于结束节次！");
            return;
        }

        CourseData course;
        course.name = nameEdit->text();
        course.dayOfWeek = dayCombo->currentIndex() + 1;
        course.startSlot = startSlotSpin->value();
        course.endSlot = endSlotSpin->value();
        course.location = locationEdit->text();
        course.teacher = teacherEdit->text();
        course.courseType = typeCombo->currentText().mid(2); // 移除表情符号
        course.credits = creditSpin->value();
        course.startDate = startDateEdit->date();
        course.endDate = endDateEdit->date();
        course.examDate = examDateEdit->date();

        if (m_courseManager->addCourse(course)) {
            QMessageBox::information(&dialog, "成功", "课程添加成功！");
            dialog.accept();
            populateCourseTable();
        } else {
            QMessageBox::critical(&dialog, "错误", "添加课程失败！");
        }
    });

    dialog.exec();
}void MainWindow::showEditCourseDialog(int courseId)
{
    CourseData course = m_courseManager->getCourseById(courseId);
    if (course.id == -1) return;

    QDialog dialog(this);
    dialog.setWindowTitle("编辑课程");
    dialog.setFixedSize(550, 700);

    // 设置对话框样式 - 更清新的白色系
    dialog.setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #f8fafc, stop:1 #e2e8f0);
            font-family: 'Microsoft YaHei', 'Segoe UI';
        }
        QGroupBox {
            background: white;
            border: 1.5px solid #e2e8f0;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            font-weight: 600;
            color: #1e293b;
            font-size: 14px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px 0 8px;
            color: #475569;
            font-weight: 600;
            font-size: 13px;
        }
        /* 日历控件样式 */
        QCalendarWidget {
            background: white;
            border: 1.5px solid #e1e5e9;
            border-radius: 10px;
        }
        QCalendarWidget QToolButton {
            background: #f8f9fa;
            color: #2c3e50;
            font-weight: bold;
            border-radius: 6px;
            padding: 5px;
            margin: 2px;
        }
        QCalendarWidget QToolButton:hover {
            background: #e3f2fd;
        }
        QCalendarWidget QMenu {
            background: white;
            border: 1.5px solid #e1e5e9;
            border-radius: 6px;
        }
        QCalendarWidget QSpinBox {
            padding: 5px;
            border: 1.5px solid #e1e5e9;
            border-radius: 6px;
            background: white;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: #f8f9fa;
            border-bottom: 1px solid #e1e5e9;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
        QCalendarWidget QAbstractItemView:enabled {
            background: white;
            color: #2c3e50;
            selection-background-color: #90caf9;
            selection-color: white;
            border: none;
            outline: none;
        }
        QCalendarWidget QAbstractItemView:disabled {
            color: #bdc3c7;
        }
        QCalendarWidget QHeaderView::section {
            background: #f1f5f9;
            color: #475569;
            font-weight: bold;
            padding: 8px;
            border: none;
        }
        QCalendarWidget QAbstractItemView:enabled:selected {
            background: #2196f3;
            color: white;
            border-radius: 4px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // === 顶部标题区域 ===
    QWidget *headerWidget = new QWidget();
    headerWidget->setStyleSheet(R"(
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
            stop:0 #fff7ed, stop:1 #f8fafc);
        border: 1.5px solid #fed7aa;
        border-radius: 12px;
        margin: 8px;
        padding: 5px;
    )");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);

    // 图标标签
    QLabel *iconLabel = new QLabel("✏️");
    iconLabel->setStyleSheet(R"(
        font-size: 28px;
        margin: 8px;
        padding: 8px;
        background: #fef3c7;
        border-radius: 8px;
    )");

    // 标题
    QLabel *titleLabel = new QLabel("编辑课程");
    titleLabel->setStyleSheet(R"(
        font-size: 22px;
        font-weight: 600;
        color: #1e293b;
        background: transparent;
        margin: 10px;
    )");

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    mainLayout->addWidget(headerWidget);

    // 课程信息
    QLabel *courseInfoLabel = new QLabel(QString("📖 正在编辑: %1").arg(course.name));
    courseInfoLabel->setStyleSheet(R"(
        color: #475569;
        font-size: 13px;
        font-weight: 500;
        margin: 5px;
        padding: 10px;
        background: white;
        border: 1.5px solid #f1f5f9;
        border-radius: 10px;
    )");
    courseInfoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(courseInfoLabel);

    // === 表单区域 ===
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background: transparent;
            border: none;
            margin: 5px;
        }
        QScrollBar:vertical {
            background: #f8fafc;
            width: 8px;
            margin: 0px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #cbd5e1;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #94a3b8;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            border: none;
            background: none;
        }
    )");

    QWidget *formContainer = new QWidget();
    formContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *formContainerLayout = new QVBoxLayout(formContainer);
    formContainerLayout->setSpacing(12);
    formContainerLayout->setContentsMargins(8, 8, 8, 8);

    // 基本信息分组
    QGroupBox *basicInfoGroup = new QGroupBox("📝 基本信息");
    QFormLayout *basicFormLayout = new QFormLayout(basicInfoGroup);
    basicFormLayout->setVerticalSpacing(15);
    basicFormLayout->setHorizontalSpacing(20);
    basicFormLayout->setContentsMargins(15, 20, 15, 20);

    // 课程名称
    QLineEdit *nameEdit = new QLineEdit(course.name);
    nameEdit->setPlaceholderText("请输入课程名称...");
    nameEdit->setStyleSheet(getInputStyle());
    nameEdit->setMinimumHeight(40);

    // 上课星期
    QComboBox *dayCombo = new QComboBox();
    dayCombo->addItems({"📅 周一", "📅 周二", "📅 周三", "📅 周四", "📅 周五", "📅 周六", "📅 周日"});
    dayCombo->setStyleSheet(getComboBoxStyle());
    dayCombo->setCurrentIndex(course.dayOfWeek - 1);

    // 时间设置
    QHBoxLayout *timeLayout = new QHBoxLayout();
    QSpinBox *startSlotSpin = new QSpinBox();
    startSlotSpin->setRange(1, 10);
    startSlotSpin->setValue(course.startSlot);
    startSlotSpin->setStyleSheet(getSpinBoxStyle());

    QLabel *toLabel = new QLabel("至");
    toLabel->setStyleSheet(R"(
        color: #64748b;
        font-weight: 500;
        margin: 0 12px;
        font-size: 13px;
    )");

    QSpinBox *endSlotSpin = new QSpinBox();
    endSlotSpin->setRange(1, 10);
    endSlotSpin->setValue(course.endSlot);
    endSlotSpin->setStyleSheet(getSpinBoxStyle());

    timeLayout->addWidget(startSlotSpin);
    timeLayout->addWidget(toLabel);
    timeLayout->addWidget(endSlotSpin);
    timeLayout->addStretch();

    // 教室地点
    QLineEdit *locationEdit = new QLineEdit(course.location);
    locationEdit->setPlaceholderText("例如: A101教室");
    locationEdit->setStyleSheet(getInputStyle());

    basicFormLayout->addRow("🎯 课程名称:", nameEdit);
    basicFormLayout->addRow("📅 上课星期:", dayCombo);
    basicFormLayout->addRow("⏰ 上课节次:", timeLayout);
    basicFormLayout->addRow("📍 教室地点:", locationEdit);

    // 详细信息分组
    QGroupBox *detailInfoGroup = new QGroupBox("📋 详细信息");
    QFormLayout *detailFormLayout = new QFormLayout(detailInfoGroup);
    detailFormLayout->setVerticalSpacing(15);
    detailFormLayout->setHorizontalSpacing(20);
    detailFormLayout->setContentsMargins(15, 20, 15, 20);

    QLineEdit *teacherEdit = new QLineEdit(course.teacher);
    teacherEdit->setPlaceholderText("请输入授课教师姓名...");
    teacherEdit->setStyleSheet(getInputStyle());

    QComboBox *typeCombo = new QComboBox();
    typeCombo->addItems({"📘 必修", "📗 选修", "🔬 实验", "📙 其他"});
    typeCombo->setStyleSheet(getComboBoxStyle());

    // 设置课程类型，移除表情符号进行匹配
    QString typeWithEmoji;
    if (course.courseType == "必修") typeWithEmoji = "📘 必修";
    else if (course.courseType == "选修") typeWithEmoji = "📗 选修";
    else if (course.courseType == "实验") typeWithEmoji = "🔬 实验";
    else typeWithEmoji = "📙 其他";
    typeCombo->setCurrentText(typeWithEmoji);

    QDoubleSpinBox *creditSpin = new QDoubleSpinBox();
    creditSpin->setRange(0, 10);
    creditSpin->setValue(course.credits);
    creditSpin->setSingleStep(0.5);
    creditSpin->setStyleSheet(getSpinBoxStyle());

    detailFormLayout->addRow("👨‍🏫 授课教师:", teacherEdit);
    detailFormLayout->addRow("📊 课程类型:", typeCombo);
    detailFormLayout->addRow("⭐ 学分:", creditSpin);

    // 时间安排分组
    QGroupBox *timeInfoGroup = new QGroupBox("📅 时间安排");
    QFormLayout *timeFormLayout = new QFormLayout(timeInfoGroup);
    timeFormLayout->setVerticalSpacing(15);
    timeFormLayout->setHorizontalSpacing(20);
    timeFormLayout->setContentsMargins(15, 20, 15, 20);

    QDateEdit *startDateEdit = new QDateEdit(course.startDate);
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy年MM月dd日");
    startDateEdit->setStyleSheet(getDateEditStyle());

    QDateEdit *endDateEdit = new QDateEdit(course.endDate);
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setDisplayFormat("yyyy年MM月dd日");
    endDateEdit->setStyleSheet(getDateEditStyle());

    QDateEdit *examDateEdit = new QDateEdit();
    if (course.examDate.isValid()) {
        examDateEdit->setDate(course.examDate);
    } else {
        examDateEdit->setDate(m_courseManager->getSemesterEndDate());
    }
    examDateEdit->setCalendarPopup(true);
    examDateEdit->setDisplayFormat("yyyy年MM月dd日");
    examDateEdit->setStyleSheet(getDateEditStyle());

    timeFormLayout->addRow("🚀 开始日期:", startDateEdit);
    timeFormLayout->addRow("🏁 结束日期:", endDateEdit);
    timeFormLayout->addRow("📝 考试日期:", examDateEdit);

    formContainerLayout->addWidget(basicInfoGroup);
    formContainerLayout->addWidget(detailInfoGroup);
    formContainerLayout->addWidget(timeInfoGroup);
    formContainerLayout->addStretch();

    scrollArea->setWidget(formContainer);
    mainLayout->addWidget(scrollArea);

    // === 按钮区域 ===
    QWidget *buttonWidget = new QWidget();
    buttonWidget->setStyleSheet(R"(
        background: white;
        border: 1.5px solid #f1f5f9;
        border-radius: 12px;
        margin: 8px;
        padding: 15px;
    )");
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);

    QPushButton *cancelBtn = new QPushButton("❌ 取消");
    QPushButton *saveBtn = new QPushButton("💾 保存修改");

    cancelBtn->setStyleSheet(getButtonStyle("#ef4444"));  // 红色
    saveBtn->setStyleSheet(getButtonStyle("#f59e0b"));    // 橙色

    cancelBtn->setMinimumSize(120, 45);
    saveBtn->setMinimumSize(120, 45);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    mainLayout->addWidget(buttonWidget);

    // 连接信号
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&, nameEdit, locationEdit, startSlotSpin, endSlotSpin, teacherEdit, typeCombo, creditSpin, startDateEdit, endDateEdit, examDateEdit]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "请输入课程名称！");
            return;
        }

        if (locationEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "请输入教室地点！");
            return;
        }

        if (startSlotSpin->value() > endSlotSpin->value()) {
            QMessageBox::warning(&dialog, "输入错误", "开始节次不能大于结束节次！");
            return;
        }

        course.name = nameEdit->text();
        course.dayOfWeek = dayCombo->currentIndex() + 1;
        course.startSlot = startSlotSpin->value();
        course.endSlot = endSlotSpin->value();
        course.location = locationEdit->text();
        course.teacher = teacherEdit->text();
        course.courseType = typeCombo->currentText().mid(2); // 移除表情符号
        course.credits = creditSpin->value();
        course.startDate = startDateEdit->date();
        course.endDate = endDateEdit->date();
        course.examDate = examDateEdit->date();

        if (m_courseManager->updateCourse(course)) {
            QMessageBox::information(&dialog, "成功", "课程修改成功！");
            dialog.accept();
            populateCourseTable();
        } else {
            QMessageBox::critical(&dialog, "错误", "修改课程失败！");
        }
    });

    dialog.exec();
}

void MainWindow::showDeleteConfirmation(int courseId)
{
    CourseData course = m_courseManager->getCourseById(courseId);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("删除课程");
    msgBox.setText(QString("确定要删除课程 \"%1\" 吗？").arg(course.name));
    msgBox.setInformativeText("此操作不可撤销！");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);

    if (msgBox.exec() == QMessageBox::Yes) {
        if (m_courseManager->deleteCourse(courseId)) {
            QMessageBox::information(this, "成功", "课程删除成功！");
            populateCourseTable();
        } else {
            QMessageBox::critical(this, "错误", "删除课程失败！");
        }
    }
}// ==================== 动画效果实现 ====================

void MainWindow::animateButton(QWidget *button)
{
    if (!button) return;

    // 检查是否已有动画在运行
    if (button->property("animationRunning").toBool()) {
        return;
    }

    button->setProperty("animationRunning", true);

    QPropertyAnimation *animation = new QPropertyAnimation(button, "geometry", this);
    animation->setDuration(200);
    animation->setKeyValueAt(0, button->geometry());
    animation->setKeyValueAt(0.5, button->geometry().adjusted(-2, -2, 2, 2));
    animation->setKeyValueAt(1, button->geometry());
    animation->setEasingCurve(QEasingCurve::OutBack);

    // 修复：确保动画正确清理
    connect(animation, &QPropertyAnimation::finished, [button, animation]() {
        button->setProperty("animationRunning", false);
        animation->deleteLater();
    });

    animation->start();
}

void MainWindow::animateTableRow(int row)
{
    if (!m_courseTable || row < 0 || row >= m_courseTable->rowCount()) return;

    // 简化动画逻辑，避免复杂操作
    for (int col = 1; col < m_courseTable->columnCount(); ++col) {
        QTableWidgetItem *item = m_courseTable->item(row, col);
        if (item && !item->text().isEmpty()) {
            // 直接设置高亮颜色，不使用动画
            QColor originalColor = item->background().color();
            QColor highlightColor = QColor(255, 255, 100, 150); // 浅黄色高亮

            item->setBackground(highlightColor);

            // 使用单次定时器恢复颜色
            QTimer::singleShot(300, [this, item, originalColor]() {
                if (item) {
                    item->setBackground(originalColor);
                }
            });
        }
    }
}

void MainWindow::fadeInWidget(QWidget *widget)
{
    if (!widget) return;

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(800);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::fadeInDialog(QDialog *dialog)
{
    if (!dialog) return;

    // 直接显示对话框，不添加淡入动画
    // 动画可能导致对话框显示问题
    dialog->show();
}

void MainWindow::pulseAnimation(QWidget *widget)
{
    if (!widget) return;

    QPropertyAnimation *animation = new QPropertyAnimation(widget, "geometry", this);
    animation->setDuration(1000);
    animation->setKeyValueAt(0, widget->geometry());
    animation->setKeyValueAt(0.5, widget->geometry().adjusted(-3, -3, 3, 3));
    animation->setKeyValueAt(1, widget->geometry());
    animation->setEasingCurve(QEasingCurve::InOutSine);
    animation->setLoopCount(2); // 循环2次
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::shakeWidget(QWidget *widget)
{
    if (!widget) return;

    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos", this);
    animation->setDuration(500);
    animation->setKeyValueAt(0, widget->pos());
    animation->setKeyValueAt(0.1, widget->pos() + QPoint(5, 0));
    animation->setKeyValueAt(0.2, widget->pos() + QPoint(-5, 0));
    animation->setKeyValueAt(0.3, widget->pos() + QPoint(5, 0));
    animation->setKeyValueAt(0.4, widget->pos() + QPoint(-5, 0));
    animation->setKeyValueAt(0.5, widget->pos() + QPoint(5, 0));
    animation->setKeyValueAt(1, widget->pos());
    animation->setEasingCurve(QEasingCurve::InOutSine);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
