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
    //dtor
}

string Database::getName()
{
    return name;
}

Table* Database::getTable(string tabname)
{
    list<Table>::iterator beg, end = m_tables.end();

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
        ;
    }
    cout << "Table '" << tabname << "' is exists!" << endl;
    return NULL;
}

void Database::renameTable(string tabname, string newname)
{

}

void Database::removeTable(string tabname)
{

}

void Database::printTables()
{

}
