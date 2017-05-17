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



#endif  /*TEST_VECTOR_H*/
