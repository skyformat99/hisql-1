#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include "Table.h"

class Database
{
public:
    string getName();
    Database();

    Database(string dbname);

    virtual ~Database();

    Table* getTable(string tabname);
    Table* addTable(string tabname);
    void renameTable(string tabname, string newname);
    void removeTable(string tabname);
    void printTables();
private:
    string name;
    list<Table> m_tables;
};

#endif // DATABASE_H
