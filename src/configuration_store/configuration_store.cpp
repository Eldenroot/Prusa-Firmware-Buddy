#include "configuration_store.hpp"
#include "configuration_store.h"
using namespace configuration_store;

ConfigurationStore<ConfigurationStoreStructure> &config_store() {
    return ConfigurationStore<ConfigurationStoreStructure>::GetStore();
}
FreeRTOS_Mutex &configuration_store::get_item_mutex() {
    static FreeRTOS_Mutex mutex;
    return mutex;
}
