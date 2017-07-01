// Last Update:2017-07-01 15:57:33
/**
 * @file bubble_sort.cpp
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
    int arry[10] = { 3,2,4,5,9,7,8,6,10,1};
    vector<int> vec (arry, arry + 10);
    vector<int>::iterator iter = vec.begin();
    for(; iter != vec.end(); ++iter)
    {
        cout<<*iter<<",";
    }
    cout<<endl;
    auto it = vec.rbegin();
    for(; it != vec.rend(); ++it)
    {
        cout<<*it<<" ";
    }
    cout<<endl;

    bool happen_swap = false;
    int count_swap = 0;
    int cout_compare = 0;
    auto first = vec.begin();
    auto foruth = vec.end();
    while( first != foruth)
    {
        auto second = first;
        ++second;
        if(second != foruth)
        {
            if(*second < *first)
            {
                auto tmp = *first;
                *first = *second;
                *second = tmp;
                happen_swap = true;
                ++count_swap;
            }
            ++cout_compare;
        }
        else
        {
            if(happen_swap)
            {
                foruth = first;
                first = vec.begin();
                happen_swap = false;
                continue;
            }
        }
        ++first;
    }

    cout<<"bubble_sort is ";
    for(auto third = vec.begin(); third != vec.end(); ++third)
    {
        cout<<*third<<" ";
    }
    cout<<endl;
    cout<<" count_swap is " << count_swap <<endl;
    cout<<" cout_compare is " << cout_compare <<endl;

}
