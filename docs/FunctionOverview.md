# 基于 Qt6 的学生课表管理系统功能说明

## 1）项目概述
- **用途与用户**：提供桌面端课程排课与周视图展示，便于教务人员或班主任维护课程信息并查看周次课表。
- **技术栈**：Qt 6 Widgets + QtSql，C++17，qmake 项目文件，核心使用 SQLite 本地数据库存储课程与学期信息，默认学期为“2025-2026-1”。【F:SourceManager.pro†L1-L20】【F:coursemanager.cpp†L8-L137】
- **完成度**：课程增删改查、按周展示、搜索、导出与备份、学期设置和夜间模式均有代码实现；CSV 导入与学生维度功能尚未实现或缺失。【F:mainwindow.cpp†L270-L333】【F:coursemanager.cpp†L350-L557】

## 2）系统总体功能一览
- **课程管理**：新增、编辑、删除课程，展示课程详情，支持教师/地点等信息录入。【F:mainwindow.cpp†L270-L309】【F:coursemanager.cpp†L178-L260】
- **课表展示与周次导航**：表格按时间+周一至周日显示课程，提供上一周/下一周/本周按钮与周次计算。【F:mainwindow.cpp†L170-L235】【F:mainwindow.cpp†L464-L520】
- **课程搜索**：可按课程名、教师或地点模糊查询并在表格中高亮结果。【F:mainwindow.cpp†L669-L733】【F:coursemanager.cpp†L350-L368】
- **数据导出与备份**：导出当前学期课程为 CSV，创建数据库备份文件；导入占位未实现。【F:mainwindow.cpp†L735-L760】【F:coursemanager.cpp†L512-L557】
- **学期设置与主题**：提供设置学期入口，夜间模式切换与样式美化、刷新动画等视觉增强。【F:mainwindow.cpp†L270-L309】【F:mainwindow.cpp†L311-L444】【F:mainwindow.cpp†L639-L667】

## 3）功能模块详细说明

### 3.1 课程管理模块
- **功能说明**：维护课程名称、教师、地点、上课日、起止节次、起止日期、考试日期、课程类型、学分等；支持新增（弹窗表单）、编辑、删除与查看详情。
- **界面入口**：主窗口底部“添加课程”按钮；表格单元格点击弹出详情对话框，并可继续编辑或删除。【F:mainwindow.cpp†L170-L244】【F:mainwindow.cpp†L270-L309】
- **操作流程**：点击“添加课程”→填写表单→保存后写入 SQLite；编辑/删除通过详情弹窗触发对应更新或删除逻辑。【F:coursemanager.cpp†L178-L260】
- **核心类与函数**：`CourseManager::addCourse/updateCourse/deleteCourse/getCoursesByWeek/searchCourses` 负责数据库存取；`MainWindow` 负责表单弹窗、表格填充与按钮交互。【F:coursemanager.cpp†L178-L368】【F:mainwindow.cpp†L669-L733】

### 3.2 课表展示模块
- **展示方式**：`QTableWidget` 8 列（时间+周一至周日）、10 行节次，首列固定时间段，其余单元格显示课程名与地点并设置 tooltip（教师、节次、类型、学分）。【F:mainwindow.cpp†L196-L239】【F:mainwindow.cpp†L669-L733】
- **周次导航**：上一周/下一周/本周按钮驱动 `m_currentWeekStart` 变更，显示“第X周/总周数”并限制最后一周的导航样式。【F:mainwindow.cpp†L170-L195】【F:mainwindow.cpp†L464-L520】

### 3.3 查询与搜索模块
- **搜索条件**：关键词同时匹配课程名、教师、地点，支持模糊搜索；无关键词时恢复全部课程。【F:coursemanager.cpp†L350-L368】【F:mainwindow.cpp†L669-L677】
- **结果呈现**：搜索结果覆盖课表显示，按课程类型着色，tooltip 展示详细信息。【F:mainwindow.cpp†L694-L733】

### 3.4 数据导出与备份
- **CSV 导出**：写出课程名称、星期、节次、地点、起止日期、教师、考试日期、课程类型与学分到指定文件。【F:coursemanager.cpp†L512-L538】【F:mainwindow.cpp†L735-L749】
- **数据库备份**：复制 SQLite 数据库到时间戳文件并提示结果。【F:coursemanager.cpp†L550-L557】【F:mainwindow.cpp†L751-L759】
- **CSV 导入**：函数存在但返回固定值，功能待实现。【F:coursemanager.cpp†L544-L548】

### 3.5 学期与主题设置
- **学期管理**：维护 `semesters` 表，支持设置学期名称及起止日期，并据此计算周次；默认插入 2025-2026-1 示例学期与课程数据。【F:coursemanager.cpp†L65-L137】【F:coursemanager.cpp†L468-L509】
- **主题/动画**：夜间模式切换、淡入与刷新动画提升视觉体验；自定义样式表覆盖按钮、表格等控件。【F:mainwindow.cpp†L270-L309】【F:mainwindow.cpp†L311-L444】【F:mainwindow.cpp†L639-L667】

## 4）界面结构与导航
- **布局**：顶部标题+时钟，中部搜索栏与周次导航，中心课表，底部操作按钮区（添加课程、学期设置、夜间模式、刷新、导出、备份）。【F:mainwindow.cpp†L75-L309】
- **导航方式**：通过按钮进入添加课程/学期设置/导出备份等对话框；表格单击课程查看详情并进一步编辑或删除。【F:mainwindow.cpp†L170-L309】【F:mainwindow.cpp†L270-L309】

## 5）数据存储与模型设计
- **数据库表**：`courses` 包含课程名、星期、起止节次、地点、起止日期、教师、考试日期、课程类型、学分及所属学期；`semesters` 包含学期名与起止日期。示例数据在初始建库时插入。【F:coursemanager.cpp†L65-L137】
- **模型使用**：直接通过 `QSqlQuery` 读写 SQLite，将结果填充到内存表格，无独立的 Model/View 类绑定。【F:coursemanager.cpp†L263-L368】【F:mainwindow.cpp†L669-L733】

## 6）使用说明（示例）
1. **新建课程并查看课表**：点击“添加课程”，填写课程名称、星期、节次、地点、教师、起止日期等信息保存；返回主界面后课程按对应周与节次出现在表格中，可用周导航切换周次。【F:mainwindow.cpp†L170-L235】【F:coursemanager.cpp†L178-L306】
2. **搜索课程**：在搜索框输入课程名、教师或地点关键词点击“搜索”，表格仅保留匹配课程；清空输入并刷新可恢复全部课程。【F:mainwindow.cpp†L95-L129】【F:mainwindow.cpp†L669-L733】【F:coursemanager.cpp†L350-L368】
3. **导出与备份**：点击“导出数据”选择 CSV 保存路径导出当前学期课程；或点击“备份数据”生成数据库副本。【F:mainwindow.cpp†L735-L760】【F:coursemanager.cpp†L512-L557】

## 7）当前限制与扩展方向
- **学生相关功能缺失**：当前仅有课程与学期数据表，未包含学生信息或选课关联，需要后续扩展。
- **CSV 导入未完成**：`importFromCsv` 空实现，需要补充解析与入库逻辑。【F:coursemanager.cpp†L544-L548】
- **统计/打印能力有限**：未见按学生/课程统计或打印导出，后续可考虑报表、打印或多角色支持。
