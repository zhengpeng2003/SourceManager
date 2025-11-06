#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include "coursemanager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddCourse();
    void onEditTimeSlots();
    void onRefresh();
    void updateClock();
    void prevWeek();
    void nextWeek();
    void goToThisWeek();

private:
    void setupUI();
    void refreshCourseTable();
    QString dataFilePath() const;
    QString timesFilePath() const;

    CourseManager m_manager; // member manager (prevents dangling)
    int m_currentWeek;

    // UI widgets
    QLabel* m_timeLabel;
    QLabel* m_dateLabel;
    QLabel* m_weekLabel;
    QLabel* m_rangeLabel;

    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QPushButton* m_addBtn;
    QPushButton* m_editTimesBtn;
    QPushButton* m_refreshBtn;

    QScrollArea* m_scroll;
    QWidget* m_tableWidget;

    QTimer* m_clockTimer;
};

#endif // MAINWINDOW_H
