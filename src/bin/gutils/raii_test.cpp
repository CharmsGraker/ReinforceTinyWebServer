#include "resource_raii.h"
#include <iostream>
int main() {
    raii_guard<int> raiiGuard(1);
    std::cout << *raiiGuard.get_object();
}