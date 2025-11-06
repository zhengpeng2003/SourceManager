#ifndef COURSEMANAGER_H
#define COURSEMANAGER_H

#include "course.h"
#include <QVector>
#include <QString>
#include <utility>

// TimeSlot: lessonIndex -> (startStr, endStr)
using TimeSlot = std::pair<int, std::pair<QString,QString>>;

class CourseManager {
public:
    CourseManager();

    bool loadCoursesFromFile(const QString& filePath);
    bool saveCoursesToFile(const QString& filePath);

    bool loadTimeSlotsFromFile(const QString& filePath);
    bool saveTimeSlotsToFile(const QString& filePath);

    void addCourse(const Course& c);

    // expand into per-week single-week entries (one Course per lesson per week)
    // returns false if invalid ranges or conflict detected (unless force==true)
    bool addCourseRange(const Course& base, int startLesson, int endLesson, int startWeek, int endWeek, bool force=false);

    QVector<Course> courses() const;
    QVector<Course> coursesForWeekAndDay(int week, int day) const;

    // conflict detection (overlap in same week, same weekday and same lesson index)
    bool hasConflict(const Course& c) const;

    void updateDerived();

    void clearAll();

    QVector<TimeSlot> timeSlots() const;

private:
    QVector<Course> m_courses;
    QVector<TimeSlot> m_timeslots;
};

#endif // COURSEMANAGER_H
