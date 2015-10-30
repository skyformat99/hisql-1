#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include <stdlib.h>
#include <string.h>

#include "Table.h"

Table::Table()
{
    name = "";
    m_dbname = "";
    m_file_path = "";
}

Table::Table(string tabname, string dbname)
{
    name = tabname;
    m_dbname = dbname;

    m_file_path = string(DBFPATH) + m_dbname + "\\" + name + ".tbl";
}

Table::~Table()
{
    //dtor
}

string Table::getName()
{
    return name;
}

void Table::setName(string tabname)
{
    name = tabname;
}

list<vector<char *> >::iterator Table::getFirstRowIte()
{
    this->it = this->m_data_array.begin();
    return it;
}

list<vector<char *> >::iterator Table::getNextRowIte()
{
    ++this->it;
    return it;
}

bool Table::IteEnd()
{
    if (this->it == this->m_data_array.end()) return true;
    return false;
}

/**
 * get data pointer
 * @param   row the row locate
 * @param   col the col locate
 * @return      char*
 */
char* Table::getData(int row, int col)
{
    if (row >= countRow() || col >= countCol()) return NULL;

    list<vector<char *> >::iterator beg = this->m_data_array.begin();

    for (int i = 0; i <= row; ++i)
    {
        ++beg;
        return (*beg).at(col);
    }
    return NULL;
}

/**
 * push_back a row to the data_array
 * @param   arow a row
 * @return       list<char *>*
 */
vector<char *>* Table::pushRow(vector<string> &arow)
{
    if ((int)arow.size() == countCol())
    {
        vector<char *> tmprow;
        int i = 0;

        list<Column>::iterator beg, end = this->m_col_list.end();
        for (beg = this->m_col_list.begin(); beg != end; ++beg)
        {
            /// check data
            char * tmp = new char[(*beg).length];
            strcpy(tmp, arow.at(i).c_str());
            tmprow.push_back(tmp);
            ++i;
        }

        this->m_data_array.push_back(tmprow);
        arow.clear();
        return &this->m_data_array.back();
    }
    else
    {
        cout << " this Row can not insert in this table." << endl;
        return NULL;
    }
}

vector<char *>* Table::getRow(int row)
{
    list<vector<char *> >::iterator beg = this->m_data_array.begin();

    for (int i = 0; i <= row; ++i)
    {
        ++beg;
        return &(*beg);
    }
    return NULL;
}

bool Table::printRow(list<vector<char *> >::iterator it)
{
    int tsize = countCol();
    for (int i = 0; i < tsize; ++i)
    {
        cout << setw(20) << (*it).at(i);
    }
    cout << endl;
    return true;
}

bool Table::removeRow(list<vector<char *> >::iterator it)
{
    list<vector<char *> >::iterator target = it;
    this->m_data_array.erase(target);
    return true;
}

bool Table::makeRowString(list<vector<char *> >::iterator &it, vector<string> &tmp)
{
    int tsize = countCol();
    for (int i = 0; i < tsize; ++i)
    {
        tmp.push_back((*it).at(i));
    }
    return true;
}

vector<char *> Table::removeRowByNum(int no)
{
    list<vector<char *> >::iterator head = getFirstRowIte();

    int i = 0;

    while (! IteEnd())
    {
        if (i == no)
        {
            removeRow(head);
            return *head;
        }
        ++i;
        head = getNextRowIte();
    }
    return vector<char *>();
}

/**
 * return the row size
 * @return to get the row count
 */
int Table::countRow()
{
    return (int)this->m_data_array.size();
}

/**
 * return the column count
 * @return to get the column count
 */
int Table::countCol()
{
    return (int)this->m_col_list.size();
}

void Table::showColumns()
{
    vector<string> tmp;

    tmp.push_back("Field");
    tmp.push_back("Type");
    tmp.push_back("Null");
    tmp.push_back("Key");
    tmp.push_back("Default");
    tmp.push_back("Extra");

    list<Column>::iterator beg, end = this->m_col_list.end();

    for (beg = this->m_col_list.begin(); beg != end; ++beg)
    {
        tmp.push_back((*beg).field);

        char tmpchar[32];
        string ttmp;
        switch ((*beg).type)
        {
        case INT:
            sprintf(tmpchar, "int(%d)", (*beg).length);
            ttmp = string(tmpchar);
            if ((*beg).u_sign) ttmp += " unsigned";
            break;
        case FLOAT:
            sprintf(tmpchar, "float(%d)", (*beg).length);
            ttmp = string(tmpchar);
            if ((*beg).u_sign) ttmp += " unsigned";
            break;
        case CHAR:
            sprintf(tmpchar, "char(%d)", (*beg).length);
            ttmp = string(tmpchar);
            break;
        default:
            cout << "Fatal Error: Undefined Type!" << endl;
            exit(-1);
            break;
        }
        tmp.push_back(ttmp);

        tmp.push_back(string((*beg).null));
        tmp.push_back(string((*beg).key));
        tmp.push_back(string((*beg).default_val));
        tmp.push_back(string((*beg).extra));
    }

    g_print_table(tmp, 6);
}

int Table::writeFile()
{
    ofstream out(m_file_path.c_str(), ios::binary);
    if (out.is_open())
    {
        int rowlong = countRow();
        out.write((char*)&rowlong, sizeof(int));
        //long ip = out.tellp();
        list<vector<char *> >::iterator beg, end = this->m_data_array.end();
        for (beg = this->m_data_array.begin(); beg != end; ++beg)
        {
            int i = 0;
            list<Column>::iterator ibeg, iend = this->m_col_list.end();
            for (ibeg = this->m_col_list.begin(); ibeg != iend; ++ibeg)
            {
                //out << setw((*ibeg).length) << (*beg).at(i++);
                //long offset = (*ibeg).length * sizeof(char);
                //out.write((*beg).at(i++), offset);
                out.write((*beg).at(i++), (*ibeg).length * sizeof(char));
                //ip += offset;
            }
            //out << '\n';
            //out.seekp(ip);
        }

        out.close();
        return 0;
    }
    return -1;
}

int Table::readFile()
{
    ifstream in(m_file_path.c_str());
    if (in.is_open())
    {
        //long ip = in.tellg();
        int rowlong;
        in.read((char*)&rowlong, sizeof(int));

        m_data_array.clear();

        for (int i = 0; i < rowlong; ++i)
        {
            vector<char *> tmp;
            list<Column>::iterator beg, end = this->m_col_list.end();
            for (beg = this->m_col_list.begin(); beg != end; ++beg)
            {
                char *ch;
                ch = new char[(*beg).length];
                in.read(ch, (*beg).length * sizeof(char));
                tmp.push_back(ch);
            }
            this->m_data_array.push_back(tmp);
        }

        in.close();
        return 0;
    }
    return -1;
}

/**
 * to get the field by name
 * @param  field field name
 * @return       pointer to the column
 */
Column* Table::getField(string field)
{
    list<Column>::iterator beg, end = m_col_list.end();
    for (beg = m_col_list.begin(); beg != end; ++beg)
    {
        if (strcmp((*beg).field, field.c_str()) == 0)
        {
            return &(*beg);
        }
    }
    return NULL;
}

/**
 * set field's value
 * @param field field's name
 * @param mod   modify type
 * @param str   value
 */
void Table::setField(string field, ModType mod, string str)
{
    Column * col = getField(field);

    if (col != NULL)
    {
        switch(mod)
        {
        case MT_NAME:
            strcpy(col->field, field.c_str());
            break;
        case MT_TYPE:
            if (str == "int")
            {
                col->type = INT;
            }
            else if (str == "float")
            {
                col->type = FLOAT;
            }
            else if (str == "char")
            {
                col->type = CHAR;
            }
            else
            {
                return;
            }
            break;
        case MT_LEN:
            col->length = atoi(str.c_str());
            break;
        case MT_KEY:
            if (str == "PRI")
            {
                strcpy(col->key, "PRI");
            }
            break;
        case MT_NULL:
            if (str == "NO")
            {
                strcpy(col->null, "NO");
            }
            if (str == "YES")
            {
                strcpy(col->null, "YES");
            }
            break;
        case MT_DEFVAL:
            /// check string can be a default value or not
            strcpy(col->default_val, str.c_str());
            break;
        case MT_EXTRA:
            strcpy(col->extra, str.c_str());
            break;
        case MT_COMMENT:
            strcpy(col->comment, str.c_str());
            break;
        default:
            return;
        }
        /// may be run the field test.
    }
    cout << "Could not find field '" << field << "'." << endl;
    return;
}

/**
 * remove a field
 * @param  field field's name
 * @return       if done then true
 */
bool Table::removeField(string field)
{
    list<Column>::iterator beg, end = m_col_list.end();
    int i = 0;
    for (beg = m_col_list.begin(); beg != end; ++beg)
    {
        if (strcmp((*beg).field, field.c_str()) == 0)
        {
            m_col_list.erase(beg);
            /// remove field in data array
            list<vector<char *> >::iterator dbeg, dend = this->m_data_array.end();
            for (dbeg = this->m_data_array.begin(); dbeg != dend; ++dbeg)
            {
                (*dbeg).erase((*dbeg).begin() + i);
            }
            return true;
        }
        ++i;
    }
    return false;
}

/**
 * to add a field
 * @param   acol a Column type
 * @return       if done then true
 */
bool Table::addField(Column acol)
{
    Column *tcol = getField(acol.field);

    if (tcol == NULL)
    {
        m_col_list.push_back(acol);
        return true;
    }
    cout << "field '" << acol.field << "' is exists!" << endl;
    return false;
}
