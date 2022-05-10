#include "InterceptorRegistry.h"

InterceptorRegistry::~InterceptorRegistry() {
    {
        for (auto&&[k, interceptors]: urlRegexMap) {
            for (auto &&item: interceptors) {
                delete item;
            };
        }
    }
}
