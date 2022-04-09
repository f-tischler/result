//
// Created by flori on 09.04.2022.
//

#ifndef ERRORHANDLING_DEFINE_ERROR_H
#define ERRORHANDLING_DEFINE_ERROR_H

#include "error.h"

#define DEFINE_ERROR_CATEGORY(id, name) \
    struct name : error_category_base<id>\
    {\
        constexpr name()\
            : error_category_base<id>(#name) {}\
    }

#define DEFINE_ERROR_CODE(id, category, name, description) \
    struct name : error_code_base<id, category>\
    {                           \
        constexpr name() : error_code_base<id, category>(#name, description) {}\
    }

#endif //ERRORHANDLING_DEFINE_ERROR_H
