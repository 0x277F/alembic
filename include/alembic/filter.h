/*
 * Copyright 2021 Kioshi Morosin <hex@hex.lc>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ALEMBIC_FILTER_H
#define ALEMBIC_FILTER_H

#include "flow.h"

namespace alembic {
    /**
     * Filter elements based on a predicate.
     * @tparam X the type of elements to filter.
     */
    template <class X> struct filter {
        using input_t = X;
        using output_t = X;

        const std::function<bool(X)> predicate;

        template <size_t I, class F> constexpr void emit(const X x, const F *flow) const {
            if constexpr (I + 1 < F::length) {
                if (predicate(std::move(x))) {
                    flow->template attractor<I + 1>().template emit<I+1, F>(std::move(x), flow);
                }
            }
        }
    };
}

#endif //ALEMBIC_FILTER_H
