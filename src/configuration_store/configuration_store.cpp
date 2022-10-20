#include "configuration_store.hpp"
#include "configuration_store.h"
using namespace configuration_store;

ConfigurationStore<ConfigurationStoreStructure> &config_store() {
    return ConfigurationStore<ConfigurationStoreStructure>::GetStore();
}
