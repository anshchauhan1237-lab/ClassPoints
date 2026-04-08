#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "files/mysql.h"
MYSQL* sql_connector();
void handle_teacher_login(MYSQL *con, char *data);
void set_teacher_password(MYSQL *con, char *data);
void verify_teacher_login(MYSQL *con, char *data);
void handle_student_search(MYSQL *con, char *data);
void save_points_to_db(MYSQL *con, char *data);
void get_value(char *data, char *key, char *output, int max_len);
void json_error(char *msg);
void json_ok(char *json_body);
void url_decode(char *str);
void show_teacher_history(MYSQL *con, char *data);
void show_student_edit_profile(MYSQL *con, char *s_id, char *t_name);
void apply_point_adjustment(MYSQL *con, char *data);
void handle_student_search_for_edit(MYSQL *con, char *data);
void reset_student_points(MYSQL *con, char *data);
void show_leaderboard(MYSQL *con);
void handle_student_login(MYSQL *con, char *data);
void set_student_password(MYSQL *con, char *data);
void verify_student_login(MYSQL *con, char *data);
void show_student_summary(MYSQL *con, char *data);
void show_student_history(MYSQL *con, char *data);
void change_student_password(MYSQL *con, char *data);

int main() {
    printf("Content-type: application/json\n\n");

    MYSQL *con = sql_connector();
    if (con == NULL) {
        json_error("Could not connect to database");
        return 1;
    }

    char *data = getenv("QUERY_STRING");

    if (data == NULL || strlen(data) == 0) {
        json_error("No parameters provided");
        mysql_close(con);
        return 0;
    }

    if (strstr(data, "role=teacher")) {
        if (strstr(data, "action=set_pass")) {
            set_teacher_password(con, data);
        }
        else if (strstr(data, "action=verify")) {
            verify_teacher_login(con, data);
        }
        else if (strstr(data, "action=search_student")) {
            handle_student_search(con, data);
        }
        else if (strstr(data, "action=finalize_points")) {
            save_points_to_db(con, data);
        }
        else if (strstr(data, "action=history")) {
            show_teacher_history(con, data);
        }
        else if (strstr(data, "action=search_for_edit")) {
            handle_student_search_for_edit(con, data);
        }
        else if (strstr(data, "action=edit_profile")) {
            char s_id[20] = {0}, t_name[50] = {0};
            get_value(data, "s_id=", s_id, sizeof(s_id));
            get_value(data, "t_name=", t_name, sizeof(t_name));
            show_student_edit_profile(con, s_id, t_name);
        }
        else if (strstr(data, "action=apply_adjustment")) {
            apply_point_adjustment(con, data);
        }
        else if (strstr(data, "action=reset_points")) {
            reset_student_points(con, data);
        }
        else if (strstr(data, "action=leaderboard")) {
            show_leaderboard(con);
        }
        else if (strstr(data, "name=")) {
            handle_teacher_login(con, data);
        }
        else {
            json_error("Missing action or name parameter");
        }
    }
    else if (strstr(data, "role=student")) {
        if (strstr(data, "action=set_pass")) {
            set_student_password(con, data);
        }
        else if (strstr(data, "action=verify")) {
            verify_student_login(con, data);
        }
        else if (strstr(data, "action=summary")) {
            show_student_summary(con, data);
        }
        else if (strstr(data, "action=history")) {
            show_student_history(con, data);
        }
        else if (strstr(data, "action=change_pass")) {
            change_student_password(con, data);
        }
        else if (strstr(data, "action=leaderboard")) {
            show_leaderboard(con);
        }
        else if (strstr(data, "student_id=")) {
            handle_student_login(con, data);
        }
        else {
            json_error("Missing action or student_id parameter");
        }
    }
    else {
        json_error("Invalid role selected");
    }

    if (con) mysql_close(con);
    return 0;
}

/* ==================== DATABASE ==================== */
MYSQL* sql_connector() {
    MYSQL *con = mysql_init(NULL);
    if (con == NULL) {
        return NULL;
    }
    int ssl_mode = 1;
    mysql_options(con, MYSQL_OPT_SSL_MODE, &ssl_mode);
    if (mysql_real_connect(con, "localhost", "root", "root", "teachers", 0, NULL, 0) == NULL) {
        mysql_close(con);
        return NULL;
    }
    return con;
}

/* ==================== TEACHER: CHECK NAME ==================== */
void handle_teacher_login(MYSQL *con, char *data) {
    char name[50] = {0};
    get_value(data, "name=", name, sizeof(name));

    char query[500];
    sprintf(query, "SELECT password FROM teachers WHERE username = '%s'", name);

    if (mysql_query(con, query)) {
        json_error("Database query failed");
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        json_error("Could not retrieve results");
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == NULL) {
        printf("{\"status\":\"ok\",\"action\":\"denied\",\"message\":\"Name not recognized. Access denied.\"}");
    }
    else if (row[0] == NULL || strlen(row[0]) == 0) {
        printf("{\"status\":\"ok\",\"action\":\"set_password\",\"name\":\"%s\"}", name);
    }
    else {
        printf("{\"status\":\"ok\",\"action\":\"enter_password\",\"name\":\"%s\"}", name);
    }

    mysql_free_result(result);
}

/* ==================== TEACHER: SET PASSWORD ==================== */
void set_teacher_password(MYSQL *con, char *data) {
    if (con == NULL) {
        json_error("No database connection");
        return;
    }

    char name[50] = {0};
    char new_pass[50] = {0};
    get_value(data, "user=", name, sizeof(name));
    get_value(data, "new_pass=", new_pass, sizeof(new_pass));

    if (strlen(name) == 0 || strlen(new_pass) == 0) {
        json_error("Name and password are required");
        return;
    }

    char query[300];
    sprintf(query, "UPDATE teachers SET password = '%s' WHERE username = '%s'", new_pass, name);

    if (mysql_query(con, query)) {
        json_error("Could not save password");
    } else {
        printf("{\"status\":\"ok\",\"message\":\"Password set successfully!\"}");
    }
}

/* ==================== TEACHER: VERIFY PASSWORD ==================== */
void verify_teacher_login(MYSQL *con, char *data) {
    if (con == NULL) {
        json_error("No database connection");
        return;
    }

    char name[50] = {0};
    char input_pass[50] = {0};
    get_value(data, "user=", name, sizeof(name));
    get_value(data, "pass=", input_pass, sizeof(input_pass));

    char query[300];
    sprintf(query, "SELECT password FROM teachers WHERE username = '%s'", name);

    if (mysql_query(con, query)) {
        json_error("Database error");
        return;
    }

    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) {
        json_error("Could not retrieve results");
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);

    if (row == NULL) {
        json_error("User not found");
    }
    else {
        char *db_pass = row[0];
        if (db_pass != NULL && strcmp(input_pass, db_pass) == 0) {
            printf("{\"status\":\"ok\",\"action\":\"dashboard\",\"name\":\"%s\"}", name);
        }
        else {
            json_error("Incorrect password");
        }
    }

    mysql_free_result(res);
}

/* ==================== STUDENT FUNCTIONS ==================== */
void handle_student_selection(MYSQL *con, char *search_name) {
    char query[1000];
    sprintf(query, "SELECT student_id, name, father_name FROM students WHERE name = '%s'", search_name);

    if (mysql_query(con, query)) {
        json_error("Error searching students");
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        json_error("Could not retrieve student results");
        return;
    }

    int num_students = mysql_num_rows(result);

    if (num_students == 0) {
        json_error("No student found with that name");
    }
    else if (num_students == 1) {
        MYSQL_ROW row = mysql_fetch_row(result);
        printf("{\"status\":\"ok\",\"student_id\":\"%s\",\"name\":\"%s\",\"father\":\"%s\"}", row[0], row[1], row[2]);
    }
    else {
        printf("{\"status\":\"ok\",\"action\":\"select\",\"count\":%d,\"students\":[", num_students);
        MYSQL_ROW row;
        int first = 1;
        while ((row = mysql_fetch_row(result))) {
            if (!first) printf(",");
            printf("{\"id\":\"%s\",\"name\":\"%s\",\"father\":\"%s\"}", row[0], row[1], row[2]);
            first = 0;
        }
        printf("]}");
    }

    mysql_free_result(result);
}

void save_points(MYSQL *con, int s_id, char *desc, int awarded, int max) {
    char query[1000];
    sprintf(query, "INSERT INTO points_log (student_id, question_desc, points_awarded, max_possible) "
                   "VALUES (%d, '%s', %d, %d)", s_id, desc, awarded, max);

    if (mysql_query(con, query)) {
        json_error("Error saving points");
    } else {
        printf("{\"status\":\"ok\",\"message\":\"Points saved successfully!\"}");
    }
}

void handle_points_edit(MYSQL *con, char *search_name) {
    char query[1000];
    sprintf(query, "SELECT student_id, name, father_name FROM students WHERE name = '%s'", search_name);
    if (mysql_query(con, query)) {
        json_error("Database error");
        return;
    }
    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        json_error("Could not retrieve results");
        return;
    }
    int num_students = mysql_num_rows(result);

    if (num_students == 0) {
        json_error("Student not found");
    }
    else if (num_students > 1) {
        MYSQL_ROW row;
        printf("{\"status\":\"ok\",\"action\":\"select\",\"students\":[");
        int first = 1;
        while ((row = mysql_fetch_row(result))) {
            if (!first) printf(",");
            printf("{\"id\":\"%s\",\"name\":\"%s\",\"father\":\"%s\"}", row[0], row[1], row[2]);
            first = 0;
        }
        printf("]}");
    }
    else {
        MYSQL_ROW row = mysql_fetch_row(result);
        char *s_id = row[0];
        char *s_name = row[1];
        sprintf(query, "SELECT SUM(points_awarded) FROM points_log WHERE student_id = %s", s_id);
        if (mysql_query(con, query)) {
            mysql_free_result(result);
            json_error("Could not fetch points");
            return;
        }
        MYSQL_RES *sum_res = mysql_store_result(con);
        if (sum_res == NULL) {
            mysql_free_result(result);
            json_error("Could not retrieve sum");
            return;
        }
        MYSQL_ROW sum_row = mysql_fetch_row(sum_res);
        char *total = (sum_row && sum_row[0] != NULL) ? sum_row[0] : "0";
        printf("{\"status\":\"ok\",\"student_id\":\"%s\",\"name\":\"%s\",\"total_points\":\"%s\"}", s_id, s_name, total);
        mysql_free_result(sum_res);
    }
    mysql_free_result(result);
}

void process_adjustment(MYSQL *con, int s_id, int amount, char *reason) {
    char query[1000];
    sprintf(query, "INSERT INTO points_log (student_id, question_desc, points_awarded, max_possible) "
                   "VALUES (%d, '%s', %d, 0)", s_id, reason, amount);

    if (mysql_query(con, query)) {
        json_error("Update failed");
    } else {
        printf("{\"status\":\"ok\",\"message\":\"Points adjusted successfully!\"}");
    }
}

/* ==================== UTILITIES ==================== */
void get_value(char *data, char *key, char *output, int max_len) {
    char *start = strstr(data, key);
    if (start) {
        start += strlen(key);
        char *end = strchr(start, '&');
        int len;
        if (end) {
            len = end - start;
        } else {
            len = strlen(start);
        }
        if (len >= max_len) {
            len = max_len - 1;
        }
        strncpy(output, start, len);
        output[len] = '\0';
        url_decode(output);
    }
}

void url_decode(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src == '+') {
            *dst = ' ';
        } else if (*src == '%' && src[1] && src[2]) {
            char hex[3] = { src[1], src[2], '\0' };
            *dst = (char)strtol(hex, NULL, 16);
            src += 2;
        } else {
            *dst = *src;
        }
        src++;
        dst++;
    }
    *dst = '\0';
}

void json_error(char *msg) {
    printf("{\"status\":\"error\",\"message\":\"%s\"}", msg);
}

void json_ok(char *json_body) {
    printf("{\"status\":\"ok\",%s}", json_body);
}
/* ==================== TEACHER: SEARCH STUDENT ==================== */
void handle_student_search(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char s_name[50] = {0};
    char t_name[50] = {0};
    get_value(data, "s_name=", s_name, sizeof(s_name));
    get_value(data, "t_name=", t_name, sizeof(t_name));

    if (strlen(s_name) == 0) { json_error("Student name is required"); return; }

    char query[500];
    sprintf(query, "SELECT student_id, name, father_name FROM students WHERE name = '%s'", s_name);

    if (mysql_query(con, query)) {
        json_error("Database error while searching");
        return;
    }
    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) { json_error("Could not retrieve results"); return; }

    int count = mysql_num_rows(res);

    if (count == 0) {
        printf("{\"status\":\"error\",\"message\":\"No student found named '%s'\"}", s_name);
    }
    else if (count == 1) {
        MYSQL_ROW row = mysql_fetch_row(res);
        printf("{\"status\":\"ok\",\"action\":\"marks_form\","
               "\"student_id\":\"%s\",\"student_name\":\"%s\",\"father_name\":\"%s\"}",
               row[0], row[1], row[2]);
    }
    else {
        printf("{\"status\":\"ok\",\"action\":\"select_student\",\"students\":[");
        MYSQL_ROW row;
        int first = 1;
        while ((row = mysql_fetch_row(res))) {
            if (!first) printf(",");
            printf("{\"id\":\"%s\",\"name\":\"%s\",\"father\":\"%s\"}", row[0], row[1], row[2]);
            first = 0;
        }
        printf("]}");
    }
    mysql_free_result(res);
}

/* ==================== TEACHER: SAVE POINTS ==================== */
void save_points_to_db(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char s_id[20] = {0}, t_name[50] = {0}, desc[100] = {0}, awarded[10] = {0}, max_marks[10] = {0};
    get_value(data, "s_id=", s_id, sizeof(s_id));
    get_value(data, "t_name=", t_name, sizeof(t_name));
    get_value(data, "desc=", desc, sizeof(desc));
    get_value(data, "awarded=", awarded, sizeof(awarded));
    get_value(data, "max=", max_marks, sizeof(max_marks));

    if (strlen(s_id) == 0 || strlen(awarded) == 0 || strlen(max_marks) == 0) {
        json_error("Student ID, awarded marks, and max marks are required");
        return;
    }

    char query[1000];
    sprintf(query, "INSERT INTO points_log (student_id, teacher_name, question_desc, points_awarded, max_possible) "
                   "VALUES (%s, '%s', '%s', %s, %s)", s_id, t_name, desc, awarded, max_marks);

    if (mysql_query(con, query)) {
        char err_msg[300];
        sprintf(err_msg, "SQL Error: %s", mysql_error(con));
        json_error(err_msg);
    } else {
        /* Update students table totals */
        sprintf(query, "UPDATE students SET "
                       "total_points = (SELECT COALESCE(SUM(points_awarded),0) FROM points_log WHERE student_id = %s), "
                       "max_points = (SELECT COALESCE(SUM(max_possible),0) FROM points_log WHERE student_id = %s), "
                       "last_updated = NOW() "
                       "WHERE student_id = %s", s_id, s_id, s_id);
        mysql_query(con, query);
        printf("{\"status\":\"ok\",\"message\":\"Points saved successfully!\"}");
    }
}
/* ==================== TEACHER: POINT HISTORY ==================== */
void show_teacher_history(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char t_name[50] = {0};
    get_value(data, "teacher=", t_name, sizeof(t_name));

    if (strlen(t_name) == 0) { json_error("Teacher name is required"); return; }

    char query[1000];
    sprintf(query, "SELECT s.name, p.question_desc, p.points_awarded, p.max_possible, p.date_given "
                   "FROM points_log p "
                   "JOIN students s ON p.student_id = s.student_id "
                   "WHERE p.teacher_name = '%s' ORDER BY p.date_given DESC", t_name);

    if (mysql_query(con, query)) {
        json_error("Error fetching history");
        return;
    }

    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) { json_error("Could not retrieve history"); return; }

    printf("{\"status\":\"ok\",\"action\":\"history\",\"teacher\":\"%s\",\"records\":[", t_name);
    MYSQL_ROW row;
    int first = 1;
    while ((row = mysql_fetch_row(res))) {
        if (!first) printf(",");
        printf("{\"student\":\"%s\",\"topic\":\"%s\",\"awarded\":\"%s\",\"max\":\"%s\",\"date\":\"%s\"}",
               row[0] ? row[0] : "",
               row[1] ? row[1] : "",
               row[2] ? row[2] : "0",
               row[3] ? row[3] : "0",
               row[4] ? row[4] : "");
        first = 0;
    }
    printf("]}");
    mysql_free_result(res);
}
/* ==================== TEACHER: SEARCH FOR EDIT ==================== */
void handle_student_search_for_edit(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char s_name[50] = {0};
    char t_name[50] = {0};
    get_value(data, "s_name=", s_name, sizeof(s_name));
    get_value(data, "t_name=", t_name, sizeof(t_name));

    if (strlen(s_name) == 0) { json_error("Student name is required"); return; }

    char query[500];
    sprintf(query, "SELECT student_id, name, father_name FROM students WHERE name = '%s'", s_name);

    if (mysql_query(con, query)) { json_error("Database error"); return; }
    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) { json_error("Could not retrieve results"); return; }

    int count = mysql_num_rows(res);

    if (count == 0) {
        printf("{\"status\":\"error\",\"message\":\"No student found named '%s'\"}", s_name);
    }
    else if (count == 1) {
        MYSQL_ROW row = mysql_fetch_row(res);
        printf("{\"status\":\"ok\",\"action\":\"edit_profile\","
               "\"student_id\":\"%s\",\"student_name\":\"%s\",\"father_name\":\"%s\"}",
               row[0], row[1], row[2]);
    }
    else {
        printf("{\"status\":\"ok\",\"action\":\"select_student_edit\",\"students\":[");
        MYSQL_ROW row;
        int first = 1;
        while ((row = mysql_fetch_row(res))) {
            if (!first) printf(",");
            printf("{\"id\":\"%s\",\"name\":\"%s\",\"father\":\"%s\"}", row[0], row[1], row[2]);
            first = 0;
        }
        printf("]}");
    }
    mysql_free_result(res);
}

/* ==================== TEACHER: STUDENT EDIT PROFILE ==================== */
void show_student_edit_profile(MYSQL *con, char *s_id, char *t_name) {
    if (con == NULL) { json_error("No database connection"); return; }

    char query[1000];
    sprintf(query, "SELECT name, father_name FROM students WHERE student_id = %s", s_id);
    if (mysql_query(con, query)) { json_error("Database error"); return; }
    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) { json_error("Could not retrieve results"); return; }
    MYSQL_ROW s_row = mysql_fetch_row(res);

    if (s_row == NULL) {
        json_error("Student not found");
        mysql_free_result(res);
        return;
    }

    printf("{\"status\":\"ok\",\"action\":\"edit_profile\","
           "\"student_id\":\"%s\",\"student_name\":\"%s\",\"father_name\":\"%s\",", s_id, s_row[0], s_row[1]);
    mysql_free_result(res);

    sprintf(query, "SELECT question_desc, points_awarded, max_possible, date_given FROM points_log WHERE student_id = %s ORDER BY date_given DESC", s_id);
    if (mysql_query(con, query)) {
        printf("\"records\":[]}");
        return;
    }
    MYSQL_RES *p_res = mysql_store_result(con);
    if (p_res == NULL) {
        printf("\"records\":[]}");
        return;
    }

    printf("\"records\":[");
    MYSQL_ROW p_row;
    int first = 1;
    while ((p_row = mysql_fetch_row(p_res))) {
        if (!first) printf(",");
        printf("{\"topic\":\"%s\",\"awarded\":\"%s\",\"max\":\"%s\",\"date\":\"%s\"}",
               p_row[0] ? p_row[0] : "",
               p_row[1] ? p_row[1] : "0",
               p_row[2] ? p_row[2] : "0",
               p_row[3] ? p_row[3] : "");
        first = 0;
    }
    printf("]}");
    mysql_free_result(p_res);
}

/* ==================== TEACHER: APPLY ADJUSTMENT ==================== */
void apply_point_adjustment(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char s_id[20] = {0}, t_name[50] = {0}, desc[100] = {0}, adjust_val[10] = {0};
    get_value(data, "s_id=", s_id, sizeof(s_id));
    get_value(data, "t_name=", t_name, sizeof(t_name));
    get_value(data, "desc=", desc, sizeof(desc));
    get_value(data, "adjust_val=", adjust_val, sizeof(adjust_val));

    if (strlen(s_id) == 0 || strlen(adjust_val) == 0) {
        json_error("Student ID and adjustment value are required");
        return;
    }

    char query[1000];
    sprintf(query, "INSERT INTO points_log (student_id, teacher_name, question_desc, points_awarded, max_possible) "
                   "VALUES (%s, '%s', '%s', %s, 0)", s_id, t_name, desc, adjust_val);

    if (mysql_query(con, query)) {
        char err_msg[300];
        sprintf(err_msg, "SQL Error: %s", mysql_error(con));
        json_error(err_msg);
    } else {
        sprintf(query, "UPDATE students SET "
                       "total_points = (SELECT COALESCE(SUM(points_awarded),0) FROM points_log WHERE student_id = %s), "
                       "max_points = (SELECT COALESCE(SUM(max_possible),0) FROM points_log WHERE student_id = %s), "
                       "last_updated = NOW() "
                       "WHERE student_id = %s", s_id, s_id, s_id);
        mysql_query(con, query);
        printf("{\"status\":\"ok\",\"message\":\"Adjustment applied successfully!\"}");
    }
}

/* ==================== TEACHER: RESET STUDENT POINTS ==================== */
void reset_student_points(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char s_id[20] = {0}, t_name[50] = {0};
    get_value(data, "s_id=", s_id, sizeof(s_id));
    get_value(data, "t_name=", t_name, sizeof(t_name));

    if (strlen(s_id) == 0) {
        json_error("Student ID is required");
        return;
    }

    char query[1000];
    
    // First, delete all point logs for this student
    sprintf(query, "DELETE FROM points_log WHERE student_id = %s", s_id);
    if (mysql_query(con, query)) {
        char err_msg[300];
        sprintf(err_msg, "SQL Error: %s", mysql_error(con));
        json_error(err_msg);
        return;
    }

    // Next, reset their total and max points back to 0 in the students table
    sprintf(query, "UPDATE students SET total_points = 0, max_points = 0, last_updated = NOW() WHERE student_id = %s", s_id);
    if (mysql_query(con, query)) {
        char err_msg[300];
        sprintf(err_msg, "SQL Error: %s", mysql_error(con));
        json_error(err_msg);
    } else {
        printf("{\"status\":\"ok\",\"message\":\"All points have been completely reset!\"}");
    }
}
/* ==================== TEACHER: LEADERBOARD ==================== */
void show_leaderboard(MYSQL *con) {
    if (con == NULL) { json_error("No database connection"); return; }

    char query[1000];
    
    // We sum up the marks for each student and sort by the highest total
    sprintf(query, "SELECT s.name, COALESCE(SUM(p.points_awarded), 0) as total "
                   "FROM students s "
                   "LEFT JOIN points_log p ON p.student_id = s.student_id "
                   "GROUP BY s.student_id "
                   "HAVING total > 0 "
                   "ORDER BY total DESC LIMIT 5");

    if (mysql_query(con, query)) {
        json_error("Error fetching leaderboard");
        return;
    }

    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) { json_error("Could not retrieve leaderboard"); return; }
    
    printf("{\"status\":\"ok\",\"action\":\"leaderboard\",\"records\":[");
    MYSQL_ROW row;
    int first = 1;
    while ((row = mysql_fetch_row(res))) {
        if (!first) printf(",");
        printf("{\"name\":\"%s\",\"total\":\"%s\"}", row[0] ? row[0] : "", row[1] ? row[1] : "0");
        first = 0;
    }
    printf("]}");

    mysql_free_result(res);
}

/* ==================== STUDENT: CHECK ID ==================== */
void handle_student_login(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char student_id[20] = {0};
    get_value(data, "student_id=", student_id, sizeof(student_id));

    if (strlen(student_id) == 0) {
        json_error("Student ID is required");
        return;
    }

    char query[500];
    sprintf(query, "SELECT name, password FROM students WHERE student_id = '%s'", student_id);

    if (mysql_query(con, query)) {
        json_error("Database query failed");
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        json_error("Could not retrieve results");
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == NULL) {
        printf("{\"status\":\"ok\",\"action\":\"denied\",\"message\":\"Student ID not recognized. Access denied.\"}");
    }
    else if (row[1] == NULL || strlen(row[1]) == 0) {
        /* First time login - need to set password */
        printf("{\"status\":\"ok\",\"action\":\"set_password\",\"student_id\":\"%s\",\"name\":\"%s\"}", student_id, row[0]);
    }
    else {
        /* Password already set - need to enter it */
        printf("{\"status\":\"ok\",\"action\":\"enter_password\",\"student_id\":\"%s\",\"name\":\"%s\"}", student_id, row[0]);
    }

    mysql_free_result(result);
}

/* ==================== STUDENT: SET PASSWORD ==================== */
void set_student_password(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char student_id[20] = {0};
    char new_pass[50] = {0};
    get_value(data, "student_id=", student_id, sizeof(student_id));
    get_value(data, "new_pass=", new_pass, sizeof(new_pass));

    if (strlen(student_id) == 0 || strlen(new_pass) == 0) {
        json_error("Student ID and password are required");
        return;
    }

    char query[300];
    sprintf(query, "UPDATE students SET password = '%s' WHERE student_id = '%s'", new_pass, student_id);

    if (mysql_query(con, query)) {
        json_error("Could not save password");
    } else {
        printf("{\"status\":\"ok\",\"message\":\"Password set successfully!\"}");
    }
}

/* ==================== STUDENT: VERIFY PASSWORD ==================== */
void verify_student_login(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char student_id[20] = {0};
    char input_pass[50] = {0};
    get_value(data, "student_id=", student_id, sizeof(student_id));
    get_value(data, "pass=", input_pass, sizeof(input_pass));

    char query[300];
    sprintf(query, "SELECT name, password FROM students WHERE student_id = '%s'", student_id);

    if (mysql_query(con, query)) {
        json_error("Database error");
        return;
    }

    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) {
        json_error("Could not retrieve results");
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);

    if (row == NULL) {
        json_error("Student not found");
    }
    else {
        char *db_pass = row[1];
        if (db_pass != NULL && strcmp(input_pass, db_pass) == 0) {
            printf("{\"status\":\"ok\",\"action\":\"student_dashboard\",\"student_id\":\"%s\",\"name\":\"%s\"}", student_id, row[0]);
        }
        else {
            json_error("Incorrect password");
        }
    }

    mysql_free_result(res);
}

/* ==================== STUDENT: POINTS SUMMARY ==================== */
void show_student_summary(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char student_id[20] = {0};
    get_value(data, "student_id=", student_id, sizeof(student_id));

    if (strlen(student_id) == 0) { json_error("Student ID is required"); return; }

    char query[1000];

    /* Get student name and basic info */
    sprintf(query, "SELECT name, father_name FROM students WHERE student_id = '%s'", student_id);
    if (mysql_query(con, query)) { json_error("Database error"); return; }
    MYSQL_RES *info_res = mysql_store_result(con);
    if (info_res == NULL) { json_error("Could not retrieve results"); return; }
    MYSQL_ROW info_row = mysql_fetch_row(info_res);
    if (info_row == NULL) { json_error("Student not found"); mysql_free_result(info_res); return; }

    char s_name[100] = {0}, f_name[100] = {0};
    strncpy(s_name, info_row[0] ? info_row[0] : "", 99);
    strncpy(f_name, info_row[1] ? info_row[1] : "", 99);
    mysql_free_result(info_res);

    /* Get totals */
    sprintf(query, "SELECT COALESCE(SUM(points_awarded),0), COALESCE(SUM(max_possible),0), COUNT(*) FROM points_log WHERE student_id = '%s'", student_id);
    if (mysql_query(con, query)) { json_error("Database error"); return; }
    MYSQL_RES *sum_res = mysql_store_result(con);
    if (sum_res == NULL) { json_error("Could not retrieve results"); return; }
    MYSQL_ROW sum_row = mysql_fetch_row(sum_res);

    char *total = (sum_row && sum_row[0]) ? sum_row[0] : "0";
    char *max_pts = (sum_row && sum_row[1]) ? sum_row[1] : "0";
    char *total_entries = (sum_row && sum_row[2]) ? sum_row[2] : "0";

    printf("{\"status\":\"ok\",\"action\":\"summary\","
           "\"name\":\"%s\",\"father_name\":\"%s\","
           "\"student_id\":\"%s\","
           "\"total_points\":\"%s\",\"max_points\":\"%s\",\"total_entries\":\"%s\",",
           s_name, f_name, student_id, total, max_pts, total_entries);
    mysql_free_result(sum_res);

    /* Get recent activity (last 5) */
    sprintf(query, "SELECT p.question_desc, p.points_awarded, p.max_possible, p.teacher_name, p.date_given "
                   "FROM points_log p WHERE p.student_id = '%s' ORDER BY p.date_given DESC LIMIT 5", student_id);
    if (mysql_query(con, query)) {
        printf("\"recent\":[]}");
        return;
    }
    MYSQL_RES *rec_res = mysql_store_result(con);
    if (rec_res == NULL) {
        printf("\"recent\":[]}");
        return;
    }

    printf("\"recent\":[");
    MYSQL_ROW rec_row;
    int first = 1;
    while ((rec_row = mysql_fetch_row(rec_res))) {
        if (!first) printf(",");
        printf("{\"topic\":\"%s\",\"awarded\":\"%s\",\"max\":\"%s\",\"teacher\":\"%s\",\"date\":\"%s\"}",
               rec_row[0] ? rec_row[0] : "",
               rec_row[1] ? rec_row[1] : "0",
               rec_row[2] ? rec_row[2] : "0",
               rec_row[3] ? rec_row[3] : "",
               rec_row[4] ? rec_row[4] : "");
        first = 0;
    }
    printf("]}");
    mysql_free_result(rec_res);
}

/* ==================== STUDENT: FULL POINTS HISTORY ==================== */
void show_student_history(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char student_id[20] = {0};
    get_value(data, "student_id=", student_id, sizeof(student_id));

    if (strlen(student_id) == 0) { json_error("Student ID is required"); return; }

    char query[1000];
    sprintf(query, "SELECT p.question_desc, p.points_awarded, p.max_possible, p.teacher_name, p.date_given "
                   "FROM points_log p WHERE p.student_id = '%s' ORDER BY p.date_given DESC", student_id);

    if (mysql_query(con, query)) { json_error("Error fetching history"); return; }
    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) { json_error("Could not retrieve history"); return; }

    printf("{\"status\":\"ok\",\"action\":\"student_history\",\"records\":[");
    MYSQL_ROW row;
    int first = 1;
    while ((row = mysql_fetch_row(res))) {
        if (!first) printf(",");
        printf("{\"topic\":\"%s\",\"awarded\":\"%s\",\"max\":\"%s\",\"teacher\":\"%s\",\"date\":\"%s\"}",
               row[0] ? row[0] : "",
               row[1] ? row[1] : "0",
               row[2] ? row[2] : "0",
               row[3] ? row[3] : "",
               row[4] ? row[4] : "");
        first = 0;
    }
    printf("]}");
    mysql_free_result(res);
}

/* ==================== STUDENT: CHANGE PASSWORD ==================== */
void change_student_password(MYSQL *con, char *data) {
    if (con == NULL) { json_error("No database connection"); return; }

    char student_id[20] = {0}, old_pass[50] = {0}, new_pass[50] = {0};
    get_value(data, "student_id=", student_id, sizeof(student_id));
    get_value(data, "old_pass=", old_pass, sizeof(old_pass));
    get_value(data, "new_pass=", new_pass, sizeof(new_pass));

    if (strlen(student_id) == 0 || strlen(old_pass) == 0 || strlen(new_pass) == 0) {
        json_error("All fields are required");
        return;
    }

    /* Verify old password first */
    char query[500];
    sprintf(query, "SELECT password FROM students WHERE student_id = '%s'", student_id);
    if (mysql_query(con, query)) { json_error("Database error"); return; }
    MYSQL_RES *res = mysql_store_result(con);
    if (res == NULL) { json_error("Could not retrieve results"); return; }
    MYSQL_ROW row = mysql_fetch_row(res);

    if (row == NULL) {
        json_error("Student not found");
        mysql_free_result(res);
        return;
    }

    if (row[0] == NULL || strcmp(old_pass, row[0]) != 0) {
        json_error("Current password is incorrect");
        mysql_free_result(res);
        return;
    }
    mysql_free_result(res);

    /* Update to new password */
    sprintf(query, "UPDATE students SET password = '%s' WHERE student_id = '%s'", new_pass, student_id);
    if (mysql_query(con, query)) {
        json_error("Could not update password");
    } else {
        printf("{\"status\":\"ok\",\"message\":\"Password changed successfully!\"}");
    }
}