#include "coursemanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

CourseManager::CourseManager() {}

bool CourseManager::loadCoursesFromFile(const QString& filePath) {
    m_courses.clear();
    QFile f(filePath);
    if (!f.exists()) return true; // ok, no file yet
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open courses file for read:" << filePath;
        return false;
    }
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        Course c = Course::fromLine(line);
        m_courses.append(c);
    }
    f.close();
    updateDerived();
    return true;
}

bool CourseManager::saveCoursesToFile(const QString& filePath) {
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open courses file for write:" << filePath;
        return false;
    }
    QTextStream out(&f);
    for (const Course& c : m_courses) {
        out << c.toLine() << "\n";
    }
    f.close();
    return true;
}

bool CourseManager::loadTimeSlotsFromFile(const QString& filePath) {
    m_timeslots.clear();
    QFile f(filePath);
    if (!f.exists()) {
        // default 12 slots
        QStringList defaults = {"08:00|08:45","08:55|09:40","09:50|10:35","10:45|11:30",
                                "14:00|14:45","14:55|15:40","15:50|16:35","16:45|17:30",
                                "19:00|19:45","19:55|20:40","20:50|21:35","21:45|22:30"};
        for (int i=0;i<12;i++){
            QString s = defaults[i].split("|")[0];
            QString e = defaults[i].split("|")[1];
            m_timeslots.append({i+1, {s,e}});
        }
        return true;
    }
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open times file for read:" << filePath;
        return false;
    }
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList p = line.split("|");
        if (p.size() < 3) continue;
        int lesson = p[0].toInt();
        m_timeslots.append({lesson, {p[1], p[2]}});
    }
    f.close();
    return true;
}

bool CourseManager::saveTimeSlotsToFile(const QString& filePath) {
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open times file for write:" << filePath;
        return false;
    }
    QTextStream out(&f);
    for (const TimeSlot& t : m_timeslots) {
        out << t.first << "|" << t.second.first << "|" << t.second.second << "\n";
    }
    f.close();
    return true;
}

void CourseManager::addCourse(const Course& c) {
    m_courses.append(c);
}

bool CourseManager::hasConflict(const Course& c) const {
    for (const Course& ex : m_courses) {
        // week overlap
        if (ex.endWeek() < c.startWeek() || ex.startWeek() > c.endWeek()) continue;
        if (ex.weekDay() != c.weekDay()) continue;
        if (ex.lessonIndex() == c.lessonIndex()) return true;
    }
    return false;
}

bool CourseManager::addCourseRange(const Course& base, int startLesson, int endLesson, int startWeek, int endWeek, bool force) {
    if (startLesson > endLesson || startWeek > endWeek) return false;
    QVector<Course> created;
    for (int w = startWeek; w <= endWeek; ++w) {
        for (int ls = startLesson; ls <= endLesson; ++ls) {
            Course c = base;
            c.setLessonIndex(ls);
            // store as single-week entry to make per-week display simpler
            c.setStartWeek(w);
            c.setEndWeek(w);
            c.setWeekDay(base.weekDay());
            // set timeslot if available
            for (const TimeSlot& t : m_timeslots) {
                if (t.first == ls) {
                    c.setStartTime(QTime::fromString(t.second.first, "hh:mm"));
                    c.setEndTime(QTime::fromString(t.second.second, "hh:mm"));
                    break;
                }
            }
            c.computeDerived();
            if (!force && hasConflict(c)) {
                return false; // reject entirely if any conflict found
            }
            created.append(c);
        }
    }
    for (const Course& cc : created) m_courses.append(cc);
    return true;
}

QVector<Course> CourseManager::courses() const { return m_courses; }

QVector<Course> CourseManager::coursesForWeekAndDay(int week, int day) const {
    QVector<Course> out;
    for (const Course& c : m_courses) {
        if (c.weekDay() == day && c.startWeek() <= week && c.endWeek() >= week) {
            out.append(c);
        }
    }
    return out;
}

void CourseManager::updateDerived() {
    for (Course& c : m_courses) c.computeDerived();
}

void CourseManager::clearAll() {
    m_courses.clear();
    m_timeslots.clear();
}

QVector<TimeSlot> CourseManager::timeSlots() const {
    return m_timeslots;
}
