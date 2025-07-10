#include <common/common.h>

using namespace CommonWorker;

void test_log()
{
    for(int i=0; i< 10; i++)
    {
        LOGER_ERROR() << "test_log 1-10:" << i;
        std::cout << *CommonWorker::Log::Instance();
    }
}

int main(int argc, char **argv)
{
    test_log();
    return 0;
}