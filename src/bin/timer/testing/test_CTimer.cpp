#include "../CTimer.h"
#include "../../concurrent/ThreadFunction.h"

#include <iostream>

using namespace std;


int main() {

    auto thread_f = new ThreadFunction();
    thread_f->setRunFunction([]() {
        printf("a\n");
    });
    thread_f->registerLoop(100);
    getchar();
}
