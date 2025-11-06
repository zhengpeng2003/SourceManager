#include "course.h"
#include <QDate>

Course::Course()
    : m_name(),
    m_examDate(QDate()),
    m_teacher(),
    m_location(),
    m_lessonIndex(1),
    m_startTime(8,0),
    m_endTime(8,45),
    m_weekDay(1),
    m_startWeek(1),
    m_endWeek(1),
    m_remainingDays(-1)
{}

QString Course::name() const { return m_name; }
void Course::setName(const QString& v) { m_name = v; }

QDate Course::examDate() const { return m_examDate; }
void Course::setExamDate(const QDate& d) { m_examDate = d; }

QString Course::teacher() const { return m_teacher; }
void Course::setTeacher(const QString& v) { m_teacher = v; }

QString Course::location() const { return m_location; }
void Course::setLocation(const QString& v) { m_location = v; }

int Course::lessonIndex() const { return m_lessonIndex; }
void Course::setLessonIndex(int idx) { m_lessonIndex = idx; }

QTime Course::startTime() const { return m_startTime; }
void Course::setStartTime(const QTime& t) { m_startTime = t; }

QTime Course::endTime() const { return m_endTime; }
void Course::setEndTime(const QTime& t) { m_endTime = t; }

int Course::weekDay() const { return m_weekDay; }
void Course::setWeekDay(int v) { m_weekDay = v; }

int Course::startWeek() const { return m_startWeek; }
void Course::setStartWeek(int v) { m_startWeek = v; }

int Course::endWeek() const { return m_endWeek; }
void Course::setEndWeek(int v) { m_endWeek = v; }

int Course::remainingDays() const { return m_remainingDays; }

void Course::computeDerived() const {
    if (!m_examDate.isValid()) {
        m_remainingDays = -1;
        return;
    }
    QDate today = QDate::currentDate();
    int d = today.daysTo(m_examDate);
    m_remainingDays = (d >= 0) ? d : -1;
}

QString Course::toLine() const {
    // name|yyyy-MM-dd|teacher|location|lessonIndex|hh:mm|hh:mm|weekDay|startWeek|endWeek
    QStringList parts;
    parts << m_name;
    parts << (m_examDate.isValid() ? m_examDate.toString("yyyy-MM-dd") : QString());
    parts << m_teacher;
    parts << m_location;
    parts << QString::number(m_lessonIndex);
    parts << m_startTime.toString("hh:mm");
    parts << m_endTime.toString("hh:mm");
    parts << QString::number(m_weekDay);
    parts << QString::number(m_startWeek);
    parts << QString::number(m_endWeek);
    return parts.join("|");
}

Course Course::fromLine(const QString& line) {
    Course c;
    QStringList parts = line.split("|");
    if (parts.size() < 10) return c;
    c.m_name = parts[0];
    c.m_examDate = QDate::fromString(parts[1], "yyyy-MM-dd");
    c.m_teacher = parts[2];
    c.m_location = parts[3];
    c.m_lessonIndex = parts[4].toInt();
    c.m_startTime = QTime::fromString(parts[5], "hh:mm");
    c.m_endTime = QTime::fromString(parts[6], "hh:mm");
    c.m_weekDay = parts[7].toInt();
    c.m_startWeek = parts[8].toInt();
    c.m_endWeek = parts[9].toInt();
    c.computeDerived();
    return c;
}
