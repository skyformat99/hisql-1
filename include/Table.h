#ifndef TABLE_H
#define TABLE_H

#include "common.h"

class Table
{
public:
    string m_file_path;

    string m_dbname;

    list<vector<char *> >::iterator it;

    /** Default constructor */
    Table();

    Table(string tabname, string dbname);

    /** Default destructor */
    virtual ~Table();

    /** modify column type */
    enum ModType
    {
        MT_NAME, MT_TYPE, MT_LEN,
        MT_KEY, MT_NULL,
        MT_DEFVAL, MT_EXTRA,
        MT_COMMENT
    };

    string getName();
    void setName(string tabname);

    /**
     * get data pointer
     * @param   row the row locate
     * @param   col the col locate
     * @return      char*
     */
    char *getData(int row, int col);

    list<vector<char *> >::iterator getFirstRowIte();
    list<vector<char *> >::iterator getNextRowIte();
    bool IteEnd();

    /**
     * push_back a row to the data_array
     * @param   arow a row
     * @return       list<char *>*
     */
    vector<char *>* pushRow(vector<string> &arow);

    vector<char *>* getRow(int row);

    bool printRow(list<vector<char *> >::iterator it);

    bool removeRow(list<vector<char *> >::iterator it);

    bool makeRowString(list<vector<char *> >::iterator &it, vector<string> &tmp);

    vector<char *> removeRowByNum(int no);

    /**
     * return the row size
     * @return to get the row count
     */
    int countRow();

    /**
     * return the column count
     * @return to get the column count
     */
    int countCol();

    void showColumns();

    int writeFile();

    int readFile();

    /**
     * to get the field by name
     * @param  field field name
     * @return       pointer to the column
     */
    Column *getField(string field);

    /**
     * set field's value
     * @param field field's name
     * @param mod   modify type
     * @param str   value
     */
    void setField(string field, ModType mod, string str);

    /**
     * remove a field
     * @param  field field's name
     * @return       if done then true
     */
    bool removeField(string field);

    /**
     * to add a field
     * @param   acol a Column type
     * @return       if done then true
     */
    bool addField(Column acol);
private:
    string name;

    list<vector<char *> > m_data_array;

    list<Column> m_col_list;
};

#endif // TABLE_H
