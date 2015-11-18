#ifndef COMMANDER_H
#define COMMANDER_H

#include "Table.h"
#include "Database.h"

class Commander
{
public:
    string user;

    enum DictOpt
    {
        OPT_NONE,
        OPT_ADD_FIELD,    OPT_DROP_FIELD,
        OPT_ADD_TABLE,    OPT_DROP_TABLE,
        OPT_ADD_DATABASE, OPT_DROP_DATABASE,
        OPT_ADD_INDEX,    OPT_DROP_INDEX,
        OPT_ADD_VIEW,     OPT_DROP_VIEW
    };

    /** */
    vector<Row *> TMP_ROW;

    /** OPERATION DATABAES Pointer */
    Database * OPT_DB;

    /** OPERATION TABLE Pointer */
    Table * OPT_TABLE;

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

    /** close table */
    int closeDB(Table * tab);

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

    /**  */
    int cp_getDbTable(string &str);

    /** build DB DICT */
    int cp_buildDict(DictOpt opt, string info, int count);

    /** command line */
    char m_cmd_line[BUFFSIZE];

    /** database list */
    list<Database> databases;
};

#endif // COMMANDER_H
