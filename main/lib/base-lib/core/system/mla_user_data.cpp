//
// Created by chris on 2/9/2026.
//

#include "mla_user_data.h"

#include "mla_array_list.h"
#include "mla_string_concat.h"
#include "../mla_data_types.h"
#include "../log/mla_logging.h"

static const mla_user_data_id mla_user_data_name_empty = 0;
static mla_user_data_id mla_user_data_name_nextId = 1; // Start from 2 since 0 and 1 are reserved for empty and list

struct mla_user_data_item_t {
    mla_user_data_id id;
    mla_pointer_t external_data;
    mla_dynamic_data_t data;

    static mla_user_data_item_t init() {
        return {
            mla_user_data_name_empty,
            mla_pointer_null(),
            mla_dynamic_data_empty()
        };
    }
};

mla_user_data_item_t mla_user_data_item_empty() {
    return {
        mla_user_data_name_empty,
        mla_pointer_null(),
        mla_dynamic_data_empty()
    };
}

struct mla_user_data_list_t {

    mla_array_list_t<mla_init_struct(mla_user_data_item_t)> datas;

    static mla_user_data_list_t init() {
        return {mla_array_list_empty<mla_init_struct(mla_user_data_item_t)>()};
    }
};

mla_user_data_id mla_get_next_user_data_id() {
    return mla_user_data_name_nextId++;
}

mla_user_data_t mla_user_data_empty() {
    mla_user_data_t data = {
        mla_pointer_null()
    };

    return data;
}

mla_string_t mla_private_string_from_mla_user_data_id(mla_user_data_id id) {
    return mla_string_from_uint16(id);
}

mla_user_data_item_t* mla_private_user_data_get_for_update(mla_user_data_t &target, mla_user_data_id id) {

    if (mla_pointer_is_null(target.data)) {
        target.data = mla_malloc_struct(mla_user_data_list_t);
    }

    mla_user_data_list_t* list = mla_pointer_get_data<mla_user_data_list_t>(target.data);

    if (list == nullptr) {
        mla_error(mla_string_concat("User data list is null for name: ", mla_private_string_from_mla_user_data_id(id)));
        return nullptr;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(list->datas); ++i) {

        mla_user_data_item_t* item = mla_array_list_get_ref_unsafe(list->datas, i);
        if (item->id == id) {
            return item;
        }
    }

    mla_user_data_item_t newItem = mla_user_data_item_empty();

    if (!mla_array_list_add(list->datas, newItem)) {
        mla_error(mla_string_concat("Failed to add user data to list for name: ", mla_private_string_from_mla_user_data_id(id)));
        return nullptr;
    }

    return mla_array_list_get_ref_unsafe(list->datas, mla_array_list_size(list->datas) - 1);

}

mla_bool_t mla_user_data_remove(mla_user_data_t& target, mla_user_data_id id) {

    mla_user_data_list_t* list = mla_pointer_get_data<mla_user_data_list_t>(target.data);

    if (list == nullptr) {
        return false; // List is null, cannot remove
    }


    for (mla_size_t i = 0; i < mla_array_list_size(list->datas); ++i) {

        mla_user_data_item_t* item = mla_array_list_get_ref_unsafe(list->datas, i);
        if (item->id == id) {
            mla_array_list_remove(list->datas, i);
            return true;
        }
    }

    return false; // Not found
}

mla_bool_t mla_user_data_set_pointer(mla_user_data_t& target, mla_user_data_id id, mla_pointer_t& data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = data;
    user_data->data = mla_dynamic_data_empty();

    return true;

}

mla_bool_t mla_user_data_set_int8(mla_user_data_t &target, mla_user_data_id id,
                                  mla_int8_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asInt8 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint8(mla_user_data_t& target, mla_user_data_id id, mla_uint8_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asUint8 = data;
    return true;

}

mla_bool_t mla_user_data_set_int16(mla_user_data_t& target, mla_user_data_id id, mla_int16_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asInt16 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint16(mla_user_data_t& target, mla_user_data_id id, mla_uint16_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asUint16 = data;
    return true;

}

mla_bool_t mla_user_data_set_int32(mla_user_data_t& target, mla_user_data_id id, mla_int32_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asInt32 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint32(mla_user_data_t& target, mla_user_data_id id, mla_uint32_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asUint32 = data;
    return true;

}

mla_bool_t mla_user_data_set_int64(mla_user_data_t& target, mla_user_data_id id, mla_int64_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asInt64 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint64(mla_user_data_t& target, mla_user_data_id id, mla_uint64_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asUint64 = data;
    return true;

}

mla_bool_t mla_user_data_set_float(mla_user_data_t& target, mla_user_data_id id, mla_float_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asFloat = data;
    return true;

}

mla_bool_t mla_user_data_set_double(mla_user_data_t& target, mla_user_data_id id, mla_double_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asDouble = data;
    return true;

}

mla_bool_t mla_user_data_set_bool(mla_user_data_t& target, mla_user_data_id id, mla_bool_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asBool = data;
    return true;

}

mla_bool_t mla_user_data_set_char(mla_user_data_t& target, mla_user_data_id id, mla_char_t data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->external_data = mla_pointer_null();
    user_data->data.asChar = data;
    return true;

}

mla_bool_t mla_user_data_set_string(mla_user_data_t& target, mla_user_data_id id, mla_string_t& data) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;

    mla_c_string_t c_string = mla_string_to_cString(data);
    user_data->external_data = c_string.c_heap_str;
    return true;
}

mla_bool_t mla_user_data_inc_int8(mla_user_data_t& target, mla_user_data_id id, mla_int8_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt8 = mla_s_cast<mla_int8_t>(user_data->data.asInt8 + step);
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt8 = step;
    }

    return true;

}
mla_bool_t mla_user_data_inc_uint8(mla_user_data_t& target, mla_user_data_id id, mla_uint8_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint8 = user_data->data.asUint8 + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint8 = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_int16(mla_user_data_t& target, mla_user_data_id id, mla_int16_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt16 = mla_s_cast<mla_int16_t>(user_data->data.asInt16 + step);
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt16 = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_uint16(mla_user_data_t& target, mla_user_data_id id, mla_uint16_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint16 = user_data->data.asUint16 + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint16 = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_int32(mla_user_data_t& target, mla_user_data_id id, mla_int32_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt32 = user_data->data.asInt32 + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt32 = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_uint32(mla_user_data_t& target, mla_user_data_id id, mla_uint32_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint32 = user_data->data.asUint32 + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint32 = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_int64(mla_user_data_t& target, mla_user_data_id id, mla_int64_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt64 = user_data->data.asInt64 + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt64 = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_uint64(mla_user_data_t& target, mla_user_data_id id, mla_uint64_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint64 = user_data->data.asUint64 + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint64 = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_float(mla_user_data_t& target, mla_user_data_id id, mla_float_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asFloat = user_data->data.asFloat + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asFloat = step;
    }

    return true;

}

mla_bool_t mla_user_data_inc_double(mla_user_data_t& target, mla_user_data_id id, mla_double_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asDouble = user_data->data.asDouble + step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asDouble = step;
    }

    return true;

}

mla_bool_t mla_user_data_dec_int8(mla_user_data_t& target, mla_user_data_id id, mla_int8_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt8 = mla_s_cast<mla_int8_t>(user_data->data.asInt8 - step);
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt8 = mla_s_cast<mla_int8_t>(-step);
    }

    return true;

}

mla_bool_t mla_user_data_dec_uint8(mla_user_data_t& target, mla_user_data_id id, mla_uint8_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint8 = user_data->data.asUint8 - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint8 = 0;
    }

    return true;

}

mla_bool_t mla_user_data_dec_int16(mla_user_data_t& target, mla_user_data_id id, mla_int16_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt16 = mla_s_cast<mla_int16_t>(user_data->data.asInt16 - step);
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt16 = mla_s_cast<mla_int16_t>(-step);
    }

    return true;

}

mla_bool_t mla_user_data_dec_uint16(mla_user_data_t& target, mla_user_data_id id, mla_uint16_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint16 = user_data->data.asUint16 - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint16 = 0;
    }

    return true;

}

mla_bool_t mla_user_data_dec_int32(mla_user_data_t& target, mla_user_data_id id, mla_int32_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt32 = user_data->data.asInt32 - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt32 = -step;
    }

    return true;

}

mla_bool_t mla_user_data_dec_uint32(mla_user_data_t& target, mla_user_data_id id, mla_uint32_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint32 = user_data->data.asUint32 - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint32 = 0;
    }

    return true;

}

mla_bool_t mla_user_data_dec_int64(mla_user_data_t& target, mla_user_data_id id, mla_int64_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asInt64 = user_data->data.asInt64 - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asInt64 = -step;
    }

    return true;

}

mla_bool_t mla_user_data_dec_uint64(mla_user_data_t& target, mla_user_data_id id, mla_uint64_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asUint64 = user_data->data.asUint64 - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asUint64 = 0;
    }

    return true;

}

mla_bool_t mla_user_data_dec_float(mla_user_data_t& target, mla_user_data_id id, mla_float_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asFloat = user_data->data.asFloat - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asFloat = -step;
    }

    return true;

}

mla_bool_t mla_user_data_dec_double(mla_user_data_t& target, mla_user_data_id id, mla_double_t step) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    if (user_data->id == id) {
        user_data->data.asDouble = user_data->data.asDouble - step;
    } else {
        user_data->id = id;
        user_data->external_data = mla_pointer_null();
        user_data->data.asDouble = -step;
    }

    return true;

}

mla_bool_t mla_user_data_set_native_resource(mla_user_data_t& target, mla_user_data_id id, mla_native_resource_t resource, mla_native_resource_clean_up_hook_t clean_up_hook) {

    mla_user_data_item_t* user_data = mla_private_user_data_get_for_update(target, id);

    if (user_data == nullptr) {
        return false;
    }

    user_data->id = id;
    user_data->data = mla_dynamic_data_empty();
    user_data->external_data = mla_native_resource_to_managed_pointer(resource, clean_up_hook);

    return true;
}

mla_user_data_item_t* mla_private_user_find_item(mla_user_data_list_t* list, mla_user_data_id id) {

    if (list == nullptr) {
        return nullptr;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(list->datas); ++i) {

        mla_user_data_item_t* item = mla_array_list_get_ref_unsafe(list->datas, i);

        if (item->id == id) {
            return item;
        }
    }

    return nullptr;
}

mla_user_data_item_t* mla_user_data_get(mla_user_data_t& data, mla_user_data_id id) {

    mla_user_data_list_t* list = mla_pointer_get_data<mla_user_data_list_t>(data.data);

    return mla_private_user_find_item(list, id);
}

mla_bool_t mla_user_data_equal(const mla_user_data_t& a, const mla_user_data_t& b) {

    if (mla_pointer_is_null(a.data) && mla_pointer_is_null(b.data)) {
        return true;
    }

    mla_user_data_list_t* list_a = mla_pointer_get_data<mla_user_data_list_t>(a.data);
    mla_user_data_list_t* list_b = mla_pointer_get_data<mla_user_data_list_t>(b.data);

    if (list_a == list_b) {
        return true;
    }

    if (list_a == nullptr || list_b == nullptr) {
        return false;
    }

    // check the count of the list
    if (mla_array_list_size(list_a->datas) != mla_array_list_size(list_b->datas)) {
        return false;
    }

    // Compare items

    for (mla_size_t i = 0; i < mla_array_list_size(list_a->datas); ++i) {

        mla_user_data_item_t* item_a = mla_array_list_get_ref_unsafe(list_a->datas, i);
        mla_user_data_item_t* item_b = mla_private_user_find_item(list_b, item_a->id);

        if (item_b == nullptr) {
            return false;
        }

        if (mla_memcmp(&item_a->data, &item_b->data, sizeof(mla_dynamic_data_t)) != 0) {
            return false;
        }
    }


    return true;

}

mla_pointer_t mla_user_data_get_pointer(const mla_user_data_t& userData, mla_user_data_id id) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);

    if (found == nullptr) {
        return mla_pointer_null();
    }

    return found->external_data;
}

mla_int8_t mla_user_data_get_int8(const mla_user_data_t& userData, mla_user_data_id id, mla_int8_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt8;
}

mla_uint8_t mla_user_data_get_uint8(const mla_user_data_t& userData, mla_user_data_id id, mla_uint8_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint8;
}

mla_int16_t mla_user_data_get_int16(const mla_user_data_t& userData, mla_user_data_id id, mla_int16_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt16;
}

mla_uint16_t mla_user_data_get_uint16(const mla_user_data_t& userData, mla_user_data_id id, mla_uint16_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint16;
}

mla_int32_t mla_user_data_get_int32(const mla_user_data_t& userData, mla_user_data_id id, mla_int32_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt32;
}

mla_uint32_t mla_user_data_get_uint32(const mla_user_data_t& userData, mla_user_data_id id, mla_uint32_t defaultValue) {;

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint32;
}

mla_int64_t mla_user_data_get_int64(const mla_user_data_t& userData, mla_user_data_id id, mla_int64_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt64;
}

mla_uint64_t mla_user_data_get_uint64(const mla_user_data_t& userData, mla_user_data_id id, mla_uint64_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint64;
}

mla_float_t mla_user_data_get_float(const mla_user_data_t& userData, mla_user_data_id id, mla_float_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asFloat;
}

mla_double_t mla_user_data_get_double(const mla_user_data_t& userData, mla_user_data_id id, mla_double_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asDouble;
}

mla_bool_t mla_user_data_get_bool(const mla_user_data_t& userData, mla_user_data_id id, mla_bool_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asBool;
}

mla_string_t mla_user_data_get_string(const mla_user_data_t& userData, mla_user_data_id id, mla_string_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);

    if (found == nullptr) {
        return defaultValue;
    }

    mla_char_t* str_data = mla_pointer_get_data<mla_char_t>(found->external_data);

    if (str_data == nullptr) {
        return defaultValue;
    }

    return mla_string(found->external_data, mla_strlen(str_data));
}

mla_char_t mla_user_data_get_char(const mla_user_data_t& userData, mla_user_data_id id, mla_char_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asChar;
}

mla_int8_t mla_user_data_get_and_replace_int8(const mla_user_data_t& userData, mla_user_data_id id, mla_int8_t newValue, mla_int8_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_int8_t result;

    if (found->id == id) {
        result = found->data.asInt8;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asInt8 = newValue;
    return result;

}

mla_uint8_t mla_user_data_get_and_replace_uint8(const mla_user_data_t& userData, mla_user_data_id id, mla_uint8_t newValue, mla_uint8_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_uint8_t result;

    if (found->id == id) {
        result = found->data.asUint8;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asUint8 = newValue;
    return result;

}

mla_int16_t mla_user_data_get_and_replace_int16(const mla_user_data_t& userData, mla_user_data_id id, mla_int16_t newValue, mla_int16_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_int16_t result;

    if (found->id == id) {
        result = found->data.asInt16;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asInt16 = newValue;
    return result;

}

mla_uint16_t mla_user_data_get_and_replace_uint16(const mla_user_data_t& userData, mla_user_data_id id, mla_uint16_t newValue, mla_uint16_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_uint16_t result;

    if (found->id == id) {
        result = found->data.asUint16;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asUint16 = newValue;
    return result;

}

mla_int32_t mla_user_data_get_and_replace_int32(const mla_user_data_t& userData, mla_user_data_id id, mla_int32_t newValue, mla_int32_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_int32_t result;

    if (found->id == id) {
        result = found->data.asInt32;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asInt32 = newValue;
    return result;

}

mla_uint32_t mla_user_data_get_and_replace_uint32(const mla_user_data_t& userData, mla_user_data_id id, mla_uint32_t newValue, mla_uint32_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_uint32_t result;

    if (found->id == id) {
        result = found->data.asUint32;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asUint32 = newValue;
    return result;

}

mla_int64_t mla_user_data_get_and_replace_int64(const mla_user_data_t& userData, mla_user_data_id id, mla_int64_t newValue, mla_int64_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_int64_t result;

    if (found->id == id) {
        result = found->data.asInt64;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asInt64 = newValue;
    return result;

}

mla_uint64_t mla_user_data_get_and_replace_uint64(const mla_user_data_t& userData, mla_user_data_id id, mla_uint64_t newValue, mla_uint64_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_uint64_t result;

    if (found->id == id) {
        result = found->data.asUint64;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asUint64 = newValue;
    return result;

}
mla_float_t mla_user_data_get_and_replace_float(const mla_user_data_t& userData, mla_user_data_id id, mla_float_t newValue, mla_float_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_float_t result;

    if (found->id == id) {
        result = found->data.asFloat;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asFloat = newValue;
    return result;

}

mla_double_t mla_user_data_get_and_replace_double(const mla_user_data_t& userData, mla_user_data_id id, mla_double_t newValue, mla_double_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_double_t result;

    if (found->id == id) {
        result = found->data.asDouble;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asDouble = newValue;
    return result;

}

mla_bool_t mla_user_data_get_and_replace_bool(const mla_user_data_t& userData, mla_user_data_id id, mla_bool_t newValue, mla_bool_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_bool_t result;

    if (found->id == id) {
        result = found->data.asBool;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asBool = newValue;
    return result;

}

mla_char_t mla_user_data_get_and_replace_char(const mla_user_data_t& userData, mla_user_data_id id, mla_char_t newValue, mla_char_t defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }
    mla_char_t result;

    if (found->id == id) {
        result = found->data.asChar;
    } else {
        found->id = id;
        found->external_data = mla_pointer_null();
        result = defaultValue;
    }

    found->data.asChar = newValue;
    return result;

}

mla_string_t mla_user_data_get_and_replace_string(const mla_user_data_t& userData, mla_user_data_id id, const mla_string_t& newValue, const mla_string_t& defaultValue) {

    mla_user_data_item_t* found = mla_private_user_data_get_for_update(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        // Update failed, return default value
        return defaultValue;
    }

    mla_string_t result = mla_string_empty();

    if (found->id == id) {

        mla_char_t* str_data = mla_pointer_get_data<mla_char_t>(found->external_data);

        if (str_data != nullptr) {
            result = mla_string(found->external_data, mla_strlen(str_data));
        }

    } else {

        found->id = id;
        result = defaultValue;
    }

    // Now replace with new value
    mla_c_string_t c_new_string = mla_string_to_cString(newValue);
    found->external_data = c_new_string.c_heap_str;
    return result;

}


mla_native_resource_t mla_user_data_get_native_resource(const mla_user_data_t& userData, mla_user_data_id id, mla_native_resource_t defaultValue) {

    const mla_user_data_item_t* found = mla_user_data_get(mla_c_cast<mla_user_data_t &>(userData), id);
    if (found == nullptr) {
        return defaultValue;
    }

    mla_native_resource_t* resource = mla_native_resource_from_managed_pointer(found->external_data);

    if (resource == nullptr) {
        return defaultValue;
    }

    return *resource;
}


mla_user_data_t mla_user_data_copy(const mla_user_data_t& other) {

    mla_user_data_t copy = mla_user_data_empty();

    mla_user_data_list_t* otherList = mla_pointer_get_data<mla_user_data_list_t>(other.data);

    if (otherList == nullptr) {
        return copy; // No data to copy
    }

    mla_user_data_list_t* newList = mla_pointer_get_data<mla_user_data_list_t>(copy.data);

    if (newList == nullptr) {
        return copy;
    }

    // copy
    mla_array_list_add_all(newList->datas, otherList->datas);
    return copy;
}