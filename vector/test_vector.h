// Last Update:2017-05-16 10:05:01
/**
 * @file test_vector.h
 * @brief vector Mathod come true.
 * @author wangchenxi
 * @version 0.1.00
 * @date 2017-05-16
 */

#ifndef TEST_VECTOR_H
#define TEST_VECTOR_H

#include <iostream>
#include <vector>

namespace rbegin_rend
{
    int test_main(int argc, char* argv[])
    {
        std::vector<int> myvector(5); // 5 default-constructed ints;

        for(std::vector<int>::reverse_iterator iter = myvector.rbegin(); iter != myvector.rend(); ++iter)
            std::cout << ' '<< *iter ;
        std::cout << '\n';

        return 0;
    }
}
namespace common_use
{
    int test_main(int argc, char* argv[])
    {
        std::vector<int> client(16);
        std::cout<<"client size is "<<client.size()<<std::endl;
        int* p = &(*(client.begin()));
        int* q = p + 16;
        for(;p != q; ++p)
        {
            *p = 100;
            std::cout<< int(q-p) <<std::endl;
        }
        std::vector<int>::reverse_iterator iter = client.rbegin();
        for(; iter != client.rend(); ++iter)
        {
            std::cout<< *iter <<std::endl;
        }
    return 0;
    }
}


#endif  /*TEST_VECTOR_H*/
