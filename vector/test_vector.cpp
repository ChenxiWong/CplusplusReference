// Last Update:2017-05-16 10:05:01
/**
 * @file test_vector.cpp
 * @brief 测试vector's Muthod.
 * @author wangchenxi
 * @version 0.1.00
 * @date 2017-05-16
 */
#include "test_vector.h"

#define test_work(m)  \
    do \
    { \
        m::test_main( argc, argv); \
    } while(0)

int main(int argc, char* argv[])
{
    test_work(common_use);
    std::string str("");
    std::cout<<"test rbegin_rend , input: reverse"<<std::endl;
    std::cin>>str;
    if(str == "reverse")
    {
        test_work(rbegin_rend);
    }
    return 0;
}
