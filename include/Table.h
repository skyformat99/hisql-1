#ifndef TABLE_H
#define TABLE_H

#include "common.h"

class Table
{
public:
    string m_file_path;

    string m_dbname;

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

    DataSet::iterator getRowIte();
    bool IteEnd(DataSet::iterator it);

    /**
     * push_back a row to the data_array
     * @param   arow a row
     * @return       list<char *>*
     */
    Row* pushRow(vector<string> &arow);

    Row* getRow(int row);

    Row* getLastRow();

    bool printRow(DataSet::iterator it);

    bool removeRow(DataSet::iterator it);

    bool makeRowString(DataSet::iterator &it, vector<string> &tmp);

    Row removeRowByNum(int no);

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

    void showRows();

    int writeFile();

    int readFile();

    /**
     * to get the field by name
     * @param  field field name
     * @return       pointer to the column
     */
    Column *getField(string field);
    int getFieldLoc(string field);

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

    int setFields(ColumnList& col);

    ColumnList::iterator getColIte();
    bool colIteEnd(ColumnList::iterator it);

    void clearAll();

    bool dataCheck(string& data, Column &col);

    bool readIndex(vector<Row *>& tmp, string indexname,string field);

    bool writeIndex(string indexname, string field);

    DataSet m_data_array;

    ColumnList m_col_list;
private:
    string name;
};

#endif // TABLE_H
