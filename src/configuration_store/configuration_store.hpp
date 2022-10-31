#pragma once
#include "eeprom_access.hpp"
#include "filament.hpp"
#include <utility>
#include <tuple>
#include "configuration_store_structure.hpp"
#include "item_updater.hpp"
#include "mem_config_item.hpp"
#include <variant>

#ifndef EEPROM_UNITTEST
    #include "disable_interrupts.h"
#endif

namespace configuration_store {
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
using Backend = std::variant<EepromAccess>;

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
#ifdef EEPROM_UNITTEST
public:
#endif
    Backend backend;

public:
    template <class T>
    void set(const char *key, const T &data) {
        std::visit(
            [&](auto &access) { access.template set(key, data); },
            backend);
    };

    ConfigurationStore(Backend &&backend)
        : backend(std::move(backend)) {};
    ConfigurationStore(const ConfigurationStore &other) = delete;
    void init();
    static ConfigurationStore &GetStore();
    void dump_data(Backend data_dump, bool save_default_values = false);
    void factory_reset();
};

template <class CONFIG>
void ConfigurationStore<CONFIG>::init() {
    ItemUpdater updater(*this);
    std::visit([&](auto &access) { access.init(updater); },
        backend);
}

template <class CONFIG>
ConfigurationStore<CONFIG> &ConfigurationStore<CONFIG>::GetStore() {
    static ConfigurationStore<CONFIG> store(EepromAccess {});
    return store;
}

/**
 * @brief sets values of items to their default value and invalidates data in eeprom
 * @tparam CONFIG
 */
template <class CONFIG>
void ConfigurationStore<CONFIG>::factory_reset() {
    // just invalidate the eeprom and reset the printer
    std::unique_lock lock(get_item_mutex());
    EepromAccess::instance().reset();
}

template <class CONFIG>
void ConfigurationStore<CONFIG>::dump_data(Backend data_dump, bool save_default) {
    backend.swap(data_dump);
    std::apply([&](auto &... items) {
        ((items.dump_data(save_default)), ...);
    },
        CONFIG::tuplify());
    backend.swap(data_dump);
}

template <class T, class CovertTo>
void MemConfigItem<T, CovertTo>::set(T new_data) {
    std::unique_lock<FreeRTOS_Mutex> lock(get_item_mutex());
    if (data != new_data) {
#ifndef EEPROM_UNITTEST
        buddy::DisableInterrupts disable;
#endif
        data = new_data;
        // using eeprom access singleton directly, because I don't want to have pointer in every item
        ConfigurationStore<>::GetStore().template set(key, data);
    }
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
#ifndef EEPROM_UNITTEST
        buddy::DisableInterrupts disable;
#endif
        data = new_data;
        // using eeprom access singleton directly, because I don't want to have pointer in every item
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
template <class T, size_t SIZE>
std::array<T, SIZE> MemConfigItem<std::array<T, SIZE>>::get() {
#ifndef EEPROM_UNITTEST
    buddy::DisableInterrupts disable;
#endif
    return data;
}

template <class T, size_t SIZE>
void MemConfigItem<std::array<T, SIZE>>::dump_data(bool save_default) {
    if (save_default || check_data()) {
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
template <size_t SIZE>
void MemConfigItem<std::array<char, SIZE>>::set(const char *new_data) {
    if (strcmp((char *)(data.data()), new_data) != 0) {
        std::unique_lock<FreeRTOS_Mutex> lock(get_item_mutex());
        {
#ifndef EEPROM_UNITTEST
            buddy::DisableInterrupts disable;
#endif
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
#ifndef EEPROM_UNITTEST
            buddy::DisableInterrupts disable;
#endif
            data = new_data;
        }
        // using eeprom access singleton directly, because I don't want to have pointer in every item
        ConfigurationStore<>::GetStore().template set(key, data);
    }
}
template <size_t SIZE>
std::array<char, SIZE> MemConfigItem<std::array<char, SIZE>>::get() {
#ifndef EEPROM_UNITTEST
    buddy::DisableInterrupts disable;
#endif
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
template <class T, size_t SIZE>
bool MemConfigItem<std::array<T, SIZE>>::check_data() {
    for (const T &item : data) {
        if (item != def_val) {
            return false;
        }
    }
    return true;
}
}
configuration_store::ConfigurationStore<configuration_store::ConfigurationStoreStructure> &config_store();
