#pragma once
#include "eeprom_access.hpp"
#include "filament.hpp"
#include <utility>
#include <tuple>
#include "configuration_store_structure.hpp"
#include "item_updater.hpp"
#include "disable_interrupts.h"

namespace configuration_store {
enum class BackendType {
    Eeprom
};
FreeRTOS_Mutex &get_item_mutex();
/**
 * This class is configuration store which provides easy access to its values.
 * It initializes the values from eeprom and persists them into it on change.
 * Data is written to eeprom only if the value is non-default or if it is overwriting non-default data with default data
 *
 * To add new item to configuration store, you need to add it to json from which it is generated
 *
 * The values are stored in RAM so reads does not read data from the eeprom.
 *
 * To access the values get singleton instance of the store and access the items like members of any other class. To change the items use their Set and Get methods.
 */
template <class CONFIG>
class ConfigurationStore : public CONFIG {
    BackendType selected_backend;
    EepromAccess eeprom_access;

public:
    template <class T>
    void set(const char *key, const T &data) {
        switch (selected_backend) {
        case BackendType::Eeprom: {
            // could fail only if we need to do clean up. By dumping non-default data from configuration store we solve problem with old invalid types still stored in eeprom
            bool res = eeprom_access.template set(key, data);
            if (!res) {
                dump_data(BackendType::Eeprom, false);
            }
        } break;
        }
    };

    ConfigurationStore() = default;
    ConfigurationStore(const ConfigurationStore &other) = delete;
    void init();
    static ConfigurationStore &GetStore();
    void dump_data(BackendType backend, bool save_default_values = false);
    void factory_reset();
};

template <class CONFIG>
void ConfigurationStore<CONFIG>::init() {
    ItemUpdater updater(*this);
    EepromAccess::instance().init(updater);
}

template <class CONFIG>
ConfigurationStore<CONFIG> &ConfigurationStore<CONFIG>::GetStore() {
    static ConfigurationStore store {};
    return store;
}

/**
 * @brief sets values of items to their default value and invalidates data in eeprom
 * @tparam CONFIG
 */
template <class CONFIG>
void ConfigurationStore<CONFIG>::factory_reset() {

    std::apply([&](auto &... items) {
        ((items.set_to_default()), ...);
    },
        CONFIG::tuplify());
    EepromAccess::instance().reset();
}
template <class CONFIG>
void ConfigurationStore<CONFIG>::dump_data(BackendType backend, bool save_default) {
    BackendType current_backend = selected_backend;
    selected_backend = backend;
    std::apply([&](auto &... items) {
        ((items.dump_data(save_default)), ...);
    },
        CONFIG::tuplify());
    selected_backend = current_backend;
}
template <class T, class CovertTo>
void MemConfigItem<T, CovertTo>::set(T new_data) {
    std::unique_lock<FreeRTOS_Mutex> lock(get_item_mutex());
    if (data != new_data) {
        buddy::DisableInterrupts disable;
        data = new_data;
    }
    // using eeprom access singleton directly, because I don't want to have pointer in every item
    ConfigurationStore<>::GetStore().template set(key, data);
}
template <class T, class CovertTo>
T MemConfigItem<T, CovertTo>::get() {
    std::unique_lock<FreeRTOS_Mutex> lock(get_item_mutex());
    return data;
}

template <class T, class CovertTo>
void MemConfigItem<T, CovertTo>::init(const T &new_data) {
    data = new_data;
}

template <class T, class CovertTo>
void MemConfigItem<T, CovertTo>::dump_data(bool save_default) {
    if (save_default || (data != def_val)) {
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
template <class T, size_t SIZE>
void MemConfigItem<std::array<T, SIZE>>::init(const std::array<T, SIZE> &new_data) {
    data = new_data;
}

template <class T, size_t SIZE>
void MemConfigItem<std::array<T, SIZE>>::set(const std::array<T, SIZE> &new_data) {
    std::unique_lock<FreeRTOS_Mutex> lock(get_item_mutex());
    if (data != new_data) {
        buddy::DisableInterrupts disable;
        data = new_data;
    }
    // using eeprom access singleton directly, because I don't want to have pointer in every item
    ConfigurationStore<>::GetStore().template set(key, data);
}
template <class T, size_t SIZE>
std::array<T, SIZE> MemConfigItem<std::array<T, SIZE>>::get() {
    buddy::DisableInterrupts disable;
    return data;
}

template <class T, size_t SIZE>
void MemConfigItem<std::array<T, SIZE>>::dump_data(bool save_default) {
    if (save_default || (data != def_val)) {
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
template <size_t SIZE>
void MemConfigItem<std::array<char, SIZE>>::set(const char *new_data) {
    if (strcmp((char *)(data.data()), new_data) != 0) {
        std::unique_lock<FreeRTOS_Mutex> lock(get_item_mutex());
        {
            buddy::DisableInterrupts disable;
            strncpy(data.data(), new_data, SIZE);
        }
        // using eeprom access singleton directly, because I don't want to have pointer in every item
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
template <size_t SIZE>
void MemConfigItem<std::array<char, SIZE>>::set(const std::array<char, SIZE> &new_data) {
    if (data != new_data) {
        std::unique_lock<FreeRTOS_Mutex> lock(get_item_mutex());
        {
            buddy::DisableInterrupts disable;
            data = new_data;
        }
        // using eeprom access singleton directly, because I don't want to have pointer in every item
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
template <size_t SIZE>
std::array<char, SIZE> MemConfigItem<std::array<char, SIZE>>::get() {
    buddy::DisableInterrupts disable;
    return data;
}
template <size_t SIZE>
void MemConfigItem<std::array<char, SIZE>>::init(const std::array<char, SIZE> &new_data) {
    data = new_data;
}
template <size_t SIZE>
void MemConfigItem<std::array<char, SIZE>>::dump_data(bool save_default) {
    if (save_default || (strcmp(def_val, data.data()) != 0)) {
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
}
configuration_store::ConfigurationStore<configuration_store::ConfigurationStoreStructure> &config_store();
