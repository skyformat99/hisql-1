#include <iostream>
#include <iomanip>
#include <list>
#include <vector>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>

#include "common.h"

using namespace std;

char ROOTPATH[D_BUFF_SIZE];
char DBFPATH[D_BUFF_SIZE];

void g_plog(int num, Status st, char const * code, string custom, int line)
{
    char log[D_BUFF_SIZE];
    switch(st)
    {
    case S_DEBUG:
        strcpy(log, "DEBUG %d (%s): %s");
        break;
    case S_NOTICE:
        strcpy(log, "NOTICE %d (%s): %s");
        break;
    case S_WARNNING:
        strcpy(log, "WARNNING %d (%s): %s");
        break;
    case S_ERROR:
        strcpy(log, "ERROR %d (%s): %s");
        break;
    default:
        return;
        break;
    }
    char content[D_BUFF_SIZE];
    switch(num)
    {
    case 1007:
        sprintf(content, "Can't create database '%s'; database exists\n", custom.c_str());
        break;
    case 1046:
        sprintf(content, "No database selected\n");
        break;
    case 1049:
        sprintf(content, "Unknown database '%s'\n", custom.c_str());
        break;
    case 1051:
        sprintf(content, " Unknown table '%s'\n", custom.c_str());
        break;
    case 1064:
        sprintf(content, "You have an error in your SQL syntax; \
check the manual that corresponds to your HiSQL server version for\
 the right syntax to use near '%s' at line %d\n", custom.c_str(), line);
        break;
    case 1091:
        sprintf(content, "Can't DROP '%s'; check that column/key exists\n", custom.c_str());
        break;
    case 1146:
        sprintf(content, "Table '%s' doesn't exist\n", custom.c_str());
        break;
    case 9999:
        sprintf(content, "Table '%s' file can't read\n", custom.c_str());
        break;
    default:
        return;
        break;
    }
    printf(log, num, code, content);
}

void dbms_start()
{
    _getcwd(ROOTPATH, D_BUFF_SIZE);

    strcat(ROOTPATH, "\\");

    strcpy(DBFPATH, ROOTPATH);
    strcat(DBFPATH, "data\\");

    if (! g_file_dir(DBFPATH, 0))
    {
        int ret = _mkdir(DBFPATH);
        if (ret < 0)
        {
            perror(DBFPATH);
            exit(-1);
        }
    }
}

void dbms_help()
{
    cout <<
         endl << "For information about HiSQL, visit:" <<
         endl << "   http://kenx.cn/" <<
         endl << "For developer information, including the HiSQL Source Code, visit:" <<
         endl << "   https://github.com/kenxx/hisql" <<
         endl << "To buy HiSQL Enterprise support, training, or other products, visit:" <<
         endl << "   eh,,,," << endl <<
         endl << "List of all HiSQL commands:" <<
         endl << "Note that all text commands must be first on line and end with ';'" <<
         endl << "?         (\\?) Synonym for `help'." <<
         endl << "exit      (\\q) Exit mysql. Same as quit." <<
         endl << "help      (\\h) Display this help." <<
         endl << "quit      (\\q) Quit mysql." <<
         endl << "status    (\\s) Get status information from the server." << endl <<

         endl << "For server side help, type 'help contents'" << endl << endl;
}

void dbms_info()
{
    cout <<
         "Welcome to the HiSQL monitor.  Commands end with ; or \\g.\n\
Server version: " << VERSION << "HiSQL Kenneth Experiment (PRI)\n\n\
Copyright (c) 2015, Kenneth Hawk (Huo Yuheng). All rights reserved.\n\n\
Kenneth is the author's English name, he write this system for\n\
test the all (maybe not all) the functions that some DBMS has\n\
and it's a experiment for studying database.\n\n\
Type 'help;' or '\\h' for help.\n"
         << endl;
}

void g_print_table(vector<string> &target, int col = 1)
{
    int tsize = target.size();
    if (tsize == 0)
    {
        cout << "Empty." << endl;
        return;
    }
    if (tsize == 1)
    {
        target.push_back("");
    }
    if (tsize % col != 0)
    {
        int append = col - (tsize % col);
        for (int i = 0; i < append; ++i) target.push_back("");
        tsize = target.size();
    }

    int strmax[col];
    for (int i = 0; i < col; ++i) strmax[i] = 0; /// fill the array

    for (int i = 0; i < tsize; ++i)
    {
        size_t j = i % col;
        int len = target.at(i).length();
        if (len > strmax[j]) strmax[j] = len;
    }

    string line = "";

    for (int i = 0; i < col; ++i)
    {
        line += "+-";
        for (int j = 0; j <= strmax[i]; ++j) line += '-';
    }
    line += '+';

    cout << line << endl << left;

    for (int i = 0; i < col; ++i)
    {
        cout << "| " << setw(strmax[i]);
        cout << target.at(i) << " ";
    }
    cout << "|" << endl;

    cout << line << endl;

    for (int i = col; i < tsize; ++i)
    {
        cout << "| " << setw(strmax[i % col]);
        cout << target.at(i) << " ";
        if (i % col == (col - 1)) cout << "|" << endl;
    }

    cout << line << endl;

    return;
}

void g_print_dataset(DataSet &ds)
{
    int rowsize = ds.size();

    if (rowsize < 1)
    {
        cout << "Empty." << endl;
        return;
    }

    int colsize = ds.front().size();
    vector<string> printtab;
    DataSet::iterator beg, end = ds.end();
    while (beg != end)
    {
        for (int i = 0; i < colsize; ++i)
        {
            printtab.push_back((*beg).at(i));
        }
        ++beg;
    }
    g_print_table(printtab, colsize);
}

string g_rand_string(int min, int max)
{
    string x("");
    int len = g_rand(min, max);
    for (int j = 0; j < len; ++j)
    {
        int type = rand() % 100;
        if (type < 30)
        {
            x += (char)(g_rand('0', '9'));
        }
        else if (type > 30 && type < 60)
        {
            x += (char)(g_rand('A', 'Z'));
        }
        else
        {
            x += (char)(g_rand('a', 'z'));
        }
    }
    return x;
}

int g_rand_strings(vector<string> &target,
                   int l_min = 1, int l_max = 50,
                   int s_min = 1, int s_max = 15)
{
    int len = g_rand(l_min, l_max);

    for (int i = 0; i < len; ++i)
    {
        string x("");
        int len = g_rand(s_min, s_max);
        for (int j = 0; j < len; ++j)
        {
            x += (char)(g_rand(97, 122));
        }
        target.push_back(x);
    }

    return len;
}

int g_rand(int a, int b)
{
    return rand() % (b - a + 1) + a;
}

bool g_file_dir(string filename, int type)
{
    if (_access(filename.c_str(), type) == 0) return true;
    //perror(filename.c_str());
    return false;
}

void g_strlwr(char *p)
{
    if ((*p) >= 'A' && (*p) <= 'Z')
    {
        (*p) += 32;
    }

}
