# ClassPoints: Student Participation Tracking System

## 👥 Team Information
* **Ansh Chauhan** (Team Lead) - University Roll: 2592521
* **Garima Joshi** - University Roll: 2592557
* **Harshit Farswan** - University Roll: 2592572
* **Priyanshu Rawat** - University Roll: 2592718

## 🎯 Project Motivation
Manual tracking of student participation often leads to data loss and confusion between students with the same name. ClassPoints provides a lightweight digital platform to record participation instantly, ensuring records are safe and organized.

## 🛠️ Tech Stack
* **Language:** C (C99)
* **Web Interface:** CGI (Common Gateway Interface)
* **Database:** MySQL (via `mysql.h` library)
* **Server:** Apache via XAMPP

## 🚀 Key Features (Phase 2)
* **Duplicate Name Resolution:** Uses a combination of Student Name and Father’s Name to uniquely identify students.
* **Real-time Leaderboard:** Automatically calculates student rankings using SQL `SUM()` and `GROUP BY` functions.
* **Audit Trail System:** Instead of overwriting records, every point entry is logged with a timestamp in a `points_log` table for full transparency.

## 🏗️ System Architecture
The system follows a client-server architecture:
1. **User Interface (HTML Forms):** Where teachers enter participation data.
2. **Application Layer (C with CGI):** Processes input and handles logical operations.
3. **Database Layer (MySQL):** Stores student information and historical participation records.

---
*Developed as part of the PBL (Project Based Learning) curriculum.*