#ifndef MLA_HASH_MAP_H
#define MLA_HASH_MAP_H

#include "../mla_data_types.h"
#include "mla_array_list.h"

#define mla_hash_map_template_full class TKey, class TKeyInit, class Hasher, class TValue, class TValueInit = mla_default_init(TValue)
#define mla_hash_map_template class TKey, class TKeyInit, class TValue, class TValueInit = mla_default_init(TValue)

#define mla_hash_map_t_param TKey, TKeyInit, TValue, TValueInit
#define mla_hash_map_t_param_full TKey, TKeyInit, Hasher, TValue, TValueInit

template < mla_hash_map_template >
struct mla_hash_map_bucket_item_t {
    TKey key; // Key of the item
    TValue value; // Value associated with the key

    static mla_hash_map_bucket_item_t init() {
        return {
            TKeyInit::init(), // Initialize key using the provided initializer
            TValueInit::init() // Initialize value using the provided initializer
        };
    }
};

template < mla_hash_map_template >
struct mla_hash_map_bucket_t {
    mla_array_list_t<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)> items; // List of items in the bucket

    static mla_hash_map_bucket_t init() {
        return {
            mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)>() // Initialize with a default size
        };
    }
};

template < mla_hash_map_template_full >
struct mla_hash_map_t {
    mla_size_t size;
    mla_size_t bucketCount;
    mla_float_t loadFactor;
    mla_array_list_t<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)> buckets;

    static mla_hash_map_t init() {
        return { 0, 0, 0, mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>() };
    }
};

template < mla_hash_map_template_full >
inline mla_hash_map_t<mla_hash_map_t_param_full> mla_hash_map_empty() {

    return {
        0, // Initialize size to 0
        0, // Set the number of buckets to 0
        0,
        mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>()
    };

}

template < mla_hash_map_template_full >
mla_hash_map_t<mla_hash_map_t_param_full> mla_hash_map(mla_size_t bucketCount = mla_global_config_hash_map_default_bucket_size, mla_float_t loadFactor = mla_global_config_hash_map_default_load_factor) {


    auto array = mla_array_list<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>(bucketCount);
    // Get the actual bucket count (in case the initial capacity was less than requested)
    bucketCount = mla_array_list_capacity(array);

    if (bucketCount > 0) {
        for (mla_size_t i = 0; i < bucketCount; ++i) {

            auto items = mla_array_list<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)>(mla_global_config_hash_map_item_default_size);

            if (mla_array_list_capacity(items) == 0 && mla_global_config_hash_map_item_default_size > 0) {
                return mla_hash_map_empty<mla_hash_map_t_param_full>();
            }

            if (!mla_array_list_add(array, { items })) {
                return mla_hash_map_empty<mla_hash_map_t_param_full>();
            }
        }
    }

    return {
        0, // Initialize size to 0
        bucketCount, // Set the number of buckets
        loadFactor, // Set the load factor
        array // Initialize the buckets with the specified bucket count
    };

}

template < mla_hash_map_template_full >
mla_size_t mla_hash_map_size(mla_hash_map_t<mla_hash_map_t_param_full> &map) {
    return map.size; // Return the current size of the hash map
}

enum mla_hash_map_push_result: mla_int8_t {
    MLA_HASH_MAP_PUSH_REPLACED,
    MLA_HASH_MAP_PUSH_ADDED,
    MLA_HASH_MAP_PUSH_ERROR
};

template < mla_hash_map_template_full >
mla_hash_map_push_result mla_hash_map_push(mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key, const TValue &value, mla_bool_t allowReplace) {

    // Check the load factor
    if (map.bucketCount > 0) {
        mla_float_t currentLoadFactor = mla_s_cast<mla_float_t>(map.size) / mla_s_cast<mla_float_t>(map.bucketCount);
        if (currentLoadFactor > map.loadFactor) {

            // Rebalanced the hash map
            // Create a new bucket array with twice the size
            auto newBuckets = mla_array_list<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>(map.bucketCount * 2);

            // Get the actual new bucket count
            mla_size_t newBucketCount = mla_array_list_capacity(newBuckets);

            if (newBucketCount > map.bucketCount) {

                // Initialize all buckets
                mla_bool_t initSuccess = true;
                for (mla_size_t i = 0; i < newBucketCount; ++i) {
                    auto items = mla_array_list<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)>(mla_global_config_hash_map_item_default_size);

                    if (mla_array_list_capacity(items) == 0 && mla_global_config_hash_map_item_default_size > 0) {
                        initSuccess = false;
                        break;
                    }

                    if (!mla_array_list_add(newBuckets, { items })) {
                        initSuccess = false;
                        break;
                    }
                }

                if (initSuccess) {
                    // Rehash and redistribute all existing items
                    for (mla_size_t i = 0; i < map.bucketCount; ++i) {
                        auto& oldBucket = mla_array_list_get_unsafe(map.buckets, i);

                        for (mla_size_t j = 0; j < mla_array_list_size(oldBucket.items); ++j) {
                            auto& item = mla_array_list_get_unsafe(oldBucket.items, j);

                            // Calculate the new hash index
                            mla_size_t newIndex = 0;
                            if (newBucketCount > 0) {
                                newIndex = Hasher::hash(item.key) % newBucketCount;
                            }

                            // Add the item to the new bucket
                            auto& newBucket = mla_array_list_get_unsafe(newBuckets, newIndex);
                            if (!mla_array_list_add(newBucket.items, item)) {
                                initSuccess = false;
                                break;
                            }
                        }

                        if (!initSuccess) {
                            break;
                        }
                    }
                }

                if (initSuccess) {
                    // Clean up the old buckets' items
                    for (mla_size_t i = 0; i < map.bucketCount; ++i) {
                        auto& oldBucket = mla_array_list_get_unsafe(map.buckets, i);
                        oldBucket.items = mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)>();
                    }

                    // Clean up the old bucket array
                    map.buckets = mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>();

                    // Update the map with the new bucket array and count
                    map.buckets = newBuckets;
                    map.bucketCount = newBucketCount;
                } else {
                    // If we couldn't complete the rebalance, cleanup the new buckets and return error
                    for (mla_size_t i = 0; i < mla_array_list_size(newBuckets); ++i) {
                        auto& bucket = mla_array_list_get_unsafe(newBuckets, i);
                        bucket.items = mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)>();
                    }
                    return MLA_HASH_MAP_PUSH_ERROR;
                }

            } else {
                // If we couldn't increase the bucket count, we can't rebalance
                return MLA_HASH_MAP_PUSH_ERROR;
            }

        }
    }

    if (map.bucketCount == 0) {
        // Initialize with a default bucket count if not already initialized

        map.loadFactor = mla_global_config_hash_map_default_load_factor;
        map.buckets = mla_array_list<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>(mla_global_config_hash_map_default_bucket_size);
        map.bucketCount = mla_array_list_capacity(map.buckets);

        if (map.bucketCount == 0) {
            return MLA_HASH_MAP_PUSH_ERROR;
        }

        for (mla_size_t i = 0; i < map.bucketCount; ++i) {

            auto items = mla_array_list<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)>(mla_global_config_hash_map_item_default_size);

            if (mla_array_list_capacity(items) == 0 && mla_global_config_hash_map_item_default_size > 0) {
                map.buckets = mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>();
                map.bucketCount = 0;
                return MLA_HASH_MAP_PUSH_ERROR;
            }

            if (!mla_array_list_add(map.buckets, { items })) {
                map.buckets = mla_array_list_empty<mla_init_struct(mla_hash_map_bucket_t<mla_hash_map_t_param>)>();
                map.bucketCount = 0;
                return MLA_HASH_MAP_PUSH_ERROR;
            }
        }
    }


    // Calculate the hash index for the key
    mla_int32_t index = Hasher::hash(key) % map.bucketCount;

    // Get the bucket at the calculated index
    mla_hash_map_bucket_t<mla_hash_map_t_param> &bucket = mla_array_list_get_unsafe(map.buckets, index);

    // Check if the key already exists in the bucket
    for (mla_size_t i = 0; i < mla_array_list_size(bucket.items); ++i) {

        auto item = mla_array_list_get_unsafe(bucket.items, i);

        if (item.key == key) {

            if (!allowReplace) {
                return MLA_HASH_MAP_PUSH_ERROR;
            }

            // Key already exists, update the value
            item.value = value;
            return MLA_HASH_MAP_PUSH_REPLACED;
        }
    }

    // Key does not exist, add a new item to the bucket
    mla_hash_map_bucket_item_t<mla_hash_map_t_param> newItem = { key, value };

    if (!mla_array_list_add(bucket.items, newItem)) {
        return MLA_HASH_MAP_PUSH_ERROR;
    }

    map.size = map.size + 1; // Increase the size of the hash map
    return MLA_HASH_MAP_PUSH_ADDED;

}

template < mla_hash_map_template_full >
mla_hash_map_push_result mla_hash_map_push(mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key, const TValue &value) {
    return mla_hash_map_push(map, key, value, true);
}

template < mla_hash_map_template_full >
mla_bool_t mla_hash_map_contains(const mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key) {

    if (map.size == 0 || map.bucketCount == 0) {
        return false; // Empty map cannot contain any keys
    }

    // Calculate the hash index for the key
    mla_int32_t index = Hasher::hash(key) % map.bucketCount;

    // Get the bucket at the calculated index
    const mla_hash_map_bucket_t<mla_hash_map_t_param> &bucket = mla_array_list_get_unsafe(map.buckets, index);

    // Check if the key exists in the bucket
    for (mla_size_t i = 0; i < mla_array_list_size(bucket.items); ++i) {
        if (mla_array_list_get_unsafe(bucket.items, i).key == key) {
            return true; // Key found
        }
    }

    return false; // Key not found
}

template < mla_hash_map_template_full >
mla_bool_t mla_hash_map_remove(mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key) {

    if (map.size == 0 || map.bucketCount == 0) {
        return false; // Empty map cannot contain any keys
    }

    // Calculate the hash index for the key
    mla_int32_t index = Hasher::hash(key) % map.bucketCount;

    // Get the bucket at the calculated index
    mla_hash_map_bucket_t<mla_hash_map_t_param> &bucket = mla_array_list_get_unsafe(map.buckets, index);

    // Check if the key exists in the bucket
    for (mla_size_t i = 0; i < mla_array_list_size(bucket.items); ++i) {

        if (mla_array_list_get_unsafe(bucket.items, i).key == key) {
            // Key found, remove it
            mla_array_list_remove(bucket.items, i);
            map.size = map.size -1; // Decrease the size of the hash map
            return true; // Item removed successfully
        }
    }

    return false; // Key not found
}

template< mla_hash_map_template_full >
mla_array_list_t<TKey, TKeyInit> mla_hash_map_keys(const mla_hash_map_t<mla_hash_map_t_param_full> &map) {

    mla_array_list_t<TKey, TKeyInit> keys = mla_array_list< TKey, TKeyInit >(map.size);

    for (mla_size_t i = 0; i < mla_array_list_size(map.buckets); ++i) {

        auto bucketItem = mla_array_list_get_ref(map.buckets, i);

        for (mla_size_t j = 0; j < mla_array_list_size(bucketItem->items); ++j) {

            auto item = mla_array_list_get_ref(bucketItem->items, j);
            mla_array_list_add(keys, item->key);
        }

    }

    return keys;

}

template< mla_hash_map_template_full >
mla_array_list_t<TValue, TValueInit> mla_hash_map_values(const mla_hash_map_t<mla_hash_map_t_param_full> &map) {

    mla_array_list_t<TValue, TValueInit> values = mla_array_list< TValue, TValueInit >(map.size);

    for (mla_size_t i = 0; i < mla_array_list_size(map.buckets); ++i) {

        auto bucketItem = mla_array_list_get_ref(map.buckets, i);

        for (mla_size_t j = 0; j < mla_array_list_size(bucketItem->items); ++j) {

            auto item = mla_array_list_get_ref(bucketItem->items, j);
            mla_array_list_add(values, item->value);
        }

    }

    return values;

}

template < mla_hash_map_template_full >
mla_bool_t mla_hash_map_get(const mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key, TValue &value) {

    if (map.size == 0 || map.bucketCount == 0) {
        value = TValueInit::init(); // Set value to default if map is empty
        return false; // Empty map cannot contain any keys
    }

    // Calculate the hash index for the key
    mla_int32_t index = Hasher::hash(key) % map.bucketCount;

    // Get the bucket at the calculated index
    const mla_hash_map_bucket_t<mla_hash_map_t_param> &bucket = mla_array_list_get_unsafe(map.buckets, index);

    // Check if the key exists in the bucket
    for (mla_size_t i = 0; i < mla_array_list_size(bucket.items); ++i) {
        const mla_hash_map_bucket_item_t<mla_hash_map_t_param> &item = mla_array_list_get_unsafe(bucket.items, i);

        if (item.key == key) {
            value = item.value; // Set the value to the output parameter
            return true; // Key found
        }
    }

    value = TValueInit::init(); // Set value to default if key not found
    return false; // Key not found
}

template < mla_hash_map_template_full >
TValue* mla_hash_map_get_ref (const mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key) {

    if (map.size == 0 || map.bucketCount == 0) {
        return nullptr; // Empty map cannot contain any keys
    }

    // Calculate the hash index for the key
    mla_int32_t index = Hasher::hash(key) % map.bucketCount;

    // Get the bucket at the calculated index
    mla_hash_map_bucket_t<mla_hash_map_t_param> &bucket = mla_array_list_get_unsafe(map.buckets, index);

    // Check if the key exists in the bucket
    for (mla_size_t i = 0; i < mla_array_list_size(bucket.items); ++i) {
        mla_hash_map_bucket_item_t<mla_hash_map_t_param> &item = mla_array_list_get_unsafe(bucket.items, i);

        if (item.key == key) {
            return &item.value; // Return a reference to the value
        }
    }

    return nullptr; // Key not found, return null
}

template < mla_hash_map_template_full >
void mla_hash_map_clear(mla_hash_map_t<mla_hash_map_t_param_full> &map) {

    if (map.size == 0) {
        return; // Nothing to clear
    }

    for (mla_size_t i = 0; i < map.bucketCount; ++i) {

        mla_hash_map_bucket_t<mla_hash_map_t_param> &bucket = mla_array_list_get_unsafe(map.buckets, i);
        bucket.items = mla_array_list<mla_init_struct(mla_hash_map_bucket_item_t<mla_hash_map_t_param>)>(mla_global_config_hash_map_item_default_size);

    }

    map.size = 0; // Reset the size of the hash map

}


#endif
