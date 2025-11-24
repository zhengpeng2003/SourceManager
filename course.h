#ifndef COURSE_H
#define COURSE_H

#include <QString>
#include <QTime>
#include <QDate>

class Course {
public:
    Course();

    // ID from DB (if -1 not yet saved)
    int id() const;
    void setId(int);

    QString name() const;
    void setName(const QString&);

    QDate examDate() const;
    void setExamDate(const QDate&);

    QString teacher() const;
    void setTeacher(const QString&);

    QString location() const;
    void setLocation(const QString&);

    // lesson timing stored as lesson index (1..12)
    int lessonIndex() const;
    void setLessonIndex(int idx);

    QTime startTime() const;
    void setStartTime(const QTime&);
    QTime endTime() const;
    void setEndTime(const QTime&);

    int weekDay() const;    // 1..7 (Qt: 1=Monday)
    void setWeekDay(int);

    int startWeek() const;
    void setStartWeek(int);
    int endWeek() const;
    void setEndWeek(int);

    int remainingDays() const;
    void computeDerived() const;

private:
    int m_id;
    QString m_name;
    QDate m_examDate;
    QString m_teacher;
    QString m_location;
    int m_lessonIndex;
    QTime m_startTime;
    QTime m_endTime;
    int m_weekDay;
    int m_startWeek;
    int m_endWeek;

    mutable int m_remainingDays;
};

#endif // COURSE_H
