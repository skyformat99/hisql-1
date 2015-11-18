#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include "Table.h"

typedef list<Table> TableList;

class Database
{
public:
    string getName();
    Database();

    Database(string dbname);

    virtual ~Database();

    Table* getTable(string tabname);
    Table* addTable(string tabname);
    int renameTable(string tabname, string newname);
    int removeTable(string tabname);
    void printTables();
    int openTable();

    TableList m_tables;
private:
    string name;
};

#endif // DATABASE_H
