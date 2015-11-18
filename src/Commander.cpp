#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
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
                else
                {
                    /** just sending the buff to commander */
                    strcpy(this->m_cmd_line, buff);
                    this->exec();
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
    clock_t start = clock();
    string str = "";
    char *p = this->m_cmd_line;
    p = g_getword(p, str, true);
    if (str == "login")
    {
        OPT_TABLE = SYSTEM_DB->getTable("user");
        p = g_getword(p, str, false);
        bool found = false;
        int loc = OPT_TABLE->getFieldLoc("user");
        DataSet::iterator it = OPT_TABLE->getRowIte();
        while (!OPT_TABLE->IteEnd(it))
        {
            if (strcmp((*it).at(loc), str.c_str()) == 0)
            {
                string tuser = str;
                found = true;
                if (*p == '\0')
                {
                    cout << "Input Password: ";
                    getline(cin, str);
                }
                else
                    p = g_getword(p, str, false);
                loc = OPT_TABLE->getFieldLoc("pass");
                if (strcmp((*it).at(loc), str.c_str()) == 0)
                {
                    user = tuser;
                    cout << "Success to login!" << endl;
                }
                else
                    cout << "Warning: Wrong Password!" << endl;
            }
            ++it;
        }
        if (!found)
        {
            cout << "Fatal: No Such User!" << endl;
        }
        return;
    }
    if (user.length() == 0)
    {
        cout << "Permission denied!" << endl;
        return;
    }
    if (str == "alter")
    {
        p = g_getword(p, str, true);
        if (str == "table")
        {
            if (DEBUG) cout << "Alter Table" << endl;
            p = g_getword(p, str, false); /// get table name
            if (this->cp_getDbTable(str)) return;
            p = g_getword(p, str, false); /// get next key word
            if (str == "add")
            {
                p = g_getword_mod(p, str, false, ' ', '\0');
                g_trim(str, ' ');
                Column acol;
                if (! g_convert_col(str, acol))
                {
                    cout << "Fatal: Can't not get col." << endl;
                    closeDB(OPT_TABLE);
                    return;
                }
                if (DEBUG)
                    cout << "Add Field '" << OPT_TABLE->getName() << "." << acol.field << "':" << endl;
                if (! OPT_TABLE->addField(acol)) /// ADD FIELD!!
                {
                    cout << "Fatal: Can't not add col in this table!" << endl;
                    closeDB(OPT_TABLE);
                    return;
                }
                this->cp_buildDict(OPT_ADD_FIELD, OPT_DB->getName() + "." + OPT_TABLE->getName(), 0);
                if (DEBUG) OPT_TABLE->showColumns();
                OPT_TABLE->writeFile(); /// write it
                g_pok(1, start);
                closeDB(OPT_TABLE);
                return;
            } /// end ADD
            else if (str == "drop")
            {
                p = g_getword(p, str, true);
                if (str == "column")
                {
                    p = g_getword(p, str, false);
                    g_trim(str, '`');
                    if (DEBUG)
                        cout << "Drop Field '" << OPT_TABLE->getName() << "." << str << "':" << endl;
                    if (DEBUG) OPT_TABLE->showColumns();
                    if (! OPT_TABLE->removeField(str))
                    {
                        cout << "Fatal: Can't drop this col." << endl;
                        closeDB(OPT_TABLE);
                        return;
                    }
                    this->cp_buildDict(OPT_DROP_FIELD, OPT_DB->getName() + "." + OPT_TABLE->getName(), 0);
                    OPT_TABLE->writeFile();
                    if (DEBUG) OPT_TABLE->showColumns();
                    g_pok(1, start);
                    closeDB(OPT_TABLE);
                    return;
                }
            } /// end DORP
        } /// end TABLE
    } /// end ALTER
    else if (str == "create")
    {
        p = g_getword(p, str, true);
        if (str == "database")
        {
            p = g_getword(p, str, false);
            g_trim(str, '`');
            OPT_DB = loadDB(str);
            if (OPT_DB == NULL)
            {
                int ret = createDB(str);
                if (ret < 0)
                {
                    cout << "Fatal: Can't Create Database!" << endl;
                    return;
                }
                this->cp_buildDict(OPT_ADD_DATABASE, str, 0);
                cout << "Database created!" << endl;
                return;
            }
            g_plog(1007, S_ERROR, "HY000", str, 0);
            return;
        } /// end DATABASE
        else if (str == "table")
        {
            p = g_getword_mod(p, str, false, ' ', '(');

            if (str.find('.') != string::npos)
            {
                string dbname = str.substr(0, str.find('.'));
                str = str.substr(str.find('.') + 1);
                g_trim(dbname, '`');
                OPT_DB = loadDB(dbname);
                if (OPT_DB == NULL)
                {
                    g_plog(1049, S_ERROR, "CT001", dbname, 0);
                    return;
                }
            }
            else
            {
                OPT_DB = this->CURRENT_DB;
                if (OPT_DB == NULL)
                {
                    g_plog(1046, S_ERROR, "CT002", "", 0);
                    return;
                }
            }
            string tabname = g_trim(str, '`');
            tabname = g_trim(tabname, ' ');
            if (*p == '(')
            {
                ++p;
                p = g_getword_mod(p, str, false, ' ', '\0');
                if (str[str.length() - 1] == ')') str = str.substr(0, str.length() - 1);
                vector<string> cols;
                ColumnList colv;
                cout << str << endl;
                g_split(str, ',', cols);
                if (cols.size() == 0) cols.push_back(str);
                for (size_t i = 0; i < cols.size(); ++i)
                {
                    Column acol;
                    g_trim(cols[i], ' ');
                    cout << cols[i] << endl;
                    if (! g_convert_col(cols[i], acol))
                    {
                        cout << "Fatal: can't set this col '" << cols[i] << "';" << endl;
                        return;
                    }
                    colv.push_back(acol);
                }
                OPT_TABLE = OPT_DB->addTable(tabname);
                if (OPT_TABLE == NULL)
                {
                    cout << "Fatal: table '" << tabname << "' is exists!" << endl;
                    return;
                }
                OPT_TABLE->setFields(colv); /// set fields
                OPT_TABLE->showColumns();
                OPT_TABLE->writeFile();
                this->cp_buildDict(OPT_ADD_TABLE, OPT_DB->getName() + "." + OPT_TABLE->getName(), colv.size());
                cout << "Table created!" << endl;
            }
            return;
        } /// end TABLE
        else if (str == "index")
        {
            ;
        } /// end INDEX
        else if (str == "view")
        {
            ;
        } /// end VIEW
    } /// end CREATE
    else if (str == "delete")
    {
        p = g_getword(p, str, true);
        if (str == "from")
        {
            p = g_getword(p, str, false);
            if (this->cp_getDbTable(str)) return;
            if (*p == '\0') /// DELETE FROM TAB : DELETE ALL
            {
                int count = OPT_TABLE->countRow();
                OPT_TABLE->clearAll();
                OPT_TABLE->writeFile();
                g_pok(count, start);
                return;
            } /// end *

            p = g_getword(p, str, true);
            if (str == "where") /// delete with condition
            {
                int count = 0; /// counting row
                p = g_getword_mod(p, str, false, ' ', '\0');
                string cmp = "", field = "", value = "";
                size_t cpos = string::npos;
                if ((cpos = str.find('>')) != string::npos
                        || (cpos = str.find('<')) != string::npos) /// if find compare operator
                {
                    field = str.substr(0, cpos);
                    cmp += str[cpos];
                    if (str.at(1 + cpos) == '=')
                    {
                        ++cpos;
                        cmp += str[cpos];
                    }
                    value = str.substr(cpos + 1);
                }
                else if ((cpos = str.find('=')) != string::npos) /// found equal
                {
                    field = str.substr(0, cpos);
                    cmp += str[cpos];
                    value = str.substr(cpos + 1);
                }
                else /// not found
                {
                    g_plog(1064, S_ERROR, "DFW01", str, 0);
                    return;
                }
                g_trim(field, ' ');
                g_trim(value, ' ');
                int loc = OPT_TABLE->getFieldLoc(field); /// get field locate
                Column * col = OPT_TABLE->getField(field); /// get field object
                if (col == NULL) /// not found
                {
                    cout << "Fatal: Field '" << field << "' not found!" << endl;
                    return;
                }
                cout << field << "<" << cmp << ">" << value << endl; /// debug
                if (col->type == CHAR && cmp != "=") /// if is a char but not equal
                {
                    cout << "Fatal: Character can not using bigger or smaller to compare." << endl;
                    return;
                }
                if (col->type == CHAR) g_shiftdot(value, '\'');
                DataSet::iterator it = OPT_TABLE->getRowIte(), tmp; /// iterator
                while (!OPT_TABLE->IteEnd(it))
                {
                    tmp = it;
                    bool del = false;
                    if (cmp == "=") /// if equal, just using strcmp
                    {
                        if (strcmp((*it).at(loc), value.c_str()) == 0) del = true;
                    }
                    else
                    {
                        if (col->type == INT)
                        {
                            int x = atol((*it).at(loc));
                            int y = atol(value.c_str());
                            if (cmp == ">=" && x >= y) del = true;
                            else if (cmp == "<=" && x <= y) del = true;
                            else if (cmp == ">" && x > y) del = true;
                            else if (cmp == "<" && x < y) del = true;
                        }
                        else if (col->type == FLOAT)
                        {
                            float x = atof((*it).at(loc));
                            float y = atof(value.c_str());
                            if (cmp == ">=" && x >= y) del = true;
                            else if (cmp == "<=" && x <= y) del = true;
                            else if (cmp == ">" && x > y) del = true;
                            else if (cmp == "<" && x < y) del = true;
                        }
                    }
                    if (del)
                    {
                        ++it;
                        OPT_TABLE->removeRow(tmp);
                        ++count;
                    }
                    else
                        ++it;
                } /// end iterator
                OPT_TABLE->writeFile();
                g_pok(count, start);
                return;
            } /// end WHERE
        } /// end FROM
    } /// end DELETE
    else if (str == "drop")
    {
        p = g_getword(p, str, true);
        if (str == "database")
        {
            p = g_getword(p, str, false);
            g_trim(str, '`');

            OPT_TABLE = SYSTEM_DB->getTable("db");

            bool found = false;
            int loc = OPT_TABLE->getFieldLoc("db_name");
            DataSet::iterator it = OPT_TABLE->getRowIte();
            while (!OPT_TABLE->IteEnd(it))
            {
                if (strcmp((*it).at(loc), str.c_str()) == 0)
                {
                    found = true;
                    loc = OPT_TABLE->getFieldLoc("table_count");
                    loc = atoi((*it).at(loc));
                    break;
                }
                ++it;
            }
            if (found)
            {
                int count = loc + 1;
                this->cp_buildDict(OPT_DROP_DATABASE, str, 0);
                deleteDB(str);
                g_pok(count, start);
                return;
            }
            g_plog(1049, S_ERROR, "DD001", str, 0);
            return;
        } /// end DATABASE
        else if (str == "table")
        {
            p = g_getword(p, str, false);
            if (this->cp_getDbTable(str)) return;
            string info = OPT_DB->getName() + "." + OPT_TABLE->getName();
            OPT_DB->removeTable(OPT_TABLE->getName());
            this->cp_buildDict(OPT_DROP_TABLE, info, 0);
            g_pok(2, start);
            return;
        } /// end TABLE
    } /// end DROP
    else if (str == "insert")
    {
        p = g_getword(p, str, true);
        if (str == "into")
        {
            p = g_getword(p, str, false); /// get table name
            if (this->cp_getDbTable(str)) return;
            p = g_getword_mod(p, str, true, ' ', '('); /// get INTO
            g_trim(str, ' ');
            if (str == "values")
            {
                p = g_getword_mod(p, str, false, ' ', '\0');
                if (DEBUG) cout << "Data: " << str << endl;
                if (str[0] == '(' && str[str.length() - 1] == ')')
                {
                    str = str.substr(1, str.length() - 2);
                    vector<string> valv;
                    g_split(str, ',', valv);
                    if (valv.size() == 0) valv.push_back(str);
                    for (size_t i = 0; i < valv.size(); ++i)
                    {
                        g_trim(valv.at(i), ' ');
                        if (DEBUG) cout << "[" << valv.at(i) << "]" << endl;
                    }
                    Row * tmp = OPT_TABLE->pushRow(valv);
                    if (tmp != NULL)
                    {
                        OPT_TABLE->writeFile();
                        g_pok(1, start);
                        return;
                    }
                    else
                    {
                        cout << "Fail!" << endl;
                        return;
                    }
                }
            } /// end VALUES
        } /// end INTO
    } /// end INSERT
    else if (str == "select")
    {
        p = g_getword(p, str, false);
        vector<string> field;
        g_split(str, ',', field);
        if (field.size() == 0) field.push_back(str);
        p = g_getword(p, str, true);
        if (str == "from")
        {
            if (DEBUG) cout << "Form";
            p = g_getword(p, str, false);
            vector<string> table, rltset;
            g_split(str, ',', table);
            if (table.size() == 0) table.push_back(str);
            if (*p == '\0')
            {
                if (DEBUG) cout << " No Where";
                if(this->cp_getDbTable(table.at(0))) return;
                cout << "Mark A" << endl;
                if (field.size() == 1 && field.at(0) == "*")
                {
                    if (table.size() == 1)
                    {
                        if (DEBUG) cout << endl;
                        OPT_TABLE->showRows();
                        g_pok(OPT_TABLE->countRow(), start);
                        return;
                    }
                    else if (table.size() == 2)
                    {
                        if (DEBUG) cout << " 2 Table" << endl;
                        int count = 0;
                        Table * tmp_tab = OPT_TABLE;
                        if (this->cp_getDbTable(table.at(1))) return;
                        /// push table a to result set
                        ColumnList::iterator cit = tmp_tab->getColIte();
                        while (!tmp_tab->colIteEnd(cit))
                        {
                            rltset.push_back(tmp_tab->getName() + "." + (*cit).field);
                            ++cit;
                        }
                        if (DEBUG) cout << "Table A has been add" << endl;
                        /// push table b to result set
                        cit = OPT_TABLE->getColIte();
                        while (!OPT_TABLE->colIteEnd(cit))
                        {
                            rltset.push_back(OPT_TABLE->getName() + "." + (*cit).field);
                            ++cit;
                        }
                        if (DEBUG) cout << "Table B has been add" << endl;
                        /// push content
                        int a = tmp_tab->countCol(), b = OPT_TABLE->countCol();
                        DataSet::iterator it = tmp_tab->getRowIte();
                        while (!tmp_tab->IteEnd(it))
                        {
                            DataSet::iterator iit = OPT_TABLE->getRowIte();
                            while (!OPT_TABLE->IteEnd(iit))
                            {
                                for (int i = 0; i < a; ++i)
                                    rltset.push_back((*it).at(i));
                                for (int i = 0; i < b; ++i)
                                    rltset.push_back((*iit).at(i));
                                ++iit;
                                ++count;
                            }
                            ++it;
                        }
                        if (DEBUG) cout << "Table content has been add" << endl;
                        g_print_table(rltset, a + b);
                        g_pok(count, start);
                        return;
                    }
                } /// end SELECT * FROM TAB
                else
                {

                    int count = 0;
                    vector<int> locs;
                    for (size_t i = 0; i < field.size(); ++i)
                    {
                        int loc = OPT_TABLE->getFieldLoc(field.at(i));
                        if (loc < 0)
                        {
                            cout << "Fatal: Can't Find Field '"
                                 << OPT_TABLE->getName() << "." << field.at(i) << "'!" << endl;
                            return;
                        }
                        locs.push_back(loc);
                        rltset.push_back(field.at(i));
                    }
                    DataSet::iterator it = OPT_TABLE->getRowIte();
                    while (!OPT_TABLE->IteEnd(it))
                    {
                        for (size_t i = 0; i < locs.size(); ++i)
                            rltset.push_back((*it).at(locs.at(i)));
                        ++it;
                        ++count;
                    }
                    g_print_table(rltset, locs.size());
                    g_pok(count, start);
                    return;
                }
            } /// no WHERE
            else
            {
                if (table.size() == 1)
                {
                    if(this->cp_getDbTable(table.at(0))) return;
                }
                else
                {
                    cout << "Just One Table!" << endl;
                    return;
                }
                /// making the field
                vector<int> locs;
                if (field.size() == 1 && field.at(0) == "*")
                {
                    int i = 0;
                    ColumnList::iterator it = OPT_TABLE->getColIte();
                    while (! OPT_TABLE->colIteEnd(it))
                    {
                        rltset.push_back((*it).field);
                        locs.push_back(i);
                        ++it;
                        ++i;
                    }
                }
                else
                {
                    for (size_t i = 0; i < field.size(); ++i)
                    {
                        int loc = OPT_TABLE->getFieldLoc(field.at(i));
                        if (loc < 0)
                        {
                            cout << "Fatal: Can't Find Field '"
                                 << OPT_TABLE->getName() << "." << field.at(i) << "'!" << endl;
                            return;
                        }
                        locs.push_back(loc);
                        rltset.push_back(field.at(i));
                    }
                }
                /// get condition
                p = g_getword(p, str, true);
                if (str == "where")
                {
                    int count = 0; /// counting row
                    p = g_getword_mod(p, str, false, ' ', '\0');
                    string cmp = "", field = "", value = "";
                    size_t cpos = string::npos;
                    if ((cpos = str.find('>')) != string::npos
                            || (cpos = str.find('<')) != string::npos) /// if find compare operator
                    {
                        field = str.substr(0, cpos);
                        cmp += str[cpos];
                        if (str.at(1 + cpos) == '=')
                        {
                            ++cpos;
                            cmp += str[cpos];
                        }
                        value = str.substr(cpos + 1);
                    }
                    else if ((cpos = str.find('=')) != string::npos) /// found equal
                    {
                        field = str.substr(0, cpos);
                        cmp += str[cpos];
                        value = str.substr(cpos + 1);
                    }
                    else /// not found
                    {
                        g_plog(1064, S_ERROR, "DFW01", str, 0);
                        return;
                    }
                    g_trim(field, ' ');
                    g_trim(value, ' ');
                    int loc = OPT_TABLE->getFieldLoc(field); /// get field locate
                    Column * col = OPT_TABLE->getField(field); /// get field object
                    if (col == NULL) /// not found
                    {
                        cout << "Fatal: Field '" << field << "' not found!" << endl;
                        return;
                    }
                    cout << field << "<" << cmp << ">" << value << endl; /// debug
                    if (col->type == CHAR && cmp != "=") /// if is a char but not equal
                    {
                        cout << "Fatal: Character can not using bigger or smaller to compare." << endl;
                        return;
                    }
                    DataSet::iterator it = OPT_TABLE->getRowIte(), tmp; /// iterator
                    while (!OPT_TABLE->IteEnd(it))
                    {
                        tmp = it;
                        bool add = false;
                        if (cmp == "=") /// if equal, just using strcmp
                        {
                            if (strcmp((*it).at(loc), value.c_str()) == 0) add = true;
                        }
                        else
                        {
                            if (col->type == INT)
                            {
                                int x = atol((*it).at(loc));
                                int y = atol(value.c_str());
                                if (cmp == ">=" && x >= y) add = true;
                                else if (cmp == "<=" && x <= y) add = true;
                                else if (cmp == ">" && x > y) add = true;
                                else if (cmp == "<" && x < y) add = true;
                            }
                            else if (col->type == FLOAT)
                            {
                                float x = atof((*it).at(loc));
                                float y = atof(value.c_str());
                                if (cmp == ">=" && x >= y) add = true;
                                else if (cmp == "<=" && x <= y) add = true;
                                else if (cmp == ">" && x > y) add = true;
                                else if (cmp == "<" && x < y) add = true;
                            }
                        }
                        if (add)
                        {
                            for (size_t i = 0; i < locs.size(); ++i)
                                rltset.push_back((*it).at(i));
                            ++count;
                        }
                        ++it;
                    } /// end iterator
                    g_print_table(rltset, locs.size());
                    g_pok(count, start);
                    return;
                } /// end WHERE
            } /// has WHERE
        } /// end FROM
    } /// end SELECT
    else if (str == "show")
    {
        p = g_getword(p, str, true);
        if (str == "databases")
        {
            int count = 0;
            vector<string> tmp;
            tmp.push_back("Database");
            OPT_TABLE = SYSTEM_DB->getTable("db");
            int loc = OPT_TABLE->getFieldLoc("db_name");
            DataSet::iterator it = OPT_TABLE->getRowIte();
            while (!OPT_TABLE->IteEnd(it))
            {
                tmp.push_back((*it).at(loc));
                ++it;
                ++count;
            }
            g_print_table(tmp, 1);
            g_pok(count, start);
            return;
        } /// end DATABASES
        else if (str == "tables")
        {
            p = g_getword(p, str, true);
            if (str == "from")
            {
                int count = 0;
                p = g_getword(p, str, false);
                g_trim(str, ' ');
                OPT_DB = loadDB(str);
                if (OPT_DB != NULL)
                {
                    vector<string> tmp;
                    tmp.push_back("Table_in_" + str);
                    OPT_TABLE = SYSTEM_DB->getTable("db");
                    int loc = OPT_TABLE->getFieldLoc("db_name"), id = OPT_TABLE->getFieldLoc("id");
                    DataSet::iterator it = OPT_TABLE->getRowIte();
                    while (!OPT_TABLE->IteEnd(it))
                    {
                        if (strcmp((*it).at(loc), str.c_str()) == 0)
                        {
                            id = atoi((*it).at(id));
                            break;
                        }
                        ++it;
                    }
                    OPT_TABLE = SYSTEM_DB->getTable("table");
                    loc = OPT_TABLE->getFieldLoc("db_id");
                    int nloc = OPT_TABLE->getFieldLoc("table_name");
                    it = OPT_TABLE->getRowIte();
                    while (!OPT_TABLE->IteEnd(it))
                    {
                        if (atoi((*it).at(loc)) == id)
                        {
                            tmp.push_back((*it).at(nloc));
                            ++count;
                        }
                        ++it;
                    }
                    g_print_table(tmp, 1);
                    g_pok(count, start);
                    return;
                }
                g_plog(1049, S_ERROR, "STF001", str, 0);
                return;
            }
        } /// end TABLES
        else if (str == "columns")
        {
            p = g_getword(p, str, true);
            if (str == "from")
            {
                p = g_getword(p, str, false);
                g_trim(str, ' ');
                if (this->cp_getDbTable(str)) return;
                OPT_TABLE->showColumns();
                return;
            }
        } /// end COLUMNS
    } /// end SHOW
    else if (str == "update")
    {
        if (DEBUG) cout << "Update";
        p = g_getword(p, str, false);
        g_trim(str, ' ');
        if (this->cp_getDbTable(str)) return;
        p = g_getword(p, str, true);
        if (str == "set")
        {
            if (DEBUG) cout << " Set";
            p = g_getword(p, str, false);
            g_trim(str, ' ');
            vector<string> v;
            g_split(str, '=', v);
            p = g_getword(p, str, true);
            if (v.size() == 2 && str == "where") /// delete with condition
            {
                if (DEBUG) cout << " Where" << endl;
                Column * mcol = OPT_TABLE->getField(v.at(0));
                int modloc = OPT_TABLE->getFieldLoc(v.at(0));
                if (mcol == NULL)
                {
                    cout << "Fatal: Not Found Field '"
                         << v.at(0) << "'!" << endl;
                    return;
                }
                if (! OPT_TABLE->dataCheck(v.at(1), *mcol)) return;
                int count = 0; /// counting row
                p = g_getword_mod(p, str, false, ' ', '\0');
                string cmp = "", field = "", value = "";
                size_t cpos = string::npos;
                if ((cpos = str.find('>')) != string::npos
                        || (cpos = str.find('<')) != string::npos) /// if find compare operator
                {
                    field = str.substr(0, cpos);
                    cmp += str[cpos];
                    if (str.at(1 + cpos) == '=')
                    {
                        ++cpos;
                        cmp += str[cpos];
                    }
                    value = str.substr(cpos + 1);
                }
                else if ((cpos = str.find('=')) != string::npos) /// found equal
                {
                    field = str.substr(0, cpos);
                    cmp += str[cpos];
                    value = str.substr(cpos + 1);
                }
                else /// not found
                {
                    g_plog(1064, S_ERROR, "DFW01", str, 0);
                    return;
                }
                g_trim(field, ' ');
                g_trim(value, ' ');
                int loc = OPT_TABLE->getFieldLoc(field); /// get field locate
                Column * col = OPT_TABLE->getField(field); /// get field object
                if (col == NULL) /// not found
                {
                    cout << "Fatal: Field '" << field << "' not found!" << endl;
                    return;
                }
                cout << field << "<" << cmp << ">" << value << endl; /// debug
                if (col->type == CHAR && cmp != "=") /// if is a char but not equal
                {
                    cout << "Fatal: Character can not using bigger or smaller to compare." << endl;
                    return;
                }
                DataSet::iterator it = OPT_TABLE->getRowIte(); /// iterator
                while (!OPT_TABLE->IteEnd(it))
                {
                    bool mod = false;
                    if (cmp == "=") /// if equal, just using strcmp
                    {
                        if (strcmp((*it).at(loc), value.c_str()) == 0) mod = true;
                    }
                    else
                    {
                        if (col->type == INT)
                        {
                            int x = atol((*it).at(loc));
                            int y = atol(value.c_str());
                            if (cmp == ">=" && x >= y) mod = true;
                            else if (cmp == "<=" && x <= y) mod = true;
                            else if (cmp == ">" && x > y) mod = true;
                            else if (cmp == "<" && x < y) mod = true;
                        }
                        else if (col->type == FLOAT)
                        {
                            float x = atof((*it).at(loc));
                            float y = atof(value.c_str());
                            if(DEBUG) cout << x << cmp << y << endl;
                            if (cmp == ">=" && x >= y) mod = true;
                            else if (cmp == "<=" && x <= y) mod = true;
                            else if (cmp == ">" && x > y) mod = true;
                            else if (cmp == "<" && x < y) mod = true;
                        }
                    }
                    if (mod)
                    {
                        strcpy((*it).at(modloc), v.at(1).c_str());
                        cout << " modified! " << endl;
                        ++count;
                    }
                    ++it;
                } /// end iterator
                OPT_TABLE->writeFile();
                g_pok(count, start);
                return;
            } /// end WHERE
        } /// end FROM
    } /// end UPDATE
    g_plog(1064, S_ERROR, "00000", str, 0);
    return;
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
    return -1;
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
        list<Database>::iterator beg, end = this->databases.end();
        for (beg = this->databases.begin(); beg != end; ++beg)
        {
            if ((*beg).getName() == dbname)
            {
                this->databases.erase(beg);
                break;
            }
        }
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
                    if (strcmp(file.name, ".") != 0
                            && strcmp(file.name, "..") != 0)
                        remove(file.name);
                    k = _findnext(HANDLE, &file);
                }
                _findclose(HANDLE);
                _chdir(DBFPATH);
                ret = _rmdir(dbpath.c_str());
                if (ret < 0)
                {
                    perror(dbpath.c_str());
                    return 0;
                }
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
    int count = 0;
    list<Database>::iterator beg, end = this->databases.end();
    for (beg = this->databases.begin(); beg != end; ++beg)
        count += (*beg).openTable();
    if (CURRENT_DB != NULL) db = CURRENT_DB->getName();
    cout << "--------------" <<
         endl << "HiSQL Ver " << VERSION << ", for Windows x86" << endl <<
         endl << "Current database:       " << db <<
         endl << "Current user:           " << user << "@localhost" <<
         endl << "Using delimiter:        empty" <<
         endl << "Server version:         " << VERSION << endl <<
         endl << "Opens: " << this->databases.size() << "    Open tables: " << count <<
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
        arow.push_back("'hisql'");
        arow.push_back("5");
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
        arow.push_back("'root'");
        arow.push_back("'666888'");
        arow.push_back("'YYYYYYYYY'");
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
        arow.push_back("'db'");
        arow.push_back("3");
        tmptab->pushRow(arow);
        /// add user table
        arow.clear();
        arow.push_back("1");
        arow.push_back("0");
        arow.push_back("'user'");
        arow.push_back("4");
        tmptab->pushRow(arow);
        /// add table table
        arow.clear();
        arow.push_back("2");
        arow.push_back("0");
        arow.push_back("'table'");
        arow.push_back("4");
        tmptab->pushRow(arow);
        /// add index table
        arow.clear();
        arow.push_back("3");
        arow.push_back("0");
        arow.push_back("'index'");
        arow.push_back("4");
        tmptab->pushRow(arow);
        /// add index table
        arow.clear();
        arow.push_back("4");
        arow.push_back("0");
        arow.push_back("'view'");
        arow.push_back("3");
        tmptab->pushRow(arow);
        tmptab->writeFile();
    }
    else
    {
        tmptab->readFile();
    }
    /// test HISQL index Table
    tmptab = SYSTEM_DB->getTable("index");
    if (tmptab == NULL)
    {
        tmptab = SYSTEM_DB->addTable("index");
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
        g_convert_col("table_id int(10) not null", col);
        tmptab->addField(col);
        /// set table_name
        g_convert_col("index_name char(200) not null", col);
        tmptab->addField(col);
        /// set column_count
        g_convert_col("column_for char(100) not null", col);
        tmptab->addField(col);
        tmptab->writeFile();
    }
    else
    {
        tmptab->readFile();
    }
    /// test HISQL view Table
    tmptab = SYSTEM_DB->getTable("view");
    if (tmptab == NULL)
    {
        tmptab = SYSTEM_DB->addTable("view");
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
        g_convert_col("view_name char(200) not null", col);
        tmptab->addField(col);
        tmptab->writeFile();
    }
    else
    {
        tmptab->readFile();
    }
}

int Commander::cp_getDbTable(string &str)
{
    if (str.find('.') != string::npos) /// have db name, we must load it
    {
        string dbname = str.substr(0, str.find('.'));
        str = str.substr(str.find('.') + 1);
        g_trim(dbname, '`');
        OPT_DB = loadDB(dbname);

        if (OPT_DB == NULL)
        {
            g_plog(1049, S_ERROR, "123456", dbname, 0);
            return 1;
        }
    }
    else
    {
        OPT_DB = this->CURRENT_DB;
        if (OPT_DB == NULL)
        {
            g_plog(1046, S_ERROR, "654321", "", 0);
            return 1;
        }
    }
    g_trim(str, '`');
    OPT_TABLE = OPT_DB->getTable(str);
    if (OPT_TABLE == NULL)
    {
        g_plog(1051, S_ERROR, "789000", OPT_DB->getName() + "." + str, 0);
        return 1;
    }

    return 0;
}

int Commander::cp_buildDict(DictOpt opt, string info, int count)
{
    Table * dt_table = NULL;
    DataSet::iterator it;
    switch(opt)
    {
    case OPT_ADD_DATABASE:
    {
        dt_table = SYSTEM_DB->getTable("db");
        int loc = dt_table->getFieldLoc("id");
        Row * last = dt_table->getLastRow();
        char buff[10];
        vector<string> arow;
        sprintf(buff, "%d", atoi(last->at(loc)) + 1);
        arow.push_back(buff);
        arow.push_back("'" + info + "'");
        arow.push_back("0");
        dt_table->pushRow(arow);
        dt_table->writeFile();
        if (DEBUG) dt_table->showRows(); /// debug
        return 0;
        break;
    }
    case OPT_DROP_DATABASE:
    {
        dt_table = SYSTEM_DB->getTable("db");
        Table * st_table = SYSTEM_DB->getTable("table");

        int loc = dt_table->getFieldLoc("id"), id;
        int nameloc = dt_table->getFieldLoc("db_name");

        it = dt_table->getRowIte();
        while (! dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(nameloc), info.c_str()) == 0)
            {
                id = atoi((*it).at(loc));
                dt_table->removeRow(it);
                break;
            }
            ++it;
        }
        dt_table->writeFile();
        if (DEBUG) dt_table->showRows(); /// debug

        loc = st_table->getFieldLoc("db_id");

        it = st_table->getRowIte();
        while (! st_table->IteEnd(it))
        {
            if (atoi((*it).at(loc)) == id)
            {
                DataSet::iterator tmp = it;
                ++it;
                st_table->removeRow(tmp);
            }
            else
                ++it;
        }
        st_table->writeFile();
        if (DEBUG) st_table->showRows(); /// debug
        return 0;
        break;
    }
    case OPT_ADD_TABLE:
    {
        string tabname = info.substr(info.find('.') + 1);
        info = info.substr(0, info.find('.')); /// db name

        dt_table = SYSTEM_DB->getTable("table");
        Table * st_db = SYSTEM_DB->getTable("db");

        int loc = st_db->getFieldLoc("db_name"), dbid = st_db->getFieldLoc("id");
        it = st_db->getRowIte();
        while (! st_db->IteEnd(it))
        {
            if (strcmp((*it).at(loc), info.c_str()) == 0) /// found db name
            {
                dbid = atoi((*it).at(dbid));
                loc = st_db->getFieldLoc("table_count");
                //char buff[10];
                sprintf((*it).at(loc), "%d", atoi((*it).at(loc)) + 1);
                //strcpy((*it).at(loc), buff);
                break;
            }
            ++it;
        }
        st_db->writeFile();

        vector<string> tmp;
        char idstr[10];
        sprintf(idstr, "%d", atoi(dt_table->getLastRow()->at(dt_table->getFieldLoc("id"))) + 1);
        tmp.push_back(idstr);
        sprintf(idstr, "%d", dbid);
        tmp.push_back(idstr);
        tmp.push_back("'" + tabname + "'");
        sprintf(idstr, "%d", count);
        tmp.push_back(idstr);
        dt_table->pushRow(tmp);
        dt_table->writeFile();
        if (DEBUG) dt_table->showRows(); /// debug
        return 0;
        break;
    }
    case OPT_DROP_TABLE:
    {
        string tabname = info.substr(info.find('.') + 1);
        info = info.substr(0, info.find('.')); /// db name

        dt_table = SYSTEM_DB->getTable("db");
        int loc = dt_table->getFieldLoc("db_name"), id = dt_table->getFieldLoc("id");
        it = dt_table->getRowIte();
        while (!dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), info.c_str()) == 0) /// found
            {
                loc = dt_table->getFieldLoc("table_count");
                id = atoi((*it).at(id));
                sprintf((*it).at(loc), "%d", atoi((*it).at(loc)) - 1);
                break;
            }
            ++it;
        }
        dt_table->writeFile();

        Table * st_table = SYSTEM_DB->getTable("table");
        loc = st_table->getFieldLoc("table_name");
        int dbid = st_table->getFieldLoc("db_id");
        it = st_table->getRowIte();
        while (!st_table->IteEnd(it))
        {
            if (atoi((*it).at(dbid)) == id
                    && strcmp((*it).at(loc), tabname.c_str()) == 0)
            {
                st_table->removeRow(it);
                break;
            }
            ++it;
        }
        st_table->writeFile();
        if (DEBUG) dt_table->showRows(); /// debug
        if (DEBUG) st_table->showRows(); /// debug
        return 0;
        break;
    }
    case OPT_ADD_FIELD:
    {
        string tabname = info.substr(info.find('.') + 1);
        info = info.substr(0, info.find('.')); /// db name

        dt_table = SYSTEM_DB->getTable("db");
        int loc = dt_table->getFieldLoc("db_name"), id = dt_table->getFieldLoc("id");
        it = dt_table->getRowIte();
        while (!dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), info.c_str()) == 0) /// found
            {
                id = atoi((*it).at(id));
                break;
            }
            ++it;
        }

        Table * st_table = SYSTEM_DB->getTable("table");
        loc = st_table->getFieldLoc("table_name");
        int dbid = st_table->getFieldLoc("db_id");
        it = st_table->getRowIte();
        while (!st_table->IteEnd(it))
        {
            if (atoi((*it).at(dbid)) == id
                    && strcmp((*it).at(loc), tabname.c_str()) == 0)
            {
                loc = st_table->getFieldLoc("column_count");
                sprintf((*it).at(loc), "%d", atoi((*it).at(loc)) + 1);
                break;
            }
            ++it;
        }
        st_table->writeFile();
        if (DEBUG) st_table->showRows(); /// debug
        return 0;
        break;
    }
    case OPT_DROP_FIELD:
    {
        string tabname = info.substr(info.find('.') + 1);
        info = info.substr(0, info.find('.')); /// db name

        dt_table = SYSTEM_DB->getTable("db");
        int loc = dt_table->getFieldLoc("db_name"), id = dt_table->getFieldLoc("id");
        it = dt_table->getRowIte();
        while (!dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), info.c_str()) == 0) /// found
            {
                id = atoi((*it).at(id));
                break;
            }
            ++it;
        }

        Table * st_table = SYSTEM_DB->getTable("table");
        loc = st_table->getFieldLoc("table_name");
        int dbid = st_table->getFieldLoc("db_id");
        it = st_table->getRowIte();
        while (!st_table->IteEnd(it))
        {
            if (atoi((*it).at(dbid)) == id
                    && strcmp((*it).at(loc), tabname.c_str()) == 0)
            {
                loc = st_table->getFieldLoc("column_count");
                sprintf((*it).at(loc), "%d", atoi((*it).at(loc)) - 1);
                break;
            }
            ++it;
        }
        st_table->writeFile();
        if (DEBUG) dt_table->showRows(); /// debug
        return 0;
        break;
    }
    case OPT_ADD_INDEX:
    {
        string indexname = info.substr(0, info.find(':')); /// index name
        info = info.substr(info.find(':') + 1);
        string dbname = info.substr(0, info.find('.')); /// db name
        info = info.substr(info.find('.') + 1);
        string tabname = info.substr(0, info.find('.')); /// table name
        info = info.substr(info.find('.') + 1);
        /// find db id
        dt_table = SYSTEM_DB->getTable("db");
        int loc = dt_table->getFieldLoc("db_name"), id = dt_table->getFieldLoc("id");
        DataSet::iterator it = dt_table->getRowIte();
        while (! dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), dbname.c_str()) == 0)
            {
                id = atoi((*it).at(id));
                break;
            }
            ++it;
        }
        /// find table id
        dt_table = SYSTEM_DB->getTable("table");
        loc = dt_table->getFieldLoc("table_name");
        it = dt_table->getRowIte();
        while (!dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), tabname.c_str()) == 0
                    && id == atoi((*it).at(dt_table->getFieldLoc("db_id"))))
            {
                id = atoi((*it).at(dt_table->getFieldLoc("id")));
                break;
            }
            ++it;
        }
        /// insert
        dt_table = SYSTEM_DB->getTable("index");
        if (dt_table->countRow() > 0)
        {
            Row * lastrow = dt_table->getLastRow();
            loc = atoi(lastrow->at(dt_table->getFieldLoc("id")));
        }
        else
            loc = 0;
        vector<string> arow;
        char buff[10];
        sprintf(buff, "%d", loc);
        arow.push_back(buff);
        sprintf(buff, "%d", id);
        arow.push_back(buff);
        arow.push_back("'" + indexname + "'");
        arow.push_back("'" + info + "'");
        dt_table->pushRow(arow);
        dt_table->writeFile();
        if (DEBUG) dt_table->showRows();
        return 0;
        break;
    }
    case OPT_DROP_INDEX:
    {
        dt_table = SYSTEM_DB->getTable("index");
        DataSet::iterator it = dt_table->getRowIte();
        int loc = dt_table->getFieldLoc("index_name");
        while (! dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), info.c_str()) == 0)
            {
                dt_table->removeRow(it);
                break;
            }
            ++it;
        }
        dt_table->writeFile();
        if (DEBUG) dt_table->showRows();
        return 0;
        break;
    }
    case OPT_ADD_VIEW:
    {
        string tabname = info.substr(info.find('.') + 1);
        info = info.substr(0, info.find('.')); /// db name
        dt_table = SYSTEM_DB->getTable("db");
        int loc = dt_table->getFieldLoc("db_name"), id = dt_table->getFieldLoc("id");
        DataSet::iterator it = dt_table->getRowIte();
        while (! dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), info.c_str()) == 0)
            {
                id = atoi((*it).at(id));
                break;
            }
        }
        dt_table = SYSTEM_DB->getTable("view");
        if (dt_table->countRow() > 0)
            loc = atoi(dt_table->getLastRow()->at(dt_table->getFieldLoc("id")));
        else
            loc = 0;
        char buff[10];
        vector<string> arow;
        sprintf(buff, "%d", loc);
        arow.push_back(buff);
        sprintf(buff, "%d", id);
        arow.push_back(buff);
        arow.push_back("'" + tabname + "'");
        dt_table->pushRow(arow);
        dt_table->writeFile();
        if (DEBUG) dt_table->showRows();
        return 0;
        break;
    }
    case OPT_DROP_VIEW:
    {
        dt_table = SYSTEM_DB->getTable("view");
        int loc = dt_table->getFieldLoc("view_name");
        DataSet::iterator it = dt_table->getRowIte();
        while (!dt_table->IteEnd(it))
        {
            if (strcmp((*it).at(loc), info.c_str()) == 0)
            {
                dt_table->removeRow(it);
                break;
            }
            ++it;
        }
        dt_table->writeFile();
        if (DEBUG) dt_table->showRows();
        return 0;
        break;
    }
    default:
        return 1;
        break;
    }
    return 1;
}

int Commander::closeDB(Table * tab)
{
    if (tab == NULL) return 0;
    if (tab->m_dbname == "hisql") return 0;
    Database * db = loadDB(tab->m_dbname);
    if (db == NULL) return 0;
    TableList::iterator it;
    for (it = db->m_tables.begin(); it != db->m_tables.end(); ++it)
    {
        if ((*it).getName() == tab->getName())
        {
            db->m_tables.erase(it);
            break;
        }
    }
    tab->~Table();
    if (db->m_tables.size() == 0)
    {
        list<Database>::iterator ldit;
        for (ldit = this->databases.begin(); ldit != this->databases.end(); ++ldit)
        {
            if ((*ldit).getName() == db->getName())
            {
                this->databases.erase(ldit);
                break;
            }
        }
    }
    return 0;
}
