
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
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>

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
    ,m_isDarkMode(false)
    ,m_canNavigateToNextWeek(true)
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
        "    min-height: 36px;"
        "    min-width: 50px;"
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
    connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::showCourseSearchDialog);

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

    // 添加设置学期按钮
    QPushButton *semesterBtn = new QPushButton("📅 设置学期", this);
    semesterBtn->setObjectName("actionButton");
    semesterBtn->setStyleSheet(actionButtonStyle);
    connect(semesterBtn, &QPushButton::clicked, this, &MainWindow::onSetSemester);

    QPushButton *themeBtn = new QPushButton("🌙 夜间模式", this);
    themeBtn->setObjectName("themeButton");
    themeBtn->setStyleSheet(actionButtonStyle);
    connect(themeBtn, &QPushButton::clicked, this, &MainWindow::onToggleTheme);

    QPushButton *refreshBtn = new QPushButton("🔄 刷新", this);
    refreshBtn->setObjectName("actionButton");
    refreshBtn->setStyleSheet(actionButtonStyle);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefresh);

    m_exportBtn = new QPushButton("📤 导出数据", this);
    m_exportBtn->setObjectName("actionButton");
    m_exportBtn->setStyleSheet(actionButtonStyle);
    connect(m_exportBtn, &QPushButton::clicked, this, &MainWindow::onExport);

    m_backupBtn = new QPushButton("💾 备份数据", this);
    m_backupBtn->setObjectName("actionButton");
    m_backupBtn->setStyleSheet(actionButtonStyle);
    connect(m_backupBtn, &QPushButton::clicked, this, &MainWindow::onBackup);

    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(semesterBtn);  // 添加设置学期按钮
    buttonLayout->addWidget(themeBtn);
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
            padding: 10px 22px;
            border-radius: 999px;
            min-height: 36px;
            min-width: 120px;
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
    int totalWeeks = m_courseManager->getSemesterWeeks();

    // 确保周数在合理范围内
    weekNumber = qMax(1, qMin(totalWeeks, weekNumber));

    QString weekText = QString("第%1周/%2周 %3 ~ %4")
                           .arg(weekNumber)
                           .arg(totalWeeks)
                           .arg(m_currentWeekStart.toString("MM.dd"))
                           .arg(weekEnd.toString("MM.dd"));

    m_weekLabel->setText(weekText);

    // 如果是最后一周，改变样式
    if (isLastWeek()) {
        m_weekLabel->setStyleSheet(R"(
            font-size: 18px;
            font-weight: bold;
            color: #ffffff;
            padding: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FF416C, stop:1 #FF4B2B);
            border-radius: 15px;
            margin: 5px;
        )");
    } else {
        // 恢复正常样式
        if (m_isDarkMode) {
            m_weekLabel->setStyleSheet(R"(
                font-size: 18px;
                font-weight: bold;
                color: #ecf0f1;
                padding: 15px;
                background: #34495e;
                border-radius: 15px;
                margin: 5px;
                border: 1px solid #404040;
            )");
        } else {
            m_weekLabel->setStyleSheet(R"(
                font-size: 18px;
                font-weight: bold;
                color: #34495e;
                padding: 15px;
                background: white;
                border-radius: 15px;
                margin: 5px;
            )");
        }
    }

    populateCourseTable();
}

QColor MainWindow::getCourseColor(const QString &courseType)
{
    if (m_isDarkMode) {
        // 夜间模式使用更亮的颜色
        if (courseType == "必修") {
            return QColor(220, 80, 70); // 亮红色
        } else if (courseType == "选修") {
            return QColor(70, 130, 220); // 亮蓝色
        } else if (courseType == "实验") {
            return QColor(70, 180, 80); // 亮绿色
        } else {
            return QColor(170, 100, 200); // 亮紫色
        }
    } else {
        // 日间模式使用原颜色
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
}
void MainWindow::populateCourseTable()
{
    if (!m_courseTable) return;

    // 开始批量更新 - 禁用信号
    m_courseTable->setUpdatesEnabled(false);
    m_courseTable->blockSignals(true);

    // 清除现有课程内容
    for (int row = 0; row < m_courseTable->rowCount(); ++row) {
        for (int col = 1; col < m_courseTable->columnCount(); ++col) {
            QTableWidgetItem *item = m_courseTable->item(row, col);
            if (item) {
                item->setText("");
                item->setBackground(QBrush());
                item->setToolTip("");
                item->setData(Qt::UserRole, QVariant());
            }
        }
    }

    try {
        // 获取当前周的课程
        QList<CourseData> courses = m_courseManager->getCoursesByWeek(m_currentWeekStart);

        for (const CourseData &course : courses) {
            int day = course.dayOfWeek - 1;
            if (day < 0 || day > 6) continue;

            // 为课程的每一节都创建独立的显示
            for (int slot = course.startSlot - 1; slot < course.endSlot; ++slot) {
                if (slot < 0 || slot >= m_courseTable->rowCount()) continue;

                QTableWidgetItem *item = m_courseTable->item(slot, day + 1);
                if (!item) {
                    item = new QTableWidgetItem();
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    m_courseTable->setItem(slot, day + 1, item);
                }

                // 每节课都显示完整信息
                QString courseText = QString("%1\n@%2").arg(course.name).arg(course.location);
                item->setText(courseText);
                item->setData(Qt::UserRole, course.id); // 每节课都有相同的course id

                // 设置颜色和样式
                QColor courseColor = getCourseColor(course.courseType);
                item->setBackground(courseColor);

                // 字体颜色设置在这里：
                item->setForeground(Qt::black); // 改为黑色字体

                item->setTextAlignment(Qt::AlignCenter);

                // 设置工具提示
                QString tooltip = QString("课程: %1\n地点: %2\n时间: 第%3-%4节\n教师: %5\n类型: %6\n学分: %7")
                                      .arg(course.name)
                                      .arg(course.location)
                                      .arg(course.startSlot)
                                      .arg(course.endSlot)
                                      .arg(course.teacher.isEmpty() ? "未设置" : course.teacher)
                                      .arg(course.courseType)
                                      .arg(course.credits);

                if (course.examDate.isValid()) {
                    tooltip += QString("\n考试: %1").arg(course.examDate.toString("yyyy-MM-dd"));
                }
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
                item->setData(Qt::UserRole, QVariant());
            }
        }
    }

    // 显示搜索结果 - 使用修复后的逻辑
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

            // 使用相同的显示逻辑
            QString courseText;
            if (slot == course.startSlot - 1) {
                courseText = QString("%1\n@%2").arg(course.name).arg(course.location);
            } else {
                courseText = QString("↳ %1").arg(course.name);
            }

            item->setText(courseText);
            item->setData(Qt::UserRole, course.id);

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

    // 总是允许返回上一周
    m_currentWeekStart = m_currentWeekStart.addDays(-7);
    updateWeekDisplay();

    // 如果从最后一周返回到非最后一周，重新启用下一周导航
    if (m_canNavigateToNextWeek == false && !isLastWeek()) {
        m_canNavigateToNextWeek = true;

        // 重新启用下一周按钮
        QPushButton* nextBtn = findChild<QPushButton*>("navButton");
        if (nextBtn && nextBtn->text().contains("下一周")) {
            nextBtn->setEnabled(true);
            if (m_isDarkMode) {
                nextBtn->setStyleSheet(
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
                    "}"
                    );
            } else {
                nextBtn->setStyleSheet(
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
                    "}"
                    );
            }
        }
    }
}

void MainWindow::nextWeek()
{
    // 检查是否允许导航到下一周
    if (!m_canNavigateToNextWeek) {
        // 显示提示信息
        QMessageBox::information(this, "提示", "本学期已结束，无法查看后续周次！");
        return;
    }

    animateButton(qobject_cast<QPushButton*>(sender()));

    // 检查当前是否是最后一周
    bool wasLastWeek = isLastWeek();

    m_currentWeekStart = m_currentWeekStart.addDays(7);
    updateWeekDisplay();

    // 如果从非最后一周进入最后一周，显示动画并禁用后续导航
    if (!wasLastWeek && isLastWeek()) {
        m_canNavigateToNextWeek = false; // 禁用下一周导航
        QTimer::singleShot(500, this, &MainWindow::showSemesterEndAnimation);

        // 也可以在这里禁用下一周按钮
        QPushButton* nextBtn = findChild<QPushButton*>("navButton");
        if (nextBtn && nextBtn->text().contains("下一周")) {
            nextBtn->setEnabled(false);
            nextBtn->setStyleSheet(
                "QPushButton {"
                "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #95a5a6, stop:1 #7f8c8d);"
                "    color: #bdc3c7;"
                "    border: none;"
                "    padding: 12px 20px;"
                "    border-radius: 20px;"
                "    font-weight: bold;"
                "    font-size: 14px;"
                "    min-width: 100px;"
                "}"
                );
        }
    }
}

void MainWindow::goToThisWeek()
{
    animateButton(qobject_cast<QPushButton*>(sender()));
    QDate today = QDate::currentDate();
    m_currentWeekStart = today.addDays(1 - today.dayOfWeek());

    // 重置导航状态
    m_canNavigateToNextWeek = true;

    // 重新启用下一周按钮
    QPushButton* nextBtn = findChild<QPushButton*>("navButton");
    if (nextBtn && nextBtn->text().contains("下一周")) {
        nextBtn->setEnabled(true);
        if (m_isDarkMode) {
            nextBtn->setStyleSheet(
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
                "}"
                );
        } else {
            nextBtn->setStyleSheet(
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
                "}"
                );
        }
    }

    updateWeekDisplay();

    // 如果跳转到本周时是最后一周，显示动画并禁用导航
    if (isLastWeek()) {
        m_canNavigateToNextWeek = false;
        QTimer::singleShot(500, this, &MainWindow::showSemesterEndAnimation);

        // 禁用下一周按钮
        if (nextBtn && nextBtn->text().contains("下一周")) {
            nextBtn->setEnabled(false);
            nextBtn->setStyleSheet(
                "QPushButton {"
                "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #95a5a6, stop:1 #7f8c8d);"
                "    color: #bdc3c7;"
                "    border: none;"
                "    padding: 12px 20px;"
                "    border-radius: 20px;"
                "    font-weight: bold;"
                "    font-size: 14px;"
                "    min-width: 100px;"
                "}"
                );
        }
    }
}
void MainWindow::showCourseDetails(int row, int column)
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

    fadeInDialog(&dialog);
    fadeInDialog(&dialog);
    fadeInDialog(&dialog);
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
    // 在保存按钮的lambda表达式中添加验证
    connect(saveBtn, &QPushButton::clicked, [&, nameEdit, dayCombo, startSlotSpin, endSlotSpin, locationEdit, teacherEdit, typeCombo, creditSpin, startDateEdit, endDateEdit, examDateEdit]() {
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

        if (startSlotSpin->value() < 1 || endSlotSpin->value() > 10) {
            QMessageBox::warning(&dialog, "输入错误", "节次范围应为1-10！");
            return;
        }

        // 检查时间段冲突（简化版）
        int dayOfWeek = dayCombo->currentIndex() + 1;
        int startSlot = startSlotSpin->value();
        int endSlot = endSlotSpin->value();

        // 这里可以添加更复杂的时间冲突检测逻辑
        if (endSlot - startSlot + 1 > 4) {
            QMessageBox::warning(&dialog, "输入提示", "课程连续节次较多，请确认时间安排是否合理。");
        }

        CourseData course;
        course.name = nameEdit->text();
        course.dayOfWeek = dayOfWeek;
        course.startSlot = startSlot;
        course.endSlot = endSlot;
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
            populateCourseTable(); // 刷新表格
        } else {
            QMessageBox::critical(&dialog, "错误", "添加课程失败！");
        }
    });
    fadeInDialog(&dialog);
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
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(dialog);
    dialog->setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    dialog->setWindowModality(Qt::ApplicationModal);
    dialog->show();

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(300);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
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
// 在构造函数中初始化


// 添加切换主题的函数
void MainWindow::onToggleTheme()
{
    m_isDarkMode = !m_isDarkMode;

    if (m_isDarkMode) {
        applyDarkStyles();
        // 更新按钮文本
        QPushButton* themeBtn = findChild<QPushButton*>("themeButton");
        if (themeBtn) {
            themeBtn->setText("日间模式");
        }
    } else {
        applyLightStyles();
        // 更新按钮文本
        QPushButton* themeBtn = findChild<QPushButton*>("themeButton");
        if (themeBtn) {
            themeBtn->setText("夜间模式");
        }
    }

    // 刷新界面
    populateCourseTable();
}

// 设置学期按钮点击处理
void MainWindow::onSetSemester()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        animateButton(button);
    }
    showSemesterDialog();
}

// 显示学期设置对话框
void MainWindow::showSemesterDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("设置学期信息");
    dialog.setFixedSize(550, 600); // 调整大小以适应新样式

    // 使用与添加课程相同的对话框样式
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
    QLabel *iconLabel = new QLabel("🎓");
    iconLabel->setStyleSheet(R"(
        font-size: 28px;
        margin: 8px;
        padding: 8px;
        background: #f1f5f9;
        border-radius: 8px;
    )");

    // 标题
    QLabel *titleLabel = new QLabel("设置学期信息");
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

    // 当前学期信息
    QLabel *currentSemesterLabel = new QLabel(
        QString("当前学期: %1\n时间: %2 至 %3\n总周数: %4 周")
            .arg(m_courseManager->getCurrentSemester())
            .arg(m_courseManager->getSemesterStartDate().toString("yyyy年MM月dd日"))
            .arg(m_courseManager->getSemesterEndDate().toString("yyyy年MM月dd日"))
            .arg(m_courseManager->getSemesterWeeks())
        );
    currentSemesterLabel->setStyleSheet(R"(
        color: #475569;
        font-size: 13px;
        font-weight: 500;
        margin: 5px;
        padding: 12px;
        background: white;
        border: 1.5px solid #f1f5f9;
        border-radius: 10px;
    )");
    currentSemesterLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(currentSemesterLabel);

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
    QGroupBox *basicInfoGroup = new QGroupBox("📅 学期设置");
    QFormLayout *basicFormLayout = new QFormLayout(basicInfoGroup);
    basicFormLayout->setVerticalSpacing(15);
    basicFormLayout->setHorizontalSpacing(20);
    basicFormLayout->setContentsMargins(15, 20, 15, 20);

    // 学期名称
    QLineEdit *semesterEdit = new QLineEdit(m_courseManager->getCurrentSemester());
    semesterEdit->setPlaceholderText("例如: 2025-2026-1");
    semesterEdit->setStyleSheet(getInputStyle());
    semesterEdit->setMinimumHeight(40);

    // 开始日期
    QDateEdit *startDateEdit = new QDateEdit(m_courseManager->getSemesterStartDate());
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy年MM月dd日");
    startDateEdit->setStyleSheet(getDateEditStyle());

    // 结束日期
    QDateEdit *endDateEdit = new QDateEdit(m_courseManager->getSemesterEndDate());
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setDisplayFormat("yyyy年MM月dd日");
    endDateEdit->setStyleSheet(getDateEditStyle());

    // 总周数显示
    QLabel *weeksLabel = new QLabel();
    updateWeeksDisplay(weeksLabel, startDateEdit->date(), endDateEdit->date());

    // 连接日期变化信号更新周数显示
    auto updateWeeksFunc = [=]() {
        updateWeeksDisplay(weeksLabel, startDateEdit->date(), endDateEdit->date());
    };

    connect(startDateEdit, &QDateEdit::dateChanged, updateWeeksFunc);
    connect(endDateEdit, &QDateEdit::dateChanged, updateWeeksFunc);

    basicFormLayout->addRow("🎓 学期名称:", semesterEdit);
    basicFormLayout->addRow("🚀 开始日期:", startDateEdit);
    basicFormLayout->addRow("🏁 结束日期:", endDateEdit);
    basicFormLayout->addRow("📊 学期周数:", weeksLabel);

    formContainerLayout->addWidget(basicInfoGroup);
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
        padding: 10px; // 减小内边距
    )");
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    QPushButton *cancelBtn = new QPushButton("❌ 取消");
    QPushButton *saveBtn = new QPushButton("💾 保存设置");

    // 修改按钮样式，确保字体完全显示
    QString dialogButtonStyle = R"(
    QPushButton {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #64b5f6, stop:1 #42a5f5);
        color: white;
        border: none;
        padding: 10px 20px;
        border-radius: 10px;
        font-weight: 600;
        font-size: 14px;
        margin: 5px;
        min-width: 100px;
        min-height: 45px;
    }
    QPushButton:hover {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #42a5f5, stop:1 #2196f3);
    }
    QPushButton:pressed {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #2196f3, stop:1 #1976d2);
    }
)";

    cancelBtn->setStyleSheet(dialogButtonStyle + "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #dc2626);");
    saveBtn->setStyleSheet(dialogButtonStyle + "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #059669);");

    // 设置固定大小确保完全显示
    cancelBtn->setFixedSize(100, 45);  // 稍微加大宽度确保文字显示完整
    saveBtn->setFixedSize(100, 45);

    // 调整按钮布局：取消按钮在左，保存按钮在右
    buttonLayout->addStretch();  // 左侧弹簧
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addStretch();  // 右侧弹簧

    // 直接将按钮区域添加到主布局，不需要前面的弹簧
    mainLayout->addWidget(buttonWidget);

    // 连接信号
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&, semesterEdit, startDateEdit, endDateEdit]() {
        QString semesterName = semesterEdit->text().trimmed();
        if (semesterName.isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "请输入学期名称！");
            return;
        }

        if (startDateEdit->date() >= endDateEdit->date()) {
            QMessageBox::warning(&dialog, "输入错误", "开始日期必须早于结束日期！");
            return;
        }

        int totalWeeks = startDateEdit->date().daysTo(endDateEdit->date()) / 7 + 1;
        if (totalWeeks < 1 || totalWeeks > 30) {
            QMessageBox::warning(&dialog, "输入错误", "学期周数应在1-30周之间！");
            return;
        }

        // 保存学期设置并持久化开始/结束日期
        if (m_courseManager->setSemester(semesterName, startDateEdit->date(), endDateEdit->date())) {
            QMessageBox::information(&dialog, "成功",
                                     QString("学期设置已保存！\n\n学期: %1\n时间: %2 至 %3\n总周数: %4 周")
                                         .arg(semesterName)
                                         .arg(startDateEdit->date().toString("yyyy年MM月dd日"))
                                         .arg(endDateEdit->date().toString("yyyy年MM月dd日"))
                                         .arg(totalWeeks));
            dialog.accept();

            // 将当前周定位到学期开始周的周一，并重置导航
            QDate startMonday = startDateEdit->date().addDays(1 - startDateEdit->date().dayOfWeek());
            m_currentWeekStart = startMonday;
            m_canNavigateToNextWeek = true;

            updateWeekDisplay();
        } else {
            QMessageBox::critical(&dialog, "错误", "保存学期设置失败！");
        }
    });

    dialog.exec();
}

// 辅助函数：更新周数标签显示
void MainWindow::updateWeeksLabel(QLabel* label, const QDate& startDate, const QDate& endDate)
{
    int weeks = startDate.daysTo(endDate) / 7 + 1;
    label->setText(QString("%1 周").arg(weeks));

    // 根据周数设置颜色提示
    if (weeks < 10) {
        label->setStyleSheet(R"(
            color: #dc2626;
            font-size: 14px;
            font-weight: 600;
            padding: 8px 12px;
            background: #fef2f2;
            border-radius: 8px;
            border: 1px solid #fecaca;
        )");
    } else if (weeks > 25) {
        label->setStyleSheet(R"(
            color: #ea580c;
            font-size: 14px;
            font-weight: 600;
            padding: 8px 12px;
            background: #fff7ed;
            border-radius: 8px;
            border: 1px solid #fed7aa;
        )");
    } else {
        label->setStyleSheet(R"(
            color: #16a34a;
            font-size: 14px;
            font-weight: 600;
            padding: 8px 12px;
            background: #f0fdf4;
            border-radius: 8px;
            border: 1px solid #bbf7d0;
        )");
    }
}
void MainWindow::applyDarkStyles()
{
    QString darkStyleSheet = R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #1a1a1a, stop:1 #2d2d2d);
            font-family: 'Microsoft YaHei', Arial, sans-serif;
            color: #e0e0e0;
        }

        #titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #ffffff;
            padding: 10px;
            background: transparent;
        }

        #clockLabel {
            font-size: 16px;
            color: #b0b0b0;
            padding: 10px;
            background: rgba(45, 45, 45, 0.9);
            border-radius: 10px;
            border: 1px solid #404040;
        }

        #searchEdit {
            padding: 8px 12px;
            border: 2px solid #404040;
            border-radius: 20px;
            font-size: 14px;
            background: #2d2d2d;
            color: #e0e0e0;
        }

        #searchEdit:focus {
            border-color: #4a9eff;
            background: #333333;
        }

        #searchEdit::placeholder {
            color: #888888;
        }

        #searchButton, #navButton, #actionButton, #themeButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4a9eff, stop:1 #357abd);
            color: white;
            padding: 8px 12px;
            border: 2px solid #404040;
            border-radius: 20px;
            font-size: 14px;
        }

        #searchButton:hover, #navButton:hover, #actionButton:hover, #themeButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #357abd, stop:1 #2a5f9a);
        }

        #todayButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6b6b, stop:1 #e05555);
        }

        #todayButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e05555, stop:1 #c44c4c);
        }

        #weekLabel {
            font-size: 18px;
            font-weight: bold;
            color: #ffffff;
            padding: 15px;
            background: #333333;
            border-radius: 15px;
            margin: 5px;
            border: 1px solid #404040;
        }

        #courseTable {
            background: #2d2d2d;
            border-radius: 15px;
            gridline-color: #404040;
            font-size: 12px;
            color: #e0e0e0;
            border: 1px solid #404040;
        }

        #courseTable::item {
            padding: 8px;
            border: none;
            color: #ffffff;
        }

        #courseTable::item:selected {
            background: rgba(74, 158, 255, 0.3);
            border: 1px solid #4a9eff;
        }

        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #333333, stop:1 #2a2a2a);
            color: #e0e0e0;
            padding: 10px;
            border: 1px solid #404040;
            font-weight: bold;
        }

        QTableWidget QTableCornerButton::section {
            background: #333333;
            border: 1px solid #404040;
        }

        /* 时间列的样式 */
        #courseTable::item:first {
            background: #333333;
            color: #b0b0b0;
            border-right: 1px solid #404040;
        }
    )";

    setStyleSheet(darkStyleSheet);

    // 额外设置时间列的样式
    if (m_courseTable) {
        for (int row = 0; row < m_courseTable->rowCount(); ++row) {
            QTableWidgetItem *timeItem = m_courseTable->item(row, 0);
            if (timeItem) {
                timeItem->setBackground(m_isDarkMode ? QColor(51, 51, 51) : QColor(240, 240, 240));
                timeItem->setForeground(m_isDarkMode ? QColor(176, 176, 176) : QColor(80, 80, 80));
            }
        }
    }
}

void MainWindow::applyLightStyles()
{
    // 使用你原来的亮色样式
    applyStyles();
}
void MainWindow::updateWeeksDisplay(QLabel* label, const QDate& startDate, const QDate& endDate)
{
    int weeks = startDate.daysTo(endDate) / 7 + 1;
    label->setText(QString("%1 周").arg(weeks));

    // 根据周数设置颜色提示
    if (weeks < 10) {
        label->setStyleSheet(R"(
            color: #dc2626;
            font-size: 14px;
            font-weight: 600;
            padding: 8px 12px;
            background: #fef2f2;
            border-radius: 8px;
            border: 1px solid #fecaca;
        )");
    } else if (weeks > 25) {
        label->setStyleSheet(R"(
            color: #ea580c;
            font-size: 14px;
            font-weight: 600;
            padding: 8px 12px;
            background: #fff7ed;
            border-radius: 8px;
            border: 1px solid #fed7aa;
        )");
    } else {
        label->setStyleSheet(R"(
            color: #16a34a;
            font-size: 14px;
            font-weight: 600;
            padding: 8px 12px;
            background: #f0fdf4;
            border-radius: 8px;
            border: 1px solid #bbf7d0;
        )");
    }
}
bool MainWindow::isLastWeek() const
{
    QDate semesterEnd = m_courseManager->getSemesterEndDate();
    QDate lastWeekStart = semesterEnd.addDays(1 - semesterEnd.dayOfWeek()); // 最后一周的周一
    return m_currentWeekStart >= lastWeekStart;
}
void MainWindow::showSemesterEndAnimation()
{
    if (!isLastWeek()) return;

    // 创建半透明覆盖层
    QWidget *overlay = new QWidget(this);
    overlay->setGeometry(rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.7);");
    overlay->show();

    // 创建动画容器
    QWidget *animationContainer = new QWidget(overlay);
    animationContainer->setFixedSize(400, 300);
    animationContainer->move(width()/2 - 200, height()/2 - 150);
    animationContainer->setStyleSheet(R"(
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);
        border-radius: 20px;
        border: 3px solid #ffffff;
    )");
    animationContainer->show();

    QVBoxLayout *layout = new QVBoxLayout(animationContainer);

    // 图标动画
    QLabel *iconLabel = new QLabel("🎓");
    iconLabel->setStyleSheet(R"(
        font-size: 80px;
        background: transparent;
        margin: 20px;
    )");
    iconLabel->setAlignment(Qt::AlignCenter);

    // 文字提示
    QLabel *textLabel = new QLabel("本学期课程结束！");
    textLabel->setStyleSheet(R"(
        font-size: 24px;
        font-weight: bold;
        color: white;
        background: transparent;
        margin: 10px;
    )");
    textLabel->setAlignment(Qt::AlignCenter);

    QLabel *subTextLabel = new QLabel("感谢本学期的努力与付出 💪");
    subTextLabel->setStyleSheet(R"(
        font-size: 16px;
        color: rgba(255,255,255,0.9);
        background: transparent;
        margin: 10px;
    )");
    subTextLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);
    layout->addWidget(subTextLabel);

    // 设置初始透明度
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(animationContainer);
    animationContainer->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(0);

    // 创建动画序列
    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);

    // 淡入动画
    QPropertyAnimation *fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(800);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // 缩放动画
    QPropertyAnimation *scaleAnimation = new QPropertyAnimation(animationContainer, "geometry");
    scaleAnimation->setDuration(800);
    scaleAnimation->setStartValue(QRect(width()/2 - 50, height()/2 - 50, 100, 100));
    scaleAnimation->setEndValue(QRect(width()/2 - 200, height()/2 - 150, 400, 300));
    scaleAnimation->setEasingCurve(QEasingCurve::OutBack);

    animationGroup->addAnimation(fadeIn);
    animationGroup->addAnimation(scaleAnimation);

    // 图标浮动动画
    QPropertyAnimation *iconFloat = new QPropertyAnimation(iconLabel, "pos");
    iconFloat->setDuration(2000);
    iconFloat->setStartValue(iconLabel->pos());
    iconFloat->setKeyValueAt(0.25, iconLabel->pos() + QPoint(0, -10));
    iconFloat->setKeyValueAt(0.5, iconLabel->pos());
    iconFloat->setKeyValueAt(0.75, iconLabel->pos() + QPoint(0, 5));
    iconFloat->setEndValue(iconLabel->pos());
    iconFloat->setLoopCount(2);

    // 文字闪烁动画
    QPropertyAnimation *textBlink = new QPropertyAnimation(textLabel, "styleSheet");
    textBlink->setDuration(1500);
    textBlink->setKeyValueAt(0, "font-size: 24px; font-weight: bold; color: white;");
    textBlink->setKeyValueAt(0.5, "font-size: 24px; font-weight: bold; color: #FFD700;");
    textBlink->setKeyValueAt(1, "font-size: 24px; font-weight: bold; color: white;");
    textBlink->setLoopCount(2);

    // 连接动画结束信号
    connect(animationGroup, &QParallelAnimationGroup::finished, [=]() {
        // 开始第二阶段的动画
        QSequentialAnimationGroup *secondStage = new QSequentialAnimationGroup(this);

        // 等待一段时间
        QPauseAnimation *pause = new QPauseAnimation(2000);

        // 淡出动画
        QPropertyAnimation *fadeOut = new QPropertyAnimation(opacityEffect, "opacity");
        fadeOut->setDuration(1000);
        fadeOut->setStartValue(1);
        fadeOut->setEndValue(0);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);

        secondStage->addAnimation(pause);
        secondStage->addAnimation(fadeOut);

        connect(secondStage, &QSequentialAnimationGroup::finished, [=]() {
            overlay->deleteLater();
        });

        secondStage->start();
    });

    // 开始主要动画
    animationGroup->start();
    iconFloat->start();
    textBlink->start();

    // 播放音效（如果有的话）
    // QApplication::beep(); // 可以取消注释来播放系统提示音
}
void MainWindow::showCourseSearchDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("课程搜索");
    dialog.setFixedSize(800, 600);

    // 使用与学期设置相同的对话框样式
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
        QTableWidget {
            background: white;
            border: 1.5px solid #e2e8f0;
            border-radius: 8px;
            gridline-color: #f1f5f9;
            selection-background-color: #e3f2fd;
        }
        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #f1f5f9;
        }
        QTableWidget::item:selected {
            background: #bbdefb;
            color: #1e293b;
        }
        QHeaderView::section {
            background: #f8fafc;
            color: #475569;
            font-weight: 600;
            padding: 10px;
            border: none;
            border-right: 1px solid #e2e8f0;
            border-bottom: 1px solid #e2e8f0;
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
    QLabel *iconLabel = new QLabel("🔍");
    iconLabel->setStyleSheet(R"(
        font-size: 28px;
        margin: 8px;
        padding: 8px;
        background: #f1f5f9;
        border-radius: 8px;
    )");

    // 标题
    QLabel *titleLabel = new QLabel("课程搜索");
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

    // === 搜索条件区域 ===
    QGroupBox *searchGroup = new QGroupBox("📝 搜索条件");
    QFormLayout *searchLayout = new QFormLayout(searchGroup);
    searchLayout->setVerticalSpacing(12);
    searchLayout->setHorizontalSpacing(15);
    searchLayout->setContentsMargins(15, 20, 15, 20);

    // 搜索输入框
    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("输入课程名称、教师或地点进行搜索...");
    searchEdit->setStyleSheet(getInputStyle());
    searchEdit->setMinimumHeight(40);

    // 搜索按钮
    QPushButton *searchBtn = new QPushButton("🔍 搜索");
    searchBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #64b5f6, stop:1 #42a5f5);
            color: white;
            border: none;
            padding: 4px 5px;
            border-radius: 8px;
            font-weight: 600;
            font-size: 14px;
            min-width: 80px;
            min-height: 40px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #42a5f5, stop:1 #2196f3);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2196f3, stop:1 #1976d2);
        }
    )");
    searchBtn->setFixedSize(80, 30);

    QHBoxLayout *searchInputLayout = new QHBoxLayout();
    searchInputLayout->addWidget(searchEdit);
    searchInputLayout->addWidget(searchBtn);
    searchInputLayout->setAlignment(searchBtn, Qt::AlignVCenter);

    searchLayout->addRow("🔎 搜索关键词:", searchInputLayout);

    mainLayout->addWidget(searchGroup);

    // === 搜索结果区域 ===
    QGroupBox *resultGroup = new QGroupBox("📋 搜索结果");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    resultLayout->setContentsMargins(15, 20, 15, 20);

    // 课程表格
    QTableWidget *courseTable = new QTableWidget();
    courseTable->setColumnCount(7);
    courseTable->setHorizontalHeaderLabels({"课程名称", "教师", "地点", "时间", "周数", "类型", "学分"});
    courseTable->horizontalHeader()->setStretchLastSection(true);
    courseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    courseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    courseTable->setAlternatingRowColors(true);

    // 设置表格样式
    courseTable->setStyleSheet(R"(
        QTableWidget {
            alternate-background-color: #fafbfc;
        }
        QTableWidget::item {
            padding: 12px 8px;
        }
    )");

    // 设置列宽
    courseTable->setColumnWidth(0, 150); // 课程名称
    courseTable->setColumnWidth(1, 100); // 教师
    courseTable->setColumnWidth(2, 120); // 地点
    courseTable->setColumnWidth(3, 120); // 时间
    courseTable->setColumnWidth(4, 80);  // 周数
    courseTable->setColumnWidth(5, 80);  // 类型
    courseTable->setColumnWidth(6, 60);  // 学分

    resultLayout->addWidget(courseTable);

    mainLayout->addWidget(resultGroup);

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
    buttonLayout->setAlignment(Qt::AlignCenter);
    QPushButton *closeBtn = new QPushButton("❌ 关闭");
    QPushButton *viewDetailBtn = new QPushButton("👁️ 查看详情");

    QString dialogButtonStyle = R"(
    QPushButton {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #64b5f6, stop:1 #42a5f5);
        color: white;
        border: none;
        padding: 6px 10px;
        border-radius: 8px;
        font-weight: 600;
        font-size: 12px;
        margin: 5px;
        min-width: 50px;
        min-height: 20px;
    }
    QPushButton:hover {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #42a5f5, stop:1 #2196f3);
    }
    QPushButton:pressed {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #2196f3, stop:1 #1976d2);
    }
    )";

    closeBtn->setStyleSheet(dialogButtonStyle + "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #dc2626);");
    viewDetailBtn->setStyleSheet(dialogButtonStyle + "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #059669);");

    closeBtn->setFixedSize(90, 30);
    viewDetailBtn->setFixedSize(110, 30);

    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(viewDetailBtn);
    buttonLayout->addStretch();

    mainLayout->addWidget(buttonWidget);

    // === 功能实现 ===
    // 搜索按钮点击事件
    connect(searchBtn, &QPushButton::clicked, [&, searchEdit, courseTable]() {
        QString keyword = searchEdit->text().trimmed();
        if (keyword.isEmpty()) {
            // 如果关键词为空，显示所有课程
            displayAllCoursesInSearch(courseTable);
        } else {
            // 根据关键词搜索课程
            searchCoursesInDialog(keyword, courseTable);
        }
    });

    // 按回车键也可以搜索
    connect(searchEdit, &QLineEdit::returnPressed, searchBtn, &QPushButton::click);

    connect(viewDetailBtn, &QPushButton::clicked, [&, courseTable]() {
        QList<QTableWidgetItem*> selectedItems = courseTable->selectedItems();
        if (selectedItems.isEmpty()) {
            QMessageBox::information(&dialog, "提示", "请先选择要查看的课程！");
            return;
        }
        int row = selectedItems.first()->row();
        int cid = courseTable->item(row, 0)->data(Qt::UserRole).toInt();
        showCourseDetailInSearch(cid);
    });

    // 关闭按钮
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    connect(courseTable, &QTableWidget::cellDoubleClicked, [&, courseTable](int row, int column) {
        Q_UNUSED(column)
        int cid = courseTable->item(row, 0)->data(Qt::UserRole).toInt();
        showCourseDetailInSearch(cid);
    });

    QString initialKeyword = m_searchEdit ? m_searchEdit->text().trimmed() : QString();
    if (initialKeyword.isEmpty()) {
        displayAllCoursesInSearch(courseTable);
    } else {
        searchCoursesInDialog(initialKeyword, courseTable);
    }

    dialog.exec();
}

void MainWindow::displayAllCoursesInSearch(QTableWidget *table)
{
    table->setRowCount(0);

    QList<CourseData> allCourses = m_courseManager->getAllCourses();

    for (const CourseData &course : allCourses) {
        int row = table->rowCount();
        table->insertRow(row);

        // 时间信息
        QString timeInfo = QString("周%1 第%2-%3节").arg(course.dayOfWeek).arg(course.startSlot).arg(course.endSlot);

        // 周数信息
        QString weeksInfo = QString("%1周").arg(course.startDate.daysTo(course.endDate) / 7 + 1);

        {
            QTableWidgetItem *nameItem = new QTableWidgetItem(course.name);
            nameItem->setData(Qt::UserRole, course.id);
            table->setItem(row, 0, nameItem);
        }
        table->setItem(row, 1, new QTableWidgetItem(course.teacher.isEmpty() ? "未设置" : course.teacher));
        table->setItem(row, 2, new QTableWidgetItem(course.location));
        table->setItem(row, 3, new QTableWidgetItem(timeInfo));
        table->setItem(row, 4, new QTableWidgetItem(weeksInfo));
        table->setItem(row, 5, new QTableWidgetItem(course.courseType));
        table->setItem(row, 6, new QTableWidgetItem(QString::number(course.credits)));
    }

    // 调整列宽
    table->resizeColumnsToContents();
}

void MainWindow::searchCoursesInDialog(const QString &keyword, QTableWidget *table)
{
    table->setRowCount(0);

    QList<CourseData> allCourses = m_courseManager->getAllCourses();
    int foundCount = 0;

    for (const CourseData &course : allCourses) {
        // 在课程名称、教师、地点中搜索
        if (course.name.contains(keyword, Qt::CaseInsensitive) ||
            course.teacher.contains(keyword, Qt::CaseInsensitive) ||
            course.location.contains(keyword, Qt::CaseInsensitive)) {

            int row = table->rowCount();
            table->insertRow(row);

            // 时间信息
            QString timeInfo = QString("周%1 第%2-%3节").arg(course.dayOfWeek).arg(course.startSlot).arg(course.endSlot);

            // 周数信息
            QString weeksInfo = QString("%1周").arg(course.startDate.daysTo(course.endDate) / 7 + 1);

            {
                QTableWidgetItem *nameItem = new QTableWidgetItem(course.name);
                nameItem->setData(Qt::UserRole, course.id);
                table->setItem(row, 0, nameItem);
            }
            table->setItem(row, 1, new QTableWidgetItem(course.teacher.isEmpty() ? "未设置" : course.teacher));
            table->setItem(row, 2, new QTableWidgetItem(course.location));
            table->setItem(row, 3, new QTableWidgetItem(timeInfo));
            table->setItem(row, 4, new QTableWidgetItem(weeksInfo));
            table->setItem(row, 5, new QTableWidgetItem(course.courseType));
            table->setItem(row, 6, new QTableWidgetItem(QString::number(course.credits)));

            foundCount++;
        }
    }

    if (foundCount == 0) {
        QMessageBox::information(table, "搜索结果", QString("未找到包含 \"%1\" 的课程").arg(keyword));
    } else {
        // 调整列宽
        table->resizeColumnsToContents();
    }
}

void MainWindow::showCourseDetailInSearch(int courseId)
{
    CourseData course = m_courseManager->getCourseById(courseId);
    if (course.id == -1) return;

    QDialog dialog(this);
    dialog.setWindowTitle("课程详情");
    dialog.setFixedSize(500, 450);
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
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

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
    QLabel *iconLabel = new QLabel("📖");
    iconLabel->setStyleSheet(R"(
        font-size: 28px;
        margin: 8px;
        padding: 8px;
        background: #fef3c7;
        border-radius: 8px;
    )");
    QLabel *titleLabel = new QLabel("课程详情");
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

    QGroupBox *infoGroup = new QGroupBox("课程信息");
    QGridLayout *gridLayout = new QGridLayout(infoGroup);
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

    QLabel *periodLabel = new QLabel("<b>课程周期:</b>");
    QString periodText = QString("%1 ~ %2").arg(course.startDate.toString("yyyy年MM月dd日")).arg(course.endDate.toString("yyyy年MM月dd日"));
    QLabel *periodValue = new QLabel(periodText);
    periodValue->setStyleSheet("color: #7f8c8d; font-size: 13px;");
    gridLayout->addWidget(periodLabel, 3, 0);
    gridLayout->addWidget(periodValue, 3, 1, 1, 3);
    gridLayout->setHorizontalSpacing(15);
    gridLayout->setVerticalSpacing(12);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(3, 1);
    mainLayout->addWidget(infoGroup);

    if (course.examDate.isValid()) {
        QGroupBox *examGroup = new QGroupBox("📝 考试信息");
        QVBoxLayout *examLayout = new QVBoxLayout(examGroup);
        QHBoxLayout *examDateLayout = new QHBoxLayout();
        QLabel *examDateLabel = new QLabel("<b>考试日期:</b>");
        QLabel *examDateValue = new QLabel(course.examDate.toString("yyyy年MM月dd日 dddd"));
        examDateValue->setStyleSheet("color: #e74c3c; font-size: 14px; font-weight: bold;");
        examDateLayout->addWidget(examDateLabel);
        examDateLayout->addWidget(examDateValue);
        examDateLayout->addStretch();
        examLayout->addLayout(examDateLayout);
        mainLayout->addWidget(examGroup);
    }

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #bdc3c7;");
    mainLayout->addWidget(line);

    QWidget *buttonWidget = new QWidget();
    buttonWidget->setStyleSheet(R"(
        background: white;
        border: 1.5px solid #f1f5f9;
        border-radius: 12px;
        margin: 8px;
        padding: 15px;
    )");
    QHBoxLayout *btnLayout = new QHBoxLayout(buttonWidget);
    QPushButton *closeBtn = new QPushButton("❌ 关闭");
    closeBtn->setStyleSheet(getButtonStyle("#ef4444") + "\nQPushButton{padding:7px 14px; font-size:12px;}\n");
    closeBtn->setMinimumSize(60, 22);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    mainLayout->addWidget(buttonWidget);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}
