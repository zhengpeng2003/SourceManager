#include "mainwindow.h"
#include "course.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <qstandardpaths.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_currentWeek(1)
{
    setupUI();

    // load files
    QString dfile = dataFilePath();
    QString tfile = timesFilePath();
    m_manager.loadTimeSlotsFromFile(tfile);
    m_manager.loadCoursesFromFile(dfile);

    // compute current week based on semester start (same logic as original)
    QDate now = QDate::currentDate();
    QDate semester(2025,9,1);
    int w = (semester.daysTo(now)/7) + 1;
    if (w < 1) w = 1; if (w>20) w=20;
    m_currentWeek = w;

    // start timers
    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start(1000);

    refreshCourseTable();
    updateClock();
}

MainWindow::~MainWindow() {
    // save on close
    m_manager.saveCoursesToFile(dataFilePath());
    m_manager.saveTimeSlotsToFile(timesFilePath());
}

QString MainWindow::dataFilePath() const {
    // prefer AppDataLocation to be writable when packaged
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) dir = QDir::currentPath(); // fallback
    QDir().mkpath(dir);
    return QDir(dir).absoluteFilePath("courses.txt");
}

QString MainWindow::timesFilePath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) dir = QDir::currentPath();
    QDir().mkpath(dir);
    return QDir(dir).absoluteFilePath("timeslots.txt");
}

void MainWindow::setupUI() {
    setWindowTitle("智能课程表（TXT版）");
    resize(1000,720);
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLay = new QVBoxLayout(central);
    mainLay->setContentsMargins(0,0,0,0);
    mainLay->setSpacing(0);

    // header with time
    QWidget* header = new QWidget;
    header->setFixedHeight(64);
    header->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #EAF4FF, stop:1 #FFFFFF);");
    QHBoxLayout* hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(16,6,16,6);

    m_timeLabel = new QLabel("00:00");
    m_timeLabel->setStyleSheet("font-size:18px; font-weight:600; color:#1E3A8A;");
    m_dateLabel = new QLabel("----");
    m_dateLabel->setStyleSheet("color:#666;");

    hlay->addWidget(m_timeLabel);
    hlay->addStretch();
    hlay->addWidget(m_dateLabel);

    mainLay->addWidget(header);

    // week header
    QWidget* weekbar = new QWidget;
    weekbar->setFixedHeight(88);
    QHBoxLayout* wlay = new QHBoxLayout(weekbar);
    wlay->setContentsMargins(16,8,16,8);

    m_weekLabel = new QLabel("第1周");
    m_weekLabel->setStyleSheet("font-size:20px; font-weight:700;");
    m_rangeLabel = new QLabel("学期 2025-09-01 起");
    m_rangeLabel->setStyleSheet("color:#666;");

    wlay->addWidget(m_weekLabel);
    wlay->addWidget(m_rangeLabel);
    wlay->addStretch();

    m_prevBtn = new QPushButton("◀ 上周");
    m_nextBtn = new QPushButton("下周 ▶");
    m_addBtn = new QPushButton("+ 添加");
    m_editTimesBtn = new QPushButton("设置上课时间");
    m_refreshBtn = new QPushButton("刷新");

    wlay->addWidget(m_prevBtn);
    wlay->addWidget(m_nextBtn);
    wlay->addWidget(m_addBtn);
    wlay->addWidget(m_editTimesBtn);
    wlay->addWidget(m_refreshBtn);

    connect(m_prevBtn, &QPushButton::clicked, this, &MainWindow::prevWeek);
    connect(m_nextBtn, &QPushButton::clicked, this, &MainWindow::nextWeek);
    connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAddCourse);
    connect(m_editTimesBtn, &QPushButton::clicked, this, &MainWindow::onEditTimeSlots);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefresh);

    mainLay->addWidget(weekbar);

    // table area
    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_tableWidget = new QWidget;
    m_scroll->setWidget(m_tableWidget);
    mainLay->addWidget(m_scroll);

    // footer (go to this week)
    QWidget* footer = new QWidget;
    footer->setFixedHeight(36);
    QHBoxLayout* fl = new QHBoxLayout(footer);
    QPushButton* gotoBtn = new QPushButton("本周");
    connect(gotoBtn, &QPushButton::clicked, this, &MainWindow::goToThisWeek);
    fl->addStretch();
    fl->addWidget(gotoBtn);
    fl->addStretch();
    mainLay->addWidget(footer);
}

void MainWindow::updateClock() {
    QDateTime now = QDateTime::currentDateTime();
    m_timeLabel->setText(now.toString("hh:mm"));
    m_dateLabel->setText(now.toString("yyyy年MM月dd日"));
}

void MainWindow::refreshCourseTable() {
    // clear existing layout & widgets safely
    QLayout* old = m_tableWidget->layout();
    if (old) {
        QLayoutItem* it;
        while ((it = old->takeAt(0)) != nullptr) {
            if (it->widget()) it->widget()->deleteLater();
            delete it;
        }
        delete old;
    }

    QVBoxLayout* vlay = new QVBoxLayout(m_tableWidget);
    vlay->setContentsMargins(8,8,8,8);
    vlay->setSpacing(8);

    // week header update
    m_weekLabel->setText(QString("第%1周").arg(m_currentWeek));
    QDate ws = QDate(2025,9,1).addDays((m_currentWeek-1)*7);
    m_rangeLabel->setText(ws.toString("M/d") + " - " + ws.addDays(6).toString("M/d"));

    // top row: day names
    QWidget* topRow = new QWidget;
    topRow->setFixedHeight(44);
    QHBoxLayout* tlay = new QHBoxLayout(topRow);
    tlay->setContentsMargins(0,0,0,0);
    tlay->setSpacing(6);

    QLabel* timeCol = new QLabel("时间");
    timeCol->setFixedWidth(90);
    tlay->addWidget(timeCol);

    QStringList days = {"周一","周二","周三","周四","周五","周六","周日"};
    for (int i=0;i<7;i++) {
        QLabel* dl = new QLabel(days[i]);
        dl->setAlignment(Qt::AlignCenter);
        dl->setStyleSheet("font-weight:600;");
        tlay->addWidget(dl);
    }
    vlay->addWidget(topRow);

    // load timeslots from manager (use getter)
    QVector<TimeSlot> tvec = m_manager.timeSlots().toVector(); // convert to QVector<TimeSlot>

    // if empty, build defaults
    if (tvec.isEmpty()) {
        QStringList defaults = {"08:00|08:45","08:55|09:40","09:50|10:35","10:45|11:30",
                                "14:00|14:45","14:55|15:40","15:50|16:35","16:45|17:30",
                                "19:00|19:45","19:55|20:40","20:50|21:35","21:45|22:30"};
        for (int i=0;i<12;i++){
            tvec.append({i+1, {defaults[i].split("|")[0], defaults[i].split("|")[1]}});
        }
    }

    // rows: one per lesson
    for (int idx=0; idx < tvec.size(); ++idx) {
        int lessonNo = tvec[idx].first;
        QString s = tvec[idx].second.first;
        QString e = tvec[idx].second.second;

        QWidget* row = new QWidget;
        row->setFixedHeight(72);
        QHBoxLayout* rlay = new QHBoxLayout(row);
        rlay->setContentsMargins(0,6,0,6);
        rlay->setSpacing(6);

        QLabel* timeLabel = new QLabel(QString("第%1节\n%2-%3").arg(lessonNo).arg(s).arg(e));
        timeLabel->setFixedWidth(90);
        timeLabel->setAlignment(Qt::AlignCenter);
        timeLabel->setStyleSheet("color:#666;");
        rlay->addWidget(timeLabel);

        // for days 1..7
        for (int day=1; day<=7; ++day) {
            QWidget* cell = new QWidget;
            QVBoxLayout* cellL = new QVBoxLayout(cell);
            cellL->setContentsMargins(6,6,6,6);

            // get list for this week/day
            QVector<Course> list = m_manager.coursesForWeekAndDay(m_currentWeek, day);

            // find a course with lessonNo
            bool placed = false;
            for (const Course& c : list) {
                if (c.lessonIndex() == lessonNo) {
                    // create button from value copy (prevent pointer issues)
                    Course copy = c; // copy for lambda
                    QPushButton* btn = new QPushButton;
                    btn->setText(copy.name() + "\n@" + copy.location());
                    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                    btn->setStyleSheet("background:#4ECDC4; color:white; border-radius:6px; font-weight:600;");
                    connect(btn, &QPushButton::clicked, this, [this, copy](){
                        // show dialog using copy (safe)
                        QDialog dlg(this);
                        dlg.setWindowTitle(copy.name());
                        dlg.setFixedSize(360,300);
                        QVBoxLayout* lay = new QVBoxLayout(&dlg);
                        QLabel* title = new QLabel(copy.name());
                        title->setStyleSheet("font-size:16px; font-weight:700;");
                        QLabel* t1 = new QLabel(QString("时间: 第%1节 %2-%3").arg(copy.lessonIndex()).arg(copy.startTime().toString("hh:mm")).arg(copy.endTime().toString("hh:mm")));
                        QLabel* t2 = new QLabel(QString("地点: %1").arg(copy.location()));
                        QLabel* t3 = new QLabel(QString("教师: %1").arg(copy.teacher()));
                        QLabel* exam = new QLabel(QString("考试日: %1").arg(copy.examDate().isValid() ? copy.examDate().toString("yyyy-MM-dd") : QString("--")));
                        QProgressBar* pg = new QProgressBar;
                        pg->setRange(0,100);
                        if (copy.examDate().isValid()) {
                            QDate sem(2025,9,1);
                            double tot = sem.daysTo(copy.examDate());
                            double passed = sem.daysTo(QDate::currentDate());
                            int pct = 0;
                            if (tot>0 && passed>0) pct = qBound(0, int((passed/tot)*100.0), 100);
                            pg->setValue(pct);
                            pg->setFormat(QString("进度: %1%").arg(pct));
                        } else {
                            pg->setValue(0);
                            pg->setFormat("进度: --");
                        }
                        lay->addWidget(title);
                        lay->addWidget(t1);
                        lay->addWidget(t2);
                        lay->addWidget(t3);
                        lay->addWidget(exam);
                        lay->addWidget(pg);
                        QPushButton* ok = new QPushButton("确定");
                        connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
                        lay->addStretch();
                        lay->addWidget(ok, 0, Qt::AlignCenter);

                        // fade-in
                        dlg.setWindowOpacity(0.0);
                        QPropertyAnimation* anim = new QPropertyAnimation(&dlg, "windowOpacity");
                        anim->setDuration(300);
                        anim->setStartValue(0.0);
                        anim->setEndValue(1.0);
                        anim->start(QAbstractAnimation::DeleteWhenStopped);
                        dlg.exec();
                    });

                    cellL->addWidget(btn);
                    placed = true;
                    break;
                }
            }

            if (!placed) {
                QLabel* empty = new QLabel;
                empty->setStyleSheet("background:transparent;");
                cellL->addWidget(empty);
            }

            rlay->addWidget(cell);
        }

        vlay->addWidget(row);
    }

    vlay->addStretch();
    m_tableWidget->adjustSize();
}

void MainWindow::onAddCourse() {
    QDialog dlg(this);
    dlg.setWindowTitle("添加课程（支持多周多节生成）");
    dlg.setModal(true);
    dlg.setFixedSize(460,560);

    QVBoxLayout* main = new QVBoxLayout(&dlg);
    main->setContentsMargins(14,14,14,14);
    main->setSpacing(10);

    QFormLayout* form = new QFormLayout;
    QLineEdit* nameEdit = new QLineEdit;
    QDateEdit* dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    QLineEdit* teacherEdit = new QLineEdit;
    QLineEdit* locEdit = new QLineEdit;

    // lesson range
    QComboBox* startLesson = new QComboBox;
    QComboBox* endLesson = new QComboBox;
    // populate from times file
    QVector<TimeSlot> tvec = m_manager.timeSlots();
    if (tvec.isEmpty()) {
        for (int i=1;i<=12;i++) {
            startLesson->addItem(QString("第%1节 %2-%3").arg(i).arg("08:00").arg("08:45"), i);
            endLesson->addItem(QString("第%1节 %2-%3").arg(i).arg("08:00").arg("08:45"), i);
        }
    } else {
        for (const TimeSlot& p : tvec) {
            int n = p.first;
            startLesson->addItem(QString("第%1节 %2-%3").arg(n).arg(p.second.first).arg(p.second.second), n);
            endLesson->addItem(QString("第%1节 %2-%3").arg(n).arg(p.second.first).arg(p.second.second), n);
        }
    }

    QComboBox* startWeek = new QComboBox;
    QComboBox* endWeek = new QComboBox;
    for (int i=1;i<=20;i++) {
        startWeek->addItem(QString("第%1周").arg(i), i);
        endWeek->addItem(QString("第%1周").arg(i), i);
    }
    startWeek->setCurrentIndex(m_currentWeek-1);
    endWeek->setCurrentIndex(m_currentWeek-1);

    form->addRow("课程名称:", nameEdit);
    form->addRow("考试日期:", dateEdit);
    form->addRow("任课教师:", teacherEdit);
    form->addRow("地点:", locEdit);
    form->addRow("开始节:", startLesson);
    form->addRow("结束节:", endLesson);
    form->addRow("开始周:", startWeek);
    form->addRow("结束周:", endWeek);

    main->addLayout(form);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    main->addWidget(box);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "错误", "课程名称不能为空");
            return;
        }
        Course base;
        base.setName(name);
        base.setExamDate(dateEdit->date());
        base.setTeacher(teacherEdit->text().trimmed());
        base.setLocation(locEdit->text().trimmed());
        base.setWeekDay(QDate::currentDate().dayOfWeek()); // default to today weekday

        int ls = startLesson->currentData().toInt();
        int le = endLesson->currentData().toInt();
        int sw = startWeek->currentData().toInt();
        int ew = endWeek->currentData().toInt();

        bool ok = m_manager.addCourseRange(base, ls, le, sw, ew, false);
        if (!ok) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "冲突", "检测到时间冲突，是否强制添加（覆盖）？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                bool forced = m_manager.addCourseRange(base, ls, le, sw, ew, true);
                if (!forced) {
                    QMessageBox::warning(this, "错误", "强制添加失败");
                    return;
                }
            } else {
                return;
            }
        }

        m_manager.saveCoursesToFile(dataFilePath());
        refreshCourseTable();
        QMessageBox::information(this, "成功", "课程已添加");
    }
}

void MainWindow::onEditTimeSlots() {
    QDialog dlg(this);
    dlg.setWindowTitle("设置每节课时间（12节）");
    dlg.setModal(true);
    dlg.setFixedSize(420,600);

    QVBoxLayout* main = new QVBoxLayout(&dlg);
    QScrollArea* scroll = new QScrollArea;
    QWidget* container = new QWidget;
    QFormLayout* form = new QFormLayout(container);

    QVector<TimeSlot> tvec = m_manager.timeSlots();
    if (tvec.isEmpty()) {
        QStringList defaults = {"08:00|08:45","08:55|09:40","09:50|10:35","10:45|11:30",
                                "14:00|14:45","14:55|15:40","15:50|16:35","16:45|17:30",
                                "19:00|19:45","19:55|20:40","20:50|21:35","21:45|22:30"};
        for (int i=0;i<12;i++) tvec.append({i+1,{defaults[i].split("|")[0], defaults[i].split("|")[1]}});
    }

    QVector<QTimeEdit*> starts, ends;
    for (int i=0;i<tvec.size();++i) {
        int n = tvec[i].first;
        QTimeEdit* sedit = new QTimeEdit(QTime::fromString(tvec[i].second.first,"hh:mm"));
        QTimeEdit* eedit = new QTimeEdit(QTime::fromString(tvec[i].second.second,"hh:mm"));
        QWidget* row = new QWidget;
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0,0,0,0);
        QLabel* lbl = new QLabel(QString("第%1节").arg(n));
        lbl->setFixedWidth(60);
        rl->addWidget(lbl);
        rl->addWidget(sedit);
        rl->addWidget(new QLabel("-"));
        rl->addWidget(eedit);
        form->addRow(row);
        starts.append(sedit);
        ends.append(eedit);
    }

    container->setLayout(form);
    scroll->setWidget(container);
    scroll->setWidgetResizable(true);
    main->addWidget(scroll);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    main->addWidget(box);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        QFile outf(timesFilePath());
        if (!outf.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "错误", "无法写入 timeslots.txt");
            return;
        }
        QTextStream out(&outf);
        for (int i=0;i<starts.size();++i) {
            out << (i+1) << "|" << starts[i]->time().toString("hh:mm") << "|" << ends[i]->time().toString("hh:mm") << "\n";
        }
        outf.close();
        m_manager.loadTimeSlotsFromFile(timesFilePath());
        refreshCourseTable();
        QMessageBox::information(this, "保存", "已保存每节课时间设置");
    }
}

void MainWindow::onRefresh() {
    m_manager.loadCoursesFromFile(dataFilePath());
    m_manager.loadTimeSlotsFromFile(timesFilePath());
    refreshCourseTable();
}

void MainWindow::prevWeek() {
    if (m_currentWeek > 1) { m_currentWeek--; refreshCourseTable(); }
}
void MainWindow::nextWeek() {
    if (m_currentWeek < 20) { m_currentWeek++; refreshCourseTable(); }
}
void MainWindow::goToThisWeek() {
    QDate now = QDate::currentDate();
    QDate sem(2025,9,1);
    int w = (sem.daysTo(now)/7) + 1;
    if (w<1) w=1; if (w>20) w=20;
    m_currentWeek = w;
    refreshCourseTable();
}
