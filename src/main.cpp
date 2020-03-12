#include "TaskPool.h"
#include <iostream>

int print(int i){
    std::this_thread::sleep_for(std::chrono::milliseconds(i%5*300));
    using namespace std;
    cout << "argc = " << i <<"\n";
    return i;
}

int main(int argc, char* argv[]){
    using namespace std;
    auto m_taskPool = std::make_shared<TaskPool>(60);

    for(int i = 0;i<10000;++i){
        m_taskPool->push(false,[i](){return print(i);});
    }

    auto re = m_taskPool->push(true,[](int i){return print(i);},200);

    cout << "re get " << re.get() << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(100));
    return 0;
}