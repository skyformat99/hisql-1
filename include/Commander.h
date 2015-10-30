#ifndef COMMANDER_H
#define COMMANDER_H

#include "Table.h"
#include "Database.h"

class Commander
{
public:
    string user;

    enum Operation
    {
        OPT_NONE,
        OPT_CREATE_TABLE, OPT_DROP_TABLE,
        OPT_ALTER_TABLE_ADD, OPT_ALTER_TABLE_DROP_COLUMN,
        OPT_INSERT, OPT_DELETE, OPT_UPDATE, OPT_SELECT,
        OPT_CREATE_INDEX, OPT_DROP_INDEX,
        OPT_CREATE_VIEW, OPT_DROP_VIEW,
        OPT_CREATE_USER, OPT_GRANT, OPT_REVOKE,
        OPT_SHOW_DATABASES, OPT_SHOW_TABLES, OPT_SHOW_COLUMNS
    };

    /** CURRENT DATABAES Pointer */
    Database * CURRENT_DB;

    /** CURRENT DATABAES Pointer */
    Database * SYSTEM_DB;

    /** BUFF SIZE CONST STATIC */
    static int const BUFFSIZE = 4096;

    /** To get the instance in singleton */
    static Commander* getCommander()
    {
        static Commander cmd;
        return &cmd;
    }

    /** Default destructor */
    virtual ~Commander();

    /** load a database */
    Database* loadDB(string dbname);

    /** Command parse and execute */
    /** a reader can read multi line */
    void reader();

    /** SQL parser and executor */
    void exec();

    /** NEXT: db operation */
    /** create a database */
    int createDB(string dbname);

    /** change the current database */
    int changeDB(string dbname);

    /** delete a database */
    int deleteDB(string dbname);
private:
    /** Default constructor */
    Commander();
    Commander(const Commander&);
    Commander& operator = (const Commander&);

    /** System load and new */
    void cp_loadSystemDB();

    /** System function */
    void cp_nowStatus();

    /** command line */
    char m_cmd_line[BUFFSIZE];

    /** database list */
    list<Database> databases;
};

#endif // COMMANDER_H
