#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <iostream>
#include <list>
#include <vector>

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

void g_print_table(vector<string> &target, int col);

int g_rand_strings(vector<string> &target, int l_min, int l_max, int s_min, int s_max);

int g_rand(int a, int b);

bool g_file_dir(string filename, int type);

void g_strlwr(char *p);

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

inline void g_shiftdot(string &str, char const c)
{
    while (str[0] == c && str[str.length() - 1] == c)
    {
        str = str.substr(1);
        str = str.substr(0, (str.length() - 1));
    }
}

#endif // COMMON_H_INCLUDED
