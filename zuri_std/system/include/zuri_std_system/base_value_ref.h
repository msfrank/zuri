/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_BASE_VALUE_REF_H
#define ZURI_STD_SYSTEM_BASE_VALUE_REF_H

#include <lyric_runtime/base_ref.h>

enum class ValueType {
    Invalid,
    Attr,
    Element,
};

class BaseValueRef : public lyric_runtime::BaseRef {
public:
    explicit BaseValueRef(const lyric_runtime::VirtualTable *vtable)
        : lyric_runtime::BaseRef(vtable)
    {};

    /**
     *
     * @return
     */
    virtual ValueType getValueType() const = 0;
};

#endif // ZURI_STD_SYSTEM_BASE_VALUE_REF_H
