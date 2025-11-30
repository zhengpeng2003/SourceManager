#include "coursemanager.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>

CourseManager::CourseManager(QObject *parent)
    : QObject(parent), m_currentSemester("2025-2026-1")
{
    initDatabase();
}

CourseManager::~CourseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}


    int CourseManager::getSemesterWeeks() const
    {
        QDate startDate = getSemesterStartDate();
        QDate endDate = getSemesterEndDate();

        if (!startDate.isValid() || !endDate.isValid() || startDate >= endDate) {
            return 0;
        }

        return startDate.daysTo(endDate) / 7 + 1;
    }


bool CourseManager::initDatabase()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }

    QString dbPath = dataPath + "/coursemanager.db";
    bool dbExists = QFile::exists(dbPath);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {

        return false;
    }

    if (!createTables()) {
        return false;
    }

    if (!dbExists) {
        upgradeDatabase();
    }

    return true;
}

bool CourseManager::createTables()
{
    QSqlQuery query;
    m_db.transaction();

    QString courseTable =
        "CREATE TABLE IF NOT EXISTS courses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "day_of_week INTEGER NOT NULL,"
        "start_slot INTEGER NOT NULL,"
        "end_slot INTEGER NOT NULL,"
        "location TEXT NOT NULL,"
        "start_date TEXT NOT NULL,"
        "end_date TEXT NOT NULL,"
        "teacher TEXT,"
        "exam_date TEXT,"
        "course_type TEXT,"
        "credits REAL DEFAULT 0,"
        "semester TEXT NOT NULL"
        ")";

    if (!query.exec(courseTable)) {

        m_db.rollback();
        return false;
    }

    QString semesterTable =
        "CREATE TABLE IF NOT EXISTS semesters ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT UNIQUE NOT NULL,"
        "start_date TEXT NOT NULL,"
        "end_date TEXT NOT NULL"
        ")";

    if (!query.exec(semesterTable)) {

        m_db.rollback();
        return false;
    }

    QString checkSemester = "SELECT COUNT(*) FROM semesters WHERE name = '2025-2026-1'";
    if (query.exec(checkSemester) && query.next() && query.value(0).toInt() == 0) {
        QString insertSemester =
            "INSERT INTO semesters (name, start_date, end_date) VALUES "
            "('2025-2026-1', '2025-09-01', '2026-01-31')";
        if (!query.exec(insertSemester)) {
            m_db.rollback();
            return false;
        }
    }

    QString checkCourses = "SELECT COUNT(*) FROM courses";
    if (query.exec(checkCourses) && query.next() && query.value(0).toInt() == 0) {
        QStringList exampleCourses = {
            "INSERT INTO courses (name, day_of_week, start_slot, end_slot, location, start_date, end_date, teacher, exam_date, course_type, credits, semester) VALUES ('Web应用开发', 1, 1, 2, '厚德楼 B601', '2025-09-01', '2026-01-31', '张老师', '2025-12-20', '必修', 3.0, '2025-2026-1')",
            "INSERT INTO courses (name, day_of_week, start_slot, end_slot, location, start_date, end_date, teacher, exam_date, course_type, credits, semester) VALUES ('大模型应用', 2, 2, 3, '厚德楼 B502', '2025-09-01', '2026-01-31', '李老师', '2025-12-22', '选修', 2.0, '2025-2026-1')",
            "INSERT INTO courses (name, day_of_week, start_slot, end_slot, location, start_date, end_date, teacher, exam_date, course_type, credits, semester) VALUES ('数据结构', 3, 3, 4, '厚德楼 B404', '2025-09-01', '2026-01-31', '王老师', '2025-12-25', '必修', 4.0, '2025-2026-1')",
            "INSERT INTO courses (name, day_of_week, start_slot, end_slot, location, start_date, end_date, teacher, exam_date, course_type, credits, semester) VALUES ('生产管理概论', 4, 1, 2, '厚德楼 B403', '2025-09-01', '2026-01-31', '赵老师', '2025-12-18', '选修', 2.5, '2025-2026-1')",
            "INSERT INTO courses (name, day_of_week, start_slot, end_slot, location, start_date, end_date, teacher, exam_date, course_type, credits, semester) VALUES ('程序设计实践', 5, 4, 5, '厚德楼 B601', '2025-09-01', '2026-01-31', '陈老师', '2025-12-28', '实验', 1.5, '2025-2026-1')"
        };

        for (const QString &sql : exampleCourses) {
            if (!query.exec(sql)) {
                m_db.rollback();
                return false;
            }
        }
    }

    m_db.commit();
    return true;
}

bool CourseManager::upgradeDatabase()
{
    QSqlQuery query;

    // 检查并添加缺失的字段
    QStringList columnsToAdd = {
        "exam_date", "course_type", "credits"
    };

    for (const QString &column : columnsToAdd) {
        QString checkColumn = QString("PRAGMA table_info(courses)");
        bool columnExists = false;

        if (query.exec(checkColumn)) {
            while (query.next()) {
                if (query.value(1).toString() == column) {
                    columnExists = true;
                    break;
                }
            }
        }

        if (!columnExists) {
            QString addColumn = QString("ALTER TABLE courses ADD COLUMN %1 TEXT").arg(column);
            if (column == "credits") {
                addColumn = QString("ALTER TABLE courses ADD COLUMN %1 REAL DEFAULT 0").arg(column);
            }

            if (!query.exec(addColumn)) {

                return false;
            }
        }
    }

    return true;
}

bool CourseManager::addCourse(const CourseData &course)
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare(
        "INSERT INTO courses (name, day_of_week, start_slot, end_slot, location, "
        "start_date, end_date, teacher, exam_date, course_type, credits, semester) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );

    query.addBindValue(course.name);
    query.addBindValue(course.dayOfWeek);
    query.addBindValue(course.startSlot);
    query.addBindValue(course.endSlot);
    query.addBindValue(course.location);
    query.addBindValue(course.startDate.toString(Qt::ISODate));
    query.addBindValue(course.endDate.toString(Qt::ISODate));
    query.addBindValue(course.teacher);
    query.addBindValue(course.examDate.isValid() ? course.examDate.toString(Qt::ISODate) : "");
    query.addBindValue(course.courseType);
    query.addBindValue(course.credits);
    query.addBindValue(m_currentSemester);

    if (!query.exec()) {

        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();
    return true;
}

bool CourseManager::updateCourse(const CourseData &course)
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare(
        "UPDATE courses SET name=?, day_of_week=?, start_slot=?, end_slot=?, "
        "location=?, start_date=?, end_date=?, teacher=?, exam_date=?, course_type=?, credits=? WHERE id=?"
        );

    query.addBindValue(course.name);
    query.addBindValue(course.dayOfWeek);
    query.addBindValue(course.startSlot);
    query.addBindValue(course.endSlot);
    query.addBindValue(course.location);
    query.addBindValue(course.startDate.toString(Qt::ISODate));
    query.addBindValue(course.endDate.toString(Qt::ISODate));
    query.addBindValue(course.teacher);
    query.addBindValue(course.examDate.isValid() ? course.examDate.toString(Qt::ISODate) : "");
    query.addBindValue(course.courseType);
    query.addBindValue(course.credits);
    query.addBindValue(course.id);

    if (!query.exec()) {
        qDebug() << "Failed to update course:" << query.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();
    return true;
}

bool CourseManager::deleteCourse(int id)
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("DELETE FROM courses WHERE id=?");
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Failed to delete course:" << query.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();
    return true;
}

QList<CourseData> CourseManager::getCoursesByWeek(const QDate &date)
{
    QList<CourseData> courses;
    QSqlQuery query;

    query.prepare(
        "SELECT id, name, day_of_week, start_slot, end_slot, location, "
        "start_date, end_date, teacher, exam_date, course_type, credits FROM courses "
        "WHERE semester = ? AND start_date <= ? AND end_date >= ? "
        "ORDER BY day_of_week, start_slot"
        );

    query.addBindValue(m_currentSemester);
    query.addBindValue(date.toString(Qt::ISODate));
    query.addBindValue(date.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            CourseData course;
            course.id = query.value(0).toInt();
            course.name = query.value(1).toString();
            course.dayOfWeek = query.value(2).toInt();
            course.startSlot = query.value(3).toInt();
            course.endSlot = query.value(4).toInt();
            course.location = query.value(5).toString();
            course.startDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
            course.endDate = QDate::fromString(query.value(7).toString(), Qt::ISODate);
            course.teacher = query.value(8).toString();

            QString examDateStr = query.value(9).toString();
            if (!examDateStr.isEmpty()) {
                course.examDate = QDate::fromString(examDateStr, Qt::ISODate);
            }

            course.courseType = query.value(10).toString();
            course.credits = query.value(11).toDouble();

            courses.append(course);
        }
    } else {
        qDebug() << "Failed to get courses:" << query.lastError().text();
    }

    return courses;
}

QList<CourseData> CourseManager::getAllCourses()
{
    QList<CourseData> courses;
    QSqlQuery query;

    query.prepare(
        "SELECT id, name, day_of_week, start_slot, end_slot, location, "
        "start_date, end_date, teacher, exam_date, course_type, credits FROM courses "
        "WHERE semester = ? ORDER BY day_of_week, start_slot"
        );

    query.addBindValue(m_currentSemester);

    if (query.exec()) {
        while (query.next()) {
            CourseData course;
            course.id = query.value(0).toInt();
            course.name = query.value(1).toString();
            course.dayOfWeek = query.value(2).toInt();
            course.startSlot = query.value(3).toInt();
            course.endSlot = query.value(4).toInt();
            course.location = query.value(5).toString();
            course.startDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
            course.endDate = QDate::fromString(query.value(7).toString(), Qt::ISODate);
            course.teacher = query.value(8).toString();

            QString examDateStr = query.value(9).toString();
            if (!examDateStr.isEmpty()) {
                course.examDate = QDate::fromString(examDateStr, Qt::ISODate);
            }

            course.courseType = query.value(10).toString();
            course.credits = query.value(11).toDouble();

            courses.append(course);
        }
    }

    return courses;
}

QList<CourseData> CourseManager::searchCourses(const QString &keyword)
{
    QList<CourseData> courses;
    QSqlQuery query;

    query.prepare(
        "SELECT id, name, day_of_week, start_slot, end_slot, location, "
        "start_date, end_date, teacher, exam_date, course_type, credits FROM courses "
        "WHERE semester = ? AND (name LIKE ? OR teacher LIKE ? OR location LIKE ?) "
        "ORDER BY day_of_week, start_slot"
        );

    QString searchPattern = "%" + keyword + "%";
    query.addBindValue(m_currentSemester);
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);

    if (query.exec()) {
        while (query.next()) {
            CourseData course;
            course.id = query.value(0).toInt();
            course.name = query.value(1).toString();
            course.dayOfWeek = query.value(2).toInt();
            course.startSlot = query.value(3).toInt();
            course.endSlot = query.value(4).toInt();
            course.location = query.value(5).toString();
            course.startDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
            course.endDate = QDate::fromString(query.value(7).toString(), Qt::ISODate);
            course.teacher = query.value(8).toString();

            QString examDateStr = query.value(9).toString();
            if (!examDateStr.isEmpty()) {
                course.examDate = QDate::fromString(examDateStr, Qt::ISODate);
            }

            course.courseType = query.value(10).toString();
            course.credits = query.value(11).toDouble();

            courses.append(course);
        }
    }

    return courses;
}

CourseData CourseManager::getCourseById(int id)
{
    CourseData course;
    QSqlQuery query;

    query.prepare(
        "SELECT id, name, day_of_week, start_slot, end_slot, location, "
        "start_date, end_date, teacher, exam_date, course_type, credits FROM courses WHERE id=?"
        );

    query.addBindValue(id);

    if (query.exec() && query.next()) {
        course.id = query.value(0).toInt();
        course.name = query.value(1).toString();
        course.dayOfWeek = query.value(2).toInt();
        course.startSlot = query.value(3).toInt();
        course.endSlot = query.value(4).toInt();
        course.location = query.value(5).toString();
        course.startDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
        course.endDate = QDate::fromString(query.value(7).toString(), Qt::ISODate);
        course.teacher = query.value(8).toString();

        QString examDateStr = query.value(9).toString();
        if (!examDateStr.isEmpty()) {
            course.examDate = QDate::fromString(examDateStr, Qt::ISODate);
        }

        course.courseType = query.value(10).toString();
        course.credits = query.value(11).toDouble();
    }

    return course;
}

bool CourseManager::setCurrentSemester(const QString &semester)
{
    m_currentSemester = semester;
    return true;
}

QString CourseManager::getCurrentSemester() const
{
    return m_currentSemester;
}

QDate CourseManager::getSemesterStartDate() const
{
    QSqlQuery query;
    query.prepare("SELECT start_date FROM semesters WHERE name=?");
    query.addBindValue(m_currentSemester);

    if (query.exec() && query.next()) {
        return QDate::fromString(query.value(0).toString(), Qt::ISODate);
    }

    return QDate(2025, 9, 1);
}

QDate CourseManager::getSemesterEndDate() const
{
    QSqlQuery query;
    query.prepare("SELECT end_date FROM semesters WHERE name=?");
    query.addBindValue(m_currentSemester);

    if (query.exec() && query.next()) {
        return QDate::fromString(query.value(0).toString(), Qt::ISODate);
    }

    return QDate(2026, 1, 31);
}

bool CourseManager::setSemester(const QString &name, const QDate &start, const QDate &end)
{
    if (!start.isValid() || !end.isValid() || start >= end) {
        return false;
    }

    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM semesters WHERE name=?");
    query.addBindValue(name);

    bool exists = false;
    if (query.exec() && query.next()) {
        exists = query.value(0).toInt() > 0;
    }

    bool ok = false;
    if (exists) {
        QSqlQuery update;
        update.prepare("UPDATE semesters SET start_date=?, end_date=? WHERE name=?");
        update.addBindValue(start.toString(Qt::ISODate));
        update.addBindValue(end.toString(Qt::ISODate));
        update.addBindValue(name);
        ok = update.exec();
    } else {
        QSqlQuery insert;
        insert.prepare("INSERT INTO semesters (name, start_date, end_date) VALUES (?, ?, ?)");
        insert.addBindValue(name);
        insert.addBindValue(start.toString(Qt::ISODate));
        insert.addBindValue(end.toString(Qt::ISODate));
        ok = insert.exec();
    }

    if (!ok) {
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();
    m_currentSemester = name;
    return true;
}

bool CourseManager::exportToCsv(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);


    // 写入表头
    out << "课程名称,星期,开始节次,结束节次,地点,开始日期,结束日期,教师,考试日期,课程类型,学分\n";

    QList<CourseData> courses = getAllCourses();
    for (const CourseData &course : courses) {
        out << course.name << ","
            << course.dayOfWeek << ","
            << course.startSlot << ","
            << course.endSlot << ","
            << course.location << ","
            << course.startDate.toString("yyyy-MM-dd") << ","
            << course.endDate.toString("yyyy-MM-dd") << ","
            << course.teacher << ","
            << (course.examDate.isValid() ? course.examDate.toString("yyyy-MM-dd") : "") << ","
            << course.courseType << ","
            << course.credits << "\n";
    }

    file.close();
    return true;
}

bool CourseManager::importFromCsv(const QString &filePath)
{
    // 实现CSV导入逻辑
    return true;
}

bool CourseManager::createBackup()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString backupPath = dataPath + "/backup_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".db";

    QFile::copy(dataPath + "/coursemanager.db", backupPath);
    return QFile::exists(backupPath);
}
