//
// Created by copilot on 12/28/2024.
//

#ifndef MLA_GLOBAL_CONFIG_ESP32_H
#define MLA_GLOBAL_CONFIG_ESP32_H

#include "../../core/config/mla_config.h"

#if !defined(mla_max_config_size)
#define mla_max_config_size (16 * 1024) // 16KB is default
#endif

#include <nvs_flash.h>
#include <nvs.h>

#define MLA_ESP32_NVS_NAMESPACE "mla_config"
#define MLA_ESP32_NVS_KEY "config_data"

// On ESP32 the configuration is stored in NVS (Non-Volatile Storage)
// NVS is a key-value storage system that persists across reboots

mla_bytes_t mla_private_esp32_read_config_input() {

    // Initialize NVS if not already initialized
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        return mla_bytes_empty();
    }

    // Open NVS handle
    nvs_handle_t nvs_handle;
    err = nvs_open(MLA_ESP32_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return mla_bytes_empty();
    }

    // Get the size of the stored blob
    size_t required_size = 0;
    err = nvs_get_blob(nvs_handle, MLA_ESP32_NVS_KEY, nullptr, &required_size);
    if (err != ESP_OK || required_size == 0 || required_size > mla_max_config_size) {
        nvs_close(nvs_handle);
        return mla_bytes_empty();
    }

    // Allocate buffer for config data
    mla_bytes_t config_data = mla_bytes(required_size);
    if (config_data.size != required_size) {
        nvs_close(nvs_handle);
        config_data = mla_bytes_empty();
        return mla_bytes_empty();
    }

    // Read the blob
    err = nvs_get_blob(nvs_handle, MLA_ESP32_NVS_KEY, mla_bytes_get_data_for_writing(config_data), &required_size);
    nvs_close(nvs_handle);

    if (err != ESP_OK) {
        config_data = mla_bytes_empty();
        return mla_bytes_empty();
    }

    return config_data;
}

mla_bytes_t mla_private_esp32_create_config_output_buffer() {

    return mla_bytes(mla_max_config_size);
}

mla_bool_t mla_private_esp32_commit_config_output(mla_bytes_t& output, mla_size_t unused_bytes) {

    // Initialize NVS if not already initialized
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        output = mla_bytes_empty();
        return false;
    }

    // Open NVS handle for writing
    nvs_handle_t nvs_handle;
    err = nvs_open(MLA_ESP32_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        output = mla_bytes_empty();
        return false;
    }

    // Calculate actual bytes to write
    mla_size_t bytesToWrite = output.size - unused_bytes;
    if (bytesToWrite > output.size) {
        bytesToWrite = output.size;
    }

    // Write the blob to NVS
    err = nvs_set_blob(nvs_handle, MLA_ESP32_NVS_KEY, output.data, bytesToWrite);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        output = mla_bytes_empty();
        return false;
    }

    // Commit the changes
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    output = mla_bytes_empty();

    return err == ESP_OK;
}

mla_bool_t mla_private_esp32_reset() {

    // Initialize NVS if not already initialized
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        return false;
    }

    // Open NVS handle
    nvs_handle_t nvs_handle;
    err = nvs_open(MLA_ESP32_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return false;
    }

    // Erase the config key
    err = nvs_erase_key(nvs_handle, MLA_ESP32_NVS_KEY);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(nvs_handle);
        return false;
    }

    // Commit the changes
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return err == ESP_OK;
}

mla_config_low_level_operations_t g_config_low_level_operations = {
    mla_private_esp32_read_config_input,
    mla_private_esp32_create_config_output_buffer,
    mla_private_esp32_commit_config_output,
    mla_private_esp32_reset
};

#endif
