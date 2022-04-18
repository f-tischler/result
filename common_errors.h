//
// Created by flori on 09.04.2022.
//

#ifndef ERRORHANDLING_COMMON_ERRORS_H
#define ERRORHANDLING_COMMON_ERRORS_H

#include "define_error.h"

namespace basic_errors
{
    DEFINE_ERROR_CATEGORY(1, basic_error_category);
    DEFINE_ERROR_CODE(1, basic_error_category, propagated_error, "Propagated error");
}

namespace assertion_errors
{
    DEFINE_ERROR_CATEGORY(2, assertion_category);
    DEFINE_ERROR_CODE(1, assertion_category, precondition_error, "Pre-condition failed");
    DEFINE_ERROR_CODE(2, assertion_category, postcondition_error, "Post-condition failed");
}

#endif //ERRORHANDLING_COMMON_ERRORS_H
