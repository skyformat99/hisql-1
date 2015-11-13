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
    cp_loadSystemDB();
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
    Database * e_opt_db = NULL; /// target db
    Table * e_opt_tab = NULL;   /// target table
    Operation opt = OPT_NONE;   /// operation
    bool has_error = false;     /// if SQL has error
    vector<string> select_tables;      /// select multiple table
    vector<string> select_fields;      /// select multiple fields
    vector<string> select_conditions;  /// select multiple conditions
    Column * altercol;      /// alter one column
    ColumnList createtab;   /// to create a new tab
    vector<string> insert_field;  /// field for insert
    vector<string> insert_value;  /// value for insert
    string something; /// create db, table name, or something

    char *p = m_cmd_line; /// pointer it
    string str = ""; /// compare string
    p = g_getword(p, str, true); /// get the first word

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
        if (str == "database") /// CREATE DATABASE...
        {
            p = g_getword(p, str, false);
            e_opt_db = loadDB(str);
            if (e_opt_db != NULL)
            {
                cout << "Database exist!" << endl;
                return;
            }
            something = str;
            opt = OPT_CREATE_DATABASE;
        }
        else if (str == "table") /// CREATE TABLE...
        {
            p = g_getword(p, str, false); /// get table name
            g_trim(str, ' ');
            something = str;
            if (str.find('.') != string::npos)
            {
                something = str.substr(0, str.find('.'));
                str = str.substr(str.find('.') + 1);
                e_opt_db = loadDB(something);
            }
            else
            {
                e_opt_db = CURRENT_DB;
            }
            if (e_opt_db == NULL)
            {
                g_plog(1051, S_ERROR, "42S02", something + "." + str, 0);
                return;
            }
            while (getline(cin, str)) /// just to the end
            {
                if (str == ";") break;
                Column acol;
                if (!g_convert_col(str, acol))
                {
                    cout << "Can not set this column" << endl;
                    return;
                }
                createtab.push_back(acol);
            }
            opt = OPT_CREATE_TABLE;
            /**
            str = g_trim(str, ' '); /// shift space
            if (str[0] == '`' && str[str.length() - 1] == '`') /// shift dot
            {
                g_shiftdot(str, '`'); /// just shift it
            }
            something = str;
            if (*p == '\0')
            {
                cout << "ERROR" << endl;
                return;
            }
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
                opt = OPT_CREATE_TABLE;
            */
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
            if (str.find('.') != string::npos)
            {
                something = str.substr(0, str.find('.'));
                str = str.substr(str.find('.') + 1);
                e_opt_db = loadDB(something);
            }
            else
            {
                e_opt_db = CURRENT_DB;
            }
            if (e_opt_db == NULL)
            {
                g_plog(1051, S_ERROR, "42S02", something + "." + str, 0);
                return;
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
                something = str;
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
        p = g_getword(p, str, true);
        if (str == "into") /// INSERT INTO
        {
            str = "";
            p = g_getword(p, str, false);
            /// find db
            if (str.find('.') != string::npos)
            {
                something = str.substr(0, str.find('.'));
                str = str.substr(str.find('.') + 1);
                e_opt_db = loadDB(something);
            }
            else
            {
                e_opt_db = CURRENT_DB;
            }
            if (e_opt_db == NULL)
            {
                g_plog(1051, S_ERROR, "42S02", something + "." + str, 0);
                return;
            }
            e_opt_tab = e_opt_db->getTable(str);
            if (e_opt_db == NULL)
            {
                g_plog(1051, S_ERROR, "42S02", something + "." + str, 0);
                return;
            }
            while (getline(cin, str))
            {
                if (str == ";") break;
                insert_value.push_back(str);
            }
            if (insert_value.size() != e_opt_tab->countCol())
            {
                cout << "can not insert" << endl;
                return;
            }
            opt = OPT_INSERT;
        }
    }
    else if (str == "show") /// SHOW...
    {
        p = g_getword(p, str, true);
        if (str == "databases")
        {
            Table * tmptab = SYSTEM_DB->getTable("db");
            vector<string> tmpv;
            tmpv.push_back("Database");
            int nameloc = tmptab->getFieldLoc("db_name");
            DataSet::iterator it = tmptab->getRowIte();
            while (! tmptab->IteEnd(it))
            {
                tmpv.push_back((*it).at(nameloc));
                ++it;
            }
            g_print_table(tmpv, 1);
            return;
        }
        else if (str == "tables") /// SHOW TABLES FROM...
        {
            p = g_getword(p, str, true);
            if (str == "from")
            {
                p = g_getword(p, str, false);
                Table * tmptab = SYSTEM_DB->getTable("db");
                int loc = tmptab->getFieldLoc("db_name");
                DataSet::iterator it = tmptab->getRowIte();
                while (! tmptab->IteEnd(it))
                {
                    if (strcmp((*it).at(loc), str.c_str()) == 0) /// found
                    {
                        loc = tmptab->getFieldLoc("id");
                        int dbid = atoi((*it).at(loc));
                        vector<string> tmpvec;
                        tmpvec.push_back("Table_in_" + str);
                        tmptab = SYSTEM_DB->getTable("table");
                        it = tmptab->getRowIte();
                        int dbidloc = tmptab->getFieldLoc("db_id");
                        int nameloc = tmptab->getFieldLoc("table_name");
                        while (! tmptab->IteEnd(it))
                        {
                            int id = atoi((*it).at(dbidloc));
                            if (id == dbid)
                            {
                                tmpvec.push_back((*it).at(nameloc));
                            }
                            ++it;
                        }
                        g_print_table(tmpvec, 1);
                        return;
                    }
                    ++it;
                }
                /// not found
                g_plog(1049, S_ERROR, "42000", str, 0);
                return;
            }
        }
        else if (str == "columns") /// SHOW COLUMNS FROM...
        {
            p = g_getword(p, str, true);
            if (str == "from")
            {
                p = g_getword(p, str, false);
                g_trim(str, ' ');
                if (str.find('.') != string::npos)
                {
                    something = str.substr(0, str.find('.'));
                    str = str.substr(str.find('.') + 1);
                    e_opt_db = loadDB(something);
                }
                else
                {
                    e_opt_db = CURRENT_DB;
                }
                if (e_opt_db == NULL)
                {
                    g_plog(1051, S_ERROR, "42S02", something + "." + str, 0);
                    return;
                }
                e_opt_tab = e_opt_db->getTable(str);
                if (e_opt_tab == NULL)
                {
                    g_plog(1051, S_ERROR, "42S02", e_opt_db->getName() + "." + str, 0);
                    return;
                }
                e_opt_tab->showColumns();
                return;
            }
        }
    }
    else if (str == "select") /// SELECT...
    {
        cout << "Field(s):";
        string tmp = "";
        while (cin >> tmp)
        {
            if (tmp == ";") break;
            select_fields.push_back(tmp);
        }
        cout << "From:";
        while (cin >> tmp)
        {
            if (tmp == ";") break;
            select_tables.push_back(tmp);
        }
        cout << "Condition:";
        while (cin >> tmp)
        {
            if (tmp == ";") break;
            select_conditions.push_back(tmp);
        }
        opt = OPT_SELECT;
    }


    /** all check, now opt it */
    if (opt != OPT_NONE && !has_error)
    {
        switch(opt)
        {
        case OPT_ALTER_TABLE_ADD:
            e_opt_tab->addField(*altercol);
            e_opt_tab->writeFile();
            break;
        case OPT_ALTER_TABLE_DROP_COLUMN:
            e_opt_tab->removeField(string(altercol->field));
            e_opt_tab->writeFile();
            break;
        case OPT_CREATE_DATABASE:
        {
            createDB(something);
            Table * db = SYSTEM_DB->getTable("db");
            db->readFile();
            vector<string> arow;
            char buff[10];
            sprintf(buff, "%d", db->countRow() + 1);
            arow.push_back(buff);
            arow.push_back(something);
            arow.push_back("0");
            db->pushRow(arow);
            db->writeFile();
            break;
        }
        case OPT_CREATE_TABLE:
        {
            Table * db = SYSTEM_DB->getTable("db");
            db->readFile();
            DataSet::iterator it = db->getRowIte();
            int loc = db->getFieldLoc("db_name"), id;
            string dbname = e_opt_db->getName();
            while (! db->IteEnd(it))
            {
                if (strcmp((*it).at(loc), dbname.c_str()) == 0)
                {
                    loc = db->getFieldLoc("table_count");
                    int c = atoi((*it).at(loc));
                    sprintf((*it).at(loc), "%d", c + 1);
                    id = atoi((*it).at(db->getFieldLoc("id")));
                    break;
                }
                ++it;
            }
            Table * tab = SYSTEM_DB->getTable("table");
            tab->readFile();
            vector<string> arow;
            char buff[10];
            sprintf(buff, "%d", tab->countRow() + 1);
            arow.push_back(buff);
            sprintf(buff, "%d", id);
            arow.push_back(buff);
            arow.push_back(something);
            sprintf(buff, "%d", createtab.size());
            arow.push_back(buff);
            tab->pushRow(arow);
            tab->writeFile();
            e_opt_tab = e_opt_db->addTable(something);
            if (e_opt_tab == NULL)
            {
                cout << "Create Table Failure!" << endl;
                return;
            }
            if (e_opt_tab->setFields(createtab) < 0)
            {
                cout << "Fail to set Fields!" << endl;
                return;
            }
            e_opt_tab->writeFile();
            break;
        }
        case OPT_CREATE_VIEW:
            break;
        case OPT_DROP_TABLE:
        {
            Table * db = SYSTEM_DB->getTable("db");
            db->readFile();
            DataSet::iterator it = db->getRowIte();
            int loc = db->getFieldLoc("db_name");
            string dbname = e_opt_db->getName();
            while (! db->IteEnd(it))
            {
                if (strcmp((*it).at(loc), dbname.c_str()) == 0)
                {
                    loc = db->getFieldLoc("table_count");
                    int c = atoi((*it).at(loc));
                    sprintf((*it).at(loc), "%d", c - 1);
                    break;
                }
                ++it;
            }
            Table * tab = SYSTEM_DB->getTable("table");
            tab->readFile();
            it = tab->getRowIte();
            while(! tab->IteEnd(it))
            {
                int loc = tab->getFieldLoc("table_name");
                if (strcmp((*it).at(loc), e_opt_tab->getName().c_str()) == 0)
                {
                    tab->removeRow(it);
                    break;
                }
                ++it;
            }
            e_opt_db->removeTable(e_opt_tab->getName());
            break;
        }
        case OPT_DROP_DATABASE:
        {
            Table * db = SYSTEM_DB->getTable("db");
            db->readFile();
            DataSet::iterator it = db->getRowIte();
            int loc = db->getFieldLoc("db_name"), id;
            while (! db->IteEnd(it))
            {
                if (strcmp((*it).at(loc), something.c_str()) == 0)
                {
                    id = atoi((*it).at(db->getFieldLoc("table_count")));
                    db->removeRow(it);
                    break;
                }
                ++it;
            }
            Table * tab = SYSTEM_DB->getTable("table");
            tab->readFile();
            it = tab->getRowIte();
            loc = tab->getFieldLoc("db_id");
            int dbid;
            while(! tab->IteEnd(it))
            {
                dbid = atoi((*it).at(loc));
                if (id == dbid)
                {
                    DataSet::iterator tmp = it;
                    tab->removeRow(tmp);
                }
                ++it;
            }
            deleteDB(something);
            break;
        }
        case OPT_INSERT:
        {
            /// insert
            e_opt_tab->pushRow(insert_value);
            e_opt_tab->writeFile();
            break;
        }
        case OPT_SELECT:
            ;
            break;
        default:
            cout << "error" << endl;
            break;
        }
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
            list<Database>::iterator beg, end = this->databases.end();
            for (beg = this->databases.begin(); beg != end; ++beg)
            {
                if ((*beg).getName() == dbname)
                    this->databases.erase(beg);
            }
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
        arow.push_back("hisql");
        arow.push_back("3");
        tmptab->pushRow(arow);
        tmptab->writeFile();
    }
    else
    {
        tmptab->readFile();
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
        tmptab->writeFile();
    }
    else
    {
        tmptab->readFile();
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
        tmptab->writeFile();
    }
    else
    {
        tmptab->readFile();
    }
}

int Commander::check_type(string &str, Column &s)
{
    if (str == "NULL")
    {
        if (strlen(s.default_val) > 0)
        {
            str = string(s.default_val);
        }
        else if (strcmp(s.null, "N")) return 0;
        return 1;
    }
    switch(s.type)
    {
    case INT:
    {
        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] < '1' || str[i] > '9')
            {
                if (i == 0 && !s.u_sign && str[i] == '-') continue;
                return -1;
            }
        }
    }
    case CHAR:
    {
        ;
    }
    case FLOAT:
    {
        if (str.find('.') != string::npos)
            if (str.find('.', str.find('.')) != string::npos)
                return -1;
        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] == '.') continue;
            if (str[i] < '1' || str[i] > '9')
            {
                if (i == 0 && !s.u_sign && str[i] == '-') continue;
                return -1;
            }
        }
    }
    default:
        return -999;
        break;
    }
    if (s.length > (int)str.length())
    {
        return -2;
    }
    return 1;
}
