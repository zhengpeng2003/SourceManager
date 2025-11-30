#ifndef COURSEMANAGER_H
#define COURSEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDate>
#include <QColor>

class CourseData
{
public:
    int id;
    QString name;
    int dayOfWeek; // 1-7 对应周一到周日
    int startSlot;
    int endSlot;
    QString location;
    QDate startDate;
    QDate endDate;
    QString teacher;
    QDate examDate;
    QString courseType;
    double credits;

    CourseData() : id(-1), dayOfWeek(1), startSlot(1), endSlot(1), credits(0) {}
    CourseData(const QString& name, int day, int start, int end, const QString& loc,
               const QDate& startDate, const QDate& endDate, const QString& teacher = "",
               const QDate& examDate = QDate(), const QString& courseType = "必修", double credits = 0)
        : id(-1), name(name), dayOfWeek(day), startSlot(start), endSlot(end),
        location(loc), startDate(startDate), endDate(endDate), teacher(teacher),
        examDate(examDate), courseType(courseType), credits(credits) {}
};

class CourseManager : public QObject
{
    Q_OBJECT

public:
    explicit CourseManager(QObject *parent = nullptr);
    ~CourseManager();
    int getSemesterWeeks() const;
    bool initDatabase();
    bool addCourse(const CourseData &course);
    bool updateCourse(const CourseData &course);
    bool deleteCourse(int id);
    QList<CourseData> getCoursesByWeek(const QDate &date);
    QList<CourseData> getAllCourses();
    QList<CourseData> searchCourses(const QString &keyword);
    CourseData getCourseById(int id);

    bool setCurrentSemester(const QString &semester);
    bool setSemester(const QString &name, const QDate &start, const QDate &end);
    QString getCurrentSemester() const;
    QDate getSemesterStartDate() const;
    QDate getSemesterEndDate() const;

    bool exportToCsv(const QString &filePath);
    bool importFromCsv(const QString &filePath);
    bool createBackup();

private:
    QSqlDatabase m_db;
    QString m_currentSemester;

    bool createTables();
    bool upgradeDatabase();
};

#endif // COURSEMANAGER_H
