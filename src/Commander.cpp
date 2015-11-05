#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <io.h>

#include "Commander.h"

using namespace std;

Commander::Commander()
{
    /// set current database to null
    CURRENT_DB = NULL;

    /// set default user
    user = "";

    /// load system database
    SYSTEM_DB = loadDB("hisql");

    if (SYSTEM_DB == NULL)
    {
        cp_loadSystemDB();
    }
}

Commander::~Commander()
{
    //dtor
}

Database* Commander::loadDB(string dbname)
{
    list<Database>::iterator beg = this->databases.begin(), end = this->databases.end();

    for (; beg != end; ++beg)
    {
        if ((*beg).getName() == dbname)
        {
            return &(*beg);
        }
    }

    if (g_file_dir(DBFPATH + dbname, 0))
    {
        Database tmp(dbname);
        databases.push_back(tmp);
        return &(databases.back());
    }
    else
    {
        return NULL;
    }
}

void Commander::reader()
{
    bool quit = false;
    do
    {
        cout << "hisql> ";
        char buff[BUFFSIZE];
        cin.getline(buff, BUFFSIZE);
        if (strlen(buff) != 0)
        {
            if (strcmp(buff, "exit") == 0 || strcmp(buff, "quit") == 0)
            {
                quit = true;
            }
            else if (strcmp(buff, "help") == 0 || strcmp(buff, "?") == 0)
            {
                dbms_help();
            }
            else if (strcmp(buff, "status") == 0)
            {
                ;
            }
            else if (buff[0] == '\\')
            {
                switch (buff[1])
                {
                case 'q':
                case 'e':
                    quit = true;
                    break;
                case '?':
                case 'h':
                    dbms_help();
                    break;
                case 's':
                    cp_nowStatus();
                    break;
                case 'u':
                {
                    char *p;
                    p = buff+2;
                    while (*p == ' ') ++p;
                    string tmpdb("");
                    while (*p != '\0')
                    {
                        tmpdb += *p;
                        ++p;
                    }
                    if (tmpdb.length() == 0)
                    {
                        cout << "ERROR:" << endl
                             << "USE must be followed by a database name" << endl;
                    }
                    else
                    {
                        if (changeDB(tmpdb) < 0)
                        {
                            g_plog(1049, S_ERROR, "42000", tmpdb, 0);
                        }
                    }
                    break;
                }
                default:
                    cout << "ERROR:" << endl << "Unknown command '" << buff << "'." << endl;
                    break;
                }
            }
            else if(buff[0] == 'u')
            {
                if (buff[1] == 's' && buff[2] == 'e')
                {
                    char *p;
                    p = buff+3;
                    while (*p == ' ') ++p;
                    string tmpdb("");
                    while (*p != '\0')
                    {
                        tmpdb += *p;
                        ++p;
                    }
                    if (tmpdb.length() == 0)
                    {
                        cout << "ERROR:" << endl
                             << "USE must be followed by a database name" << endl;
                    }
                    else
                    {
                        if (changeDB(tmpdb) < 0)
                        {
                            g_plog(1049, S_ERROR, "42000", tmpdb, 0);
                        }
                    }
                }
            }
            else
            {
                /** just sending the buff to commander */
                strcpy(this->m_cmd_line, buff);
                this->exec();
            }
        }
    }
    while(!quit);
    cout << "Bye." << endl;
}

void Commander::exec()
{
    char *p = m_cmd_line; /// pointer it

    string str = ""; /// compare string

    p = g_getword(p, str, true); /// get the first word

    Database * e_opt_db = NULL; /// target db

    Table * e_opt_tab = NULL; /// target table

    Operation opt = OPT_NONE; /// operation

    bool has_error = false; /// if SQL has error

    struct SELECT_DATA /// select data
    {
        vector<string> tables; /// multiple table
        vector<string> fields; /// multiple fields
        vector<string> alias; /// multiple fields alias
        string condition;
    };

    SELECT_DATA * sd; /// select data things

    Column * altercol; /// alter one column

    ColumnList createtab; /// to create a new tab

    struct VIEW_DATA
    {
        string name;
        vector<string> fields;
        string sql_line;
    };

    string something; /// create db, table name, or something

    /** just let it know what opt */
    if (str == "alter") /// ALTER...
    {
        p = g_getword(p, str, true);
        if (str == "table") /// ALTER TABLE...
        {
            p = g_getword(p, str, false);
            g_shiftdot(str, '`');
            size_t dp = str.find('.');
            if (dp != string::npos) /// found '.'
            {
                e_opt_db = loadDB(str.substr(0, dp)); /// load database
                if (e_opt_db == NULL) /// if can't find db
                {
                    g_plog(1146, S_ERROR, "42S00", str, 0);
                    return;
                }
                str = str.substr(dp + 1);
            }
            else /// if not found '.'
            {
                if (CURRENT_DB == NULL) /// not selected db
                {
                    g_plog(1046, S_ERROR, "3D000", str, 0);
                    return;
                }
                e_opt_db = CURRENT_DB;
            }
            e_opt_tab = e_opt_db->getTable(str); /// get table
            if (e_opt_tab == NULL) /// if can't find the table
            {
                g_plog(1146, S_ERROR, "42S01", e_opt_db->getName() + "." + str, 0);
                return;
            }
            p = g_getword(p, str, true); /// get add or drop column
            if (str == "add") /// ALTER TABLE [TBN] ADD...
            {
                p = g_getword_mod(p, str, false, ' ', '\0');
                altercol = new Column;
                if (!g_convert_col(str, *altercol))
                {
                    has_error = true;
                }
                opt = OPT_ALTER_TABLE_ADD; /// IMPORTANT
            }
            else if (str == "drop") /// ALTER TABLE [TBN] DROP...
            {
                p = g_getword(p, str, true);
                if (str == "column") /// ALTER TABLE [TBN] DROP COLUMN...
                {
                    p = g_getword(p, str, false);
                    g_shiftdot(str, '`');
                    altercol = e_opt_tab->getField(str);
                    if (altercol == NULL)
                    {
                        g_plog(1091, S_ERROR, "42000", str, 0);
                        return;
                    }
                    opt = OPT_ALTER_TABLE_DROP_COLUMN; /// IMPORTANT
                }
            }
        }
        else if (str == "database") /// ALTER DATABASE...
        {
            ;
        }
    }
    else if (str == "create") /// CREATE...
    {
        p = g_getword(p, str, true);
        if (str == "table") /// CREATE TABLE...
        {
            p = g_getword_mod(p, str, true, ' ', '('); /// get table name
            str = g_trim(str, ' '); /// shift space
            if (str[0] == '`' && str[str.length() - 1] == '`') /// shift dot
            {
                g_shiftdot(str, '`'); /// just shift it
            }
            something = str;
            if (*p == '(') /// if has some col
            {
                Column acol;
                while (*p != '\0') /// just to the end
                {
                    p = g_getword_mod(p, str, false, ' ', ',');
                    if (*p != ',')
                    {
                        str = g_trim(str, ' ');
                        if (str[str.length() - 1] == ')')
                        {
                            str = str.substr(str.length() - 1);
                            str = g_trim(str, ' ');
                        }
                        else
                        {
                            has_error = true;
                        }
                    }
                    g_convert_col(str, acol);
                    createtab.push_back(acol);
                }
            }
            opt = OPT_CREATE_TABLE;
        }
        else if (str == "view")/// CREATE VIEW...
        {
            p = g_getword(p, str, false);
            if (CURRENT_DB == NULL) /// if current db is null
            {
                g_plog(1046, S_ERROR, "3D000", str, 0);
                return;
            }
            g_trim(str, ' ');
            something = str;
            p = g_getword(p, str, true);
            if (str == "as") /// CREATE VIEW [VIEWNAME] AS...
            {
                p = g_getword_mod(p, str, false, ' ', '\0');
                e_opt_db = CURRENT_DB;
                opt = OPT_CREATE_VIEW;
            }
            else
            {
                has_error = true;
            }
        }
        else if (str == "index")/// CREATE INDEX...
        {
            ;
        }
    }
    else if (str == "drop") /// DROP...
    {
        p = g_getword(p, str, true);
        if (str == "table")/// DROP TABLE...
        {
            p = g_getword(p, str, false);
            g_trim(str, ' ');
            if (CURRENT_DB == NULL)
            {
                if (str.find('.') == string::npos) /// can't find db
                {
                    g_plog(1046, S_ERROR, "3D000", str, 0);
                    return;
                }
                e_opt_db = loadDB(str.substr(0, str.find('.'));
                str = str.substr(str.find('.') - 1);
                if (e_opt_db == NULL)
                {
                    g_plog(1051, S_ERROR, "42S02", dbname + "." + str, 0);
                    return;
                }
            }
            else
            {
                e_opt_db = CURRENT_DB;
            }
            e_opt_tab = e_opt_db->getTable(str);
            if (e_opt_tab == NULL)
            {
                g_plog(1051, S_ERROR, "42S02", e_opt_db->getName() + "." + str, 0);
                return;
            }
            opt = OPT_DROP_TABLE;
        }
        else if (str == "database")/// DROP DATABASE...
        {
            p = g_getword(p, str, false);
            g_trim(str, ' ');
            e_opt_db = loadDB(str);
            if (e_opt_db != NULL)
            {
                opt = OPT_DROP_DATABASE;
            }
        }
        else if (str == "view")/// DROP VIEW...
        {
            ;
        }
        else if (str == "index")/// DROP INDEX...
        {
            ;
        }
    }
    else if (str == "delete") /// DELETE...
    {
        p = g_getword(p, str, true);
        if (str == "*") /// DELETE *...
        {
            p = g_getword(p, str, true);
            if (str == "from") /// DELETE * FROM...
            {
                ;
            }
        }
        else if (str == "from") /// DELETE FROM...
        {
            ;
        }
    }
    else if (str == "grant") /// GRANT...
    {
        ;
    }
    else if (str == "insert") /// INSERT...
    {
        g_getword(p, str, true);
        if (str == "into") /// INSERT INTO
        {
            ;
        }
    }
    else if (str == "show") /// SHOW...
    {
        if (str == "databases")
        {
            ;
        }
        else if (str == "tables")
        {
            ;
        }
        else if (str == "columns")
        {
            ;
        }
    }
    else if (str == "select") /// SELECT...
    {
        ;
    }


    /** all check, now opt it */
    if (opt != OPT_NONE && !has_error)
    {
        cout << "DB: " << e_opt_db->getName() << endl;
        cout << "Table: " << e_opt_tab->getName() << endl;
    }
    else
    {
        g_plog(1064, S_ERROR, "42000", str, 0);
    }
}

int Commander::createDB(string dbname)
{
    if (! g_file_dir(DBFPATH + dbname, 0))
    {
        int ret = _mkdir(string(DBFPATH + dbname).c_str());
        if (ret < 0)
        {
            perror(string(DBFPATH + dbname).c_str());
            return -1;
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

int Commander::changeDB(string dbname)
{
    if (dbname == "") return 0;

    Database * tmp = loadDB(dbname);

    if (tmp != NULL)
    {
        CURRENT_DB = tmp;
        cout << "Database changed" << endl;
        return 0;
    }
    return -1;
}

int Commander::deleteDB(string dbname)
{
    string dbpath = DBFPATH + dbname;

    if (g_file_dir(dbpath, 0))
    {
        int ret = _rmdir(dbpath.c_str());
        if (ret < 0)
        {
            dbpath += "\\";
            if(_chdir(dbpath.c_str()) == 0)
            {
                _finddata_t file;
                int k;
                long HANDLE;
                k = HANDLE = _findfirst("*", &file);
                while (k != -1)
                {
                    cout << file.name << endl;
                    k = _findnext(HANDLE, &file);
                }
                _findclose(HANDLE);

                return 0;
            }
            perror(dbpath.c_str());
            return -1;
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

void Commander::cp_nowStatus()
{
    string db = "";
    if (CURRENT_DB != NULL) db = CURRENT_DB->getName();
    cout << "--------------" <<
         endl << "HiSQL Ver " << VERSION << ", for Windows x86" << endl <<
         endl << "Current database:       " << db <<
         endl << "Current user:           " << user << "@localhost" <<
         endl << "Using delimiter:        ;" <<
         endl << "Server version:         " << VERSION << endl <<
         endl << "Opens: 0    Open tables: 0" <<
         endl << "--------------" << endl;
}

void Commander::cp_loadSystemDB()
{
    string system_db_name = "hisql";
    string hipath(DBFPATH + system_db_name);

    /// test HISQL file
    if (! g_file_dir(hipath, 0))
    {
        createDB(system_db_name);
    }
    hipath += "\\";

    SYSTEM_DB = loadDB(system_db_name);

    Table * tmptab;
    vector<string> arow;
    /// test HISQL Database table
    tmptab = SYSTEM_DB->getTable("db");
    if (tmptab == NULL)
    {
        tmptab = SYSTEM_DB->addTable("db");
        if (tmptab == NULL)
        {
            cout << "Fatal Error: can't not create system table 'db'." << endl;
            exit(-1);
        }
        /// temp col
        Column col;
        /// set id
        g_convert_col("id int(10) primary key", col);
        tmptab->addField(col);
        /// set db_name
        g_convert_col("db_name char(200) not null", col);
        tmptab->addField(col);
        /// set table_count
        g_convert_col("table_count int(10) default 0", col);
        tmptab->addField(col);
        arow.clear();
        arow.push_back("0");
        arow.push_back("db");
        arow.push_back("3");
        tmptab->pushRow(arow);
    }

    /// test HISQL User Table
    tmptab = SYSTEM_DB->getTable("user");
    if (tmptab == NULL)
    {
        tmptab = SYSTEM_DB->addTable("user");
        if (tmptab == NULL)
        {
            cout << "Fatal Error: can't not create system table 'db'." << endl;
            exit(-1);
        }
        /// temp col
        Column col;
        /// set id
        g_convert_col("id int(10) primary key", col);
        tmptab->addField(col);
        /// set user name
        g_convert_col("user char(32) not null", col);
        tmptab->addField(col);
        /// set password
        g_convert_col("pass char(40) default 123456", col);
        tmptab->addField(col);
        /// set privilege
        g_convert_col("prvi char(11) default 'NNNNNNNNNN'", col);
        tmptab->addField(col);
        arow.clear();
        arow.push_back("0");
        arow.push_back("root");
        arow.push_back("666888");
        arow.push_back("YYYYYYYYYY");
        tmptab->pushRow(arow);
    }

    /// test HISQL table Table
    tmptab = SYSTEM_DB->getTable("table");
    if (tmptab == NULL)
    {
        tmptab = SYSTEM_DB->addTable("table");
        if (tmptab == NULL)
        {
            cout << "Fatal Error: can't not create system table 'db'." << endl;
            exit(-1);
        }
        /// temp col
        Column col;
        /// set id
        g_convert_col("id int(10) primary key", col);
        tmptab->addField(col);
        /// set db_id
        g_convert_col("db_id int(10) not null", col);
        tmptab->addField(col);
        /// set table_name
        g_convert_col("table_name char(200) not null", col);
        tmptab->addField(col);
        /// set column_count
        g_convert_col("column_count int(10) default 0", col);
        tmptab->addField(col);
        /// add db table
        arow.clear();
        arow.push_back("0");
        arow.push_back("0");
        arow.push_back("db");
        arow.push_back("3");
        tmptab->pushRow(arow);
        /// add user table
        arow.clear();
        arow.push_back("1");
        arow.push_back("0");
        arow.push_back("user");
        arow.push_back("4");
        tmptab->pushRow(arow);
        /// add table table
        arow.clear();
        arow.push_back("2");
        arow.push_back("0");
        arow.push_back("table");
        arow.push_back("4");
        tmptab->pushRow(arow);
    }
}
