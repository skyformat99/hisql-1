#include <stdio.h>

#include "Database.h"

Database::Database()
{
    name = "";
}

Database::Database(string dbname)
{
    name = dbname;
}

Database::~Database()
{
    this->m_tables.clear();
}

string Database::getName()
{
    return name;
}

Table* Database::getTable(string tabname)
{
    TableList::iterator beg, end = m_tables.end();

    for (beg = m_tables.begin(); beg != end; ++beg)
    {
        if ((*beg).getName() == tabname)
        {
            return &(*beg);
        }
    }

    if (g_file_dir(DBFPATH + name + "\\" + tabname + ".tbl", 0))
    {
        Table tab(tabname, name);
        if (tab.readFile() < 0)
        {
            g_plog(9999, S_ERROR, "88500", name + "."+ tabname, 0);
            return NULL;
        }
        this->m_tables.push_back(tab);
        return &this->m_tables.back();
    }

    return NULL;
}

Table* Database::addTable(string tabname)
{
    Table *tab = getTable(tabname);
    if (tab == NULL)
    {
        string path = DBFPATH + name + "\\" + tabname + ".tbl";
        if (g_file_dir(path, 0))
        {
            if (remove(path.c_str()) == 0)
            {
                return NULL;
            }
        }
        /// just new a table
        Table atab(tabname, name);
        this->m_tables.push_back(atab);
        return &this->m_tables.back();
    }
    return NULL;
}

int Database::renameTable(string tabname, string newname)
{
    Table * tab = getTable(tabname);

    if (tab != NULL)
    {
        string path = DBFPATH + name + "\\" + tabname + ".tbl";
        if (remove(path.c_str()) == 0)
        {
            tab->setName(newname);
            return 0;
        }
    }
    return -1;
}

int Database::removeTable(string tabname)
{
    /// remove it in list if has
    TableList::iterator beg = this->m_tables.begin(), end = this->m_tables.end();
    while (beg != end)
    {
        if ((*beg).getName() == tabname)
        {
            this->m_tables.erase(beg);
            break;
        }
        ++beg;
    }

    /// delete file
    string path = DBFPATH + name + "\\" + tabname + ".tbl";
    if (g_file_dir(path.c_str(), 0))
        if (remove(path.c_str()) != 0)
            return -1;
    return 0;
}

void Database::printTables()
{
    vector<string> tmp;
    tmp.push_back("Table_in_" + name);
    TableList::iterator beg, end = this->m_tables.end();
    for (beg = this->m_tables.begin(); beg!= end; ++beg)
    {
        tmp.push_back((*beg).getName());
    }
    g_print_table(tmp, 1);
}

int Database::openTable()
{
    return this->m_tables.size();
}
