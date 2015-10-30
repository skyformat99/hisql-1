#include <iostream>
#include <fstream>
#include <list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Commander.h"

using namespace std;

void recursive (vector<vector<string> > &dimValue, vector<vector<string> > &result, int layer, vector<string> &curList)
{
    if (layer < (int)dimValue.size() - 1)
    {
        if (dimValue.at(layer).size() == 0)
        {
            recursive(dimValue, result, layer + 1, curList);
        }
        else
        {
            for (int i = 0; i < (int)dimValue.at(layer).size(); i++)
            {
                vector<string> newlist(curList);
                newlist.push_back(dimValue.at(layer).at(i));
                recursive(dimValue, result, layer + 1, newlist);
            }
        }
    }
    else if (layer == (int)dimValue.size() - 1)
    {
        if (dimValue.at(layer).size() == 0)
        {
            result.push_back(curList);
        }
        else
        {
            for (int i = 0; i < (int)dimValue.at(layer).size(); i++)
            {
                vector<string> newlist(curList);
                newlist.push_back(dimValue.at(layer).at(i));
                result.push_back(newlist);
            }
        }
    }
}

int main()
{
    srand(time(NULL));

        /*vector<string> list1;
        list1.push_back("1");
        list1.push_back("2");

        vector<string> list2;
        list2.push_back("a");
        list2.push_back("b");

        vector<string> list3;
        list3.push_back("3");
        list3.push_back("4");
        list3.push_back("5");

        vector<string> list4;
        list4.push_back("c");
        list4.push_back("d");
        list4.push_back("e");

        vector<vector<string> > dimValue;
        dimValue.push_back(list1);
        dimValue.push_back(list2);
        dimValue.push_back(list3);
        dimValue.push_back(list4);

        vector<vector<string> > recursiveResult;
        /// 递归实现笛卡尔积
        vector<string> tmp;
        recursive(dimValue, recursiveResult, 0, tmp);

        cout << "递归实现笛卡尔乘积: 共 " << recursiveResult.size() << " 个结果" << endl;

        for (size_t i = 0; i < recursiveResult.size(); ++i) {
            for (size_t j = 0; j < recursiveResult.at(i).size(); ++j) {
                cout << recursiveResult.at(i).at(j) << " ";
            }
            cout << endl;
        }*/

    dbms_start();

    dbms_info();

    Commander * cmd = Commander::getCommander();

    cmd->reader();

    return 0;
}
