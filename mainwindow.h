#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDate>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include "coursemanager.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qtablewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

    QLabel *createFormLabel(const QString &text);
    void showErrorMessage(const QString &message);
    void updateWeeksLabel(QLabel *label, const QDate &startDate, const QDate &endDate);
private slots:
    void onAddCourse();
    void onRefresh();
    void onSearch();
    void onExport();
    void onBackup();
    void updateClock();
    void prevWeek();
    void nextWeek();
    void goToThisWeek();
    void onToggleTheme();
    void onSetSemester();
    void showCourseSearchDialog();

private:
    void updateWeeksDisplay(QLabel* label, const QDate& startDate, const QDate& endDate);
    void showSemesterDialog();
    Ui::MainWindow *ui;
    CourseManager *m_courseManager;
    QTimer *m_clockTimer;
    QDate m_currentWeekStart;
    //

    void displayAllCoursesInSearch(QTableWidget* table);
    void searchCoursesInDialog(const QString& keyword, QTableWidget* table);
    void showCourseDetailInSearch(int courseId);
    // UI组件指针
    QTableWidget *m_courseTable;
    QLabel *m_weekLabel;
    QLabel *m_clockLabel;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_backupBtn;

    void setupUI();
    void updateWeekDisplay();
    void populateCourseTable();
    void applyStyles();
    void showAddCourseDialog();
    void showEditCourseDialog(int courseId);
    void showDeleteConfirmation(int courseId);
    void showCourseDetails(int row, int column);

    // 动画效果
    void animateButton(QWidget *button);
    void animateTableRow(int row);
    void fadeInWidget(QWidget *widget);
    void pulseAnimation(QWidget *widget);
    void shakeWidget(QWidget *widget);
    void fadeInDialog(QDialog *dialog);


    // 工具函数
    QColor getCourseColor(const QString &courseType);
    void applyDarkStyles();
    void applyLightStyles();
private:
    bool m_canNavigateToNextWeek;
    void showSemesterEndAnimation();
    bool isLastWeek() const;
    bool m_isDarkMode;
    QString getInputStyle() {
        return R"(
        QLineEdit {
            padding: 12px 16px;
            border: 1.5px solid #e1e5e9;
            border-radius: 10px;
            font-size: 14px;
            background: #ffffff;
            color: #2c3e50;
            selection-background-color: #e3f2fd;
        }
        QLineEdit:focus {
            border-color: #90caf9;
            background: #fafbfc;
            box-shadow: 0 0 0 3px rgba(144, 202, 249, 0.1);
        }
        QLineEdit:placeholder {
            color: #a0a4a8;
            font-style: italic;
        }
    )";
    }

    QString getComboBoxStyle() {
        return R"(
        QComboBox {
            padding: 12px 16px;
            border: 1.5px solid #e1e5e9;
            border-radius: 10px;
            background: #ffffff;
            font-size: 14px;
            color: #2c3e50;
            min-height: 20px;
        }
        QComboBox:focus {
            border-color: #90caf9;
            box-shadow: 0 0 0 3px rgba(144, 202, 249, 0.1);
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 4px solid #90a4ae;
            width: 0px;
            height: 0px;
        }
        QComboBox QAbstractItemView {
            border: 1.5px solid #e1e5e9;
            border-radius: 8px;
            background: white;
            selection-background-color: #e3f2fd;
            outline: none;
        }
    )";
    }

    QString getSpinBoxStyle() {
        return R"(
        QSpinBox, QDoubleSpinBox {
            padding: 12px 16px;
            border: 1.5px solid #e1e5e9;
            border-radius: 10px;
            background: #ffffff;
            font-size: 14px;
            color: #2c3e50;
            min-width: 80px;
        }
        QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: #90caf9;
            box-shadow: 0 0 0 3px rgba(144, 202, 249, 0.1);
        }
        QSpinBox::up-button, QSpinBox::down-button,
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            border: none;
            background: #f8f9fa;
            border-radius: 4px;
            width: 20px;
        }
    )";
    }

    QString getDateEditStyle() {
        return R"(
        QDateEdit {
            padding: 12px 16px;
            border: 1.5px solid #e1e5e9;
            border-radius: 10px;
            background: #ffffff;
            font-size: 14px;
            color: #2c3e50;
        }
        QDateEdit:focus {
            border-color: #90caf9;
            box-shadow: 0 0 0 3px rgba(144, 202, 249, 0.1);
        }
        QDateEdit::drop-down {
            border: none;
            width: 25px;
        }
        QDateEdit::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 4px solid #90a4ae;
            width: 0px;
            height: 0px;
        }
    )";
    }

    QString getButtonStyle(const QString& color = "#64b5f6") {
        return QString(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 %1, stop:1 #42a5f5);
            color: white;
            border: none;
            padding: 14px 28px;
            border-radius: 10px;
            font-weight: 600;
            font-size: 14px;
            margin: 5px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #42a5f5, stop:1 #2196f3);
            transform: translateY(-1px);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2196f3, stop:1 #1976d2);
            transform: translateY(0px);
        }
        QPushButton:disabled {
            background: #b0bec5;
            color: #e0e0e0;
        }
    )").arg(color);
    }
    void updateWeeksLabel(QLabel *, QDate, QDate);
};

#endif // MAINWINDOW_H
