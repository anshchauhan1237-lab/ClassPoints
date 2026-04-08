# ClassPoints: Student Participation Tracking System

## 👥 Team Information
* [cite_start]**Ansh Chauhan** (Team Lead) - University Roll: 2592521 
* [cite_start]**Garima Joshi** - University Roll: 2592557 
* [cite_start]**Harshit Farswan** - University Roll: 2592572 
* [cite_start]**Priyanshu Rawat** - University Roll: 2592718 

## 🎯 Project Motivation
[cite_start]Manual tracking of student participation often leads to data loss and confusion, especially with students sharing the same name[cite: 8, 9]. [cite_start]ClassPoints provides a lightweight digital platform to record participation instantly and transparently[cite: 11, 14].

## 🛠️ Tech Stack
* [cite_start]**Language:** C (C99) [cite: 27, 55]
* [cite_start]**Web Interface:** CGI (Common Gateway Interface) [cite: 55, 63]
* [cite_start]**Database:** MySQL via `mysql.h` library [cite: 27, 66]
* [cite_start]**Server:** Apache via XAMPP [cite: 56, 69]

## 🚀 Key Features (Phase 2)
* [cite_start]**Duplicate Name Resolution:** Students are uniquely identified using a combination of their Name and Father's Name[cite: 39, 49, 88].
* [cite_start]**Real-time Leaderboard:** Automatically calculates student rankings using SQL `SUM()` and `GROUP BY` functions[cite: 53, 54, 77].
* [cite_start]**Audit Trail System:** Every point entry is logged with a timestamp in a `points_log` table, ensuring a full history of changes[cite: 29, 30, 78].
* [cite_start]**Teacher Authentication:** Secure login portal for managing class data[cite: 65, 84].

## 🏗️ System Architecture
[cite_start]The system follows a client-server architecture[cite: 60]:
1. [cite_start]**User Interface:** HTML Forms for teacher inputs[cite: 68, 71].
2. [cite_start]**Application Layer:** C-based CGI programs processing logic[cite: 70, 71].
3. [cite_start]**Database Layer:** MySQL storing structured student and log records[cite: 71].



---
*Developed as part of the PBL (Project Based Learning) curriculum at Graphic Era Hill University.*