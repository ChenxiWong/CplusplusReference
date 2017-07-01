// Last Update:2017-07-01 17:41:26
/**
 * @file selection_sort.cpp
 * @brief 
 * @author wangchenxi
 * @version 0.1.00
 * @date 2017-07-01
 */
#include <iostream>

#include <vector>

using namespace std;

int main(int argc, char* argv[])
{

    int arry[10] = {3, 2, 4, 5, 9, 7, 8, 6, 10, 1};
    vector<int> vec(arry, arry + 10);
    for(auto iter = vec.begin(); iter != vec.end(); ++iter)
    {
        cout<<*iter << " ";
    }
    cout<<endl;
    auto first = vec.begin();
    auto second = vec.end();
    int count_swap = 0;
    int count_compare = 0;
    for(auto third = first; third != second; )
    {
        auto fourth = third;
        ++fourth;
        if(fourth != second)
        {
            if( *fourth < *first)
            {
                int tmp = *fourth;
                *fourth = *first;
                *first = tmp;
                ++count_swap;
            }
            ++count_compare;
        }
        else
        {
            ++first;
            third = first;
            continue;
        }
        ++third;
    }
    cout<<" count_swap is " << count_swap <<endl;
    cout<<" count_compare is " << count_compare<<endl;
    cout<<" selection_sort is ";
    for(auto iter = vec.begin(); iter != vec.end(); ++iter)
    {
        cout<<*iter << " ";
    }
    cout<<endl;



}
