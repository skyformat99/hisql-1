#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <iostream>
#include <list>
#include <vector>

#include <stdlib.h>
#include <string.h>

using namespace std;

#define FIELD_MAX_SIZE 100
#define COMMENT_MAX_SIZE 256
#define EXTRA_MAX_SIZE 32
#define KEY_TYPE_SIZE 4
#define NULL_TYPE_SIZE 4
#define D_BUFF_SIZE 1024

#define VERSION "v1.0 Beta"

extern char ROOTPATH[];
extern char DBFPATH[];

enum Status {S_NORMAL, S_DEBUG, S_NOTICE, S_WARNNING, S_ERROR};

void g_plog(int num, Status st, char const * code, string custom, int line);

void dbms_start();
void dbms_help();
void dbms_info();

enum DataType {INT, CHAR, FLOAT};

struct Column
{
    char field[FIELD_MAX_SIZE];
    DataType type;
    int length;
    bool u_sign; /// if unsigned
    char null[NULL_TYPE_SIZE];
    char key[KEY_TYPE_SIZE];
    char * default_val;
    char comment[COMMENT_MAX_SIZE];
    char extra[EXTRA_MAX_SIZE];
};

struct Field
{
    char Field[FIELD_MAX_SIZE];
    int length;
};

typedef vector<Field> Fields;

typedef vector<char *> Row;

typedef list<Row> DataSet;

typedef list<Column> ColumnList;

void g_print_table(vector<string> &target, int col);

void g_print_dataset(DataSet &ds);

string g_rand_string(int min, int max);

int g_rand_strings(vector<string> &target, int l_min, int l_max, int s_min, int s_max);

int g_rand(int a, int b);

bool g_file_dir(string filename, int type);

void g_strlwr(char *p);

inline string& g_trim(string &word, char c)
{
    if (word.empty()) return word;
    word.erase(0, word.find_first_not_of(c));
    word.erase(word.find_last_not_of(c) + 1);
    return word;
}

inline char* g_getword(char *p, string &word, bool change_case)
{
    while (*p == ' ') ++p;
    word = "";
    if (change_case) g_strlwr(p);
    while (*p != ' ' && *p != '\0')
    {
        word += *p;
        ++p;
        if (change_case) g_strlwr(p);
    }
    while (*p == ' ') ++p;
    return p;
}

inline char* g_getword_mod(char *p, string &word, bool change_case, char const shift, char const endchar)
{
    while (*p == shift) ++p;
    word = "";
    if (change_case) g_strlwr(p);
    while (*p != endchar && *p != '\0')
    {
        word += *p;
        ++p;
        if (change_case) g_strlwr(p);
    }
    while (*p == shift) ++p;
    return p;
}

inline int g_check_int(string &str, int lenght, string defval, bool unsign)
{
    int len = str.length();
    if (len == 0 || str == "NULL")
    {
        str = defval;
        return 0;
    }
    if (len > lenght) return -3;
    if (str[0] == '-' && unsign) return -1;
    for (int i = 0; i < len; ++i)
    {
        if (str[i] < '0' || str[i] > '9') return -2;
    }
    return 1;
}

inline int g_check_float(string str, int lenght, string defval, bool unsign)
{
    int len = str.length();
    if (len == 0 || str == "NULL")
    {
        str = defval;
        return 0;
    }
    if (len > lenght) return -3;
    if (str[0] == '-' && unsign) return -1;
    bool dot = false;
    for (int i = 0; i < len; ++i)
    {
        if (str[i] == '.')
        {
            if (dot) return -2;
            dot = true;
            continue;
        }
        if (str[i] < '0' || str[i] > '9') return -2;
    }
    return 1;
}

inline int g_check_char(string str, int lenght, string defval)
{
    int len = str.length();
    if (len == 0 || str == "NULL")
    {
        str = defval;
        return 0;
    }
    if (len > lenght) return -3;
    return 1;
}

inline void g_shiftdot(string &str, char const c)
{
    while (str[0] == c && str[str.length() - 1] == c)
    {
        str = str.substr(1);
        str = str.substr(0, (str.length() - 1));
    }
}

inline int g_convert_col(string tmp, Column &col)
{
    strcpy(col.field, "");
    col.type = INT;
    col.length = 8;
    col.u_sign = false;
    col.default_val = NULL;
    strcpy(col.null, "YES");
    strcpy(col.key, "");
    strcpy(col.comment, "");
    strcpy(col.extra, "");

    string colname = "";
    char *p = (char*)tmp.c_str();

    p = g_getword_mod(p, colname, false, ' ', ' ');
    g_shiftdot(colname, '`');

    strcpy(col.field, colname.c_str());

    string str = "";
    p = g_getword_mod(p, str, true, ' ', '(');
    if (str.find("unsigned") != string::npos)
    {
        col.u_sign = true;
        while (*p != ' ') --p;
        p = g_getword_mod(p, str, true, ' ', '(');
    }
    if (str == "int") col.type = INT;
    else if (str == "float") col.type = FLOAT;
    else if (str == "char") col.type = CHAR;
    else return false;

    if (*p == '(')
    {
        ++p;
        p = g_getword_mod(p, str, false, ' ', ')');
        col.length = atoi(str.c_str());
        col.default_val = new char[col.length];
        strcpy(col.default_val, "");
        if (*p != ')') return false;
        ++p;
        do {
            p = g_getword(p, str, true);
            if (str == "not")
            {
                p = g_getword(p, str, true);
                if (str == "null")
                {
                    strcpy(col.null, "NO");
                }
                else return false;
            }
            else if (str == "primary")
            {
                p = g_getword(p, str, true);
                if (str == "key")
                {
                    strcpy(col.key, "PRI");
                    strcpy(col.null, "NO");
                }
                else return false;
            }
            else if (str == "default")
            {
                int ret = 0;
                p = g_getword(p, str, true);
                if (col.type == CHAR)
                {
                    if (str[0] == '\'' && str[str.length() - 1] == '\'')
                    {
                        g_shiftdot(str, '\'');
                        ret = g_check_char(str, col.length, "");
                    }
                    else return false;
                }
                else if (col.type == FLOAT)
                {
                    ret = g_check_float(str, col.length, "", col.u_sign);
                }
                else if (col.type == INT)
                {
                    ret = g_check_int(str, col.length, "", col.u_sign);
                }
                if (ret < 0) return false;
                strcpy(col.default_val, str.c_str());
            }
            else if (str == "auto_increment")
            {
                strcpy(col.extra, "auto_increment");
            }
            else if (str == "comment")
            {
                p = g_getword(p, str, true);
                g_trim(str, ' ');
                strcpy(col.comment, str.c_str());
                cout << col.comment << endl;
            }
        }
        while (*p != '\0');
    }

    if (col.default_val == NULL)
    {
        col.default_val = new char[col.length];
        strcpy(col.default_val, "");
    }
    return true;
}

/// define the opt of relation

DataSet& grc_union(DataSet& a, DataSet& b);

DataSet& grc_intersection(DataSet& a, DataSet& b);

DataSet& grc_difference(DataSet& a, DataSet& b);

DataSet& grc_cartesian_product(DataSet& a, DataSet& b);

DataSet& grc_restrict(DataSet& a, int colnum, char const * opt, string value);

DataSet& grc_project(DataSet& a, Fields fields);

DataSet& grc_join(DataSet& a, DataSet& b);

#endif // COMMON_H_INCLUDED
