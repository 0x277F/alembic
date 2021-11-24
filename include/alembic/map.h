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

#ifndef ALEMBIC_MAP_H
#define ALEMBIC_MAP_H

#include "flow.h"

namespace alembic {
    /**
     * Map one element type to another using a `static_cast`. `static_map` should only be necessary in the case of
     * explicit-only conversions.
     * @tparam X the type to convert from
     * @tparam Y the type to convert to
     */
    template <class X, class Y> struct static_map {
        using input_t = X;
        using output_t = Y;

        template <size_t I, class F> constexpr void emit(const X x, const F *flow) const {
            if constexpr(I + 1 < F::length) {
                flow->template attractor<I+1>().template emit<I+1, F>(static_cast<Y>(std::move(x)), flow);
            }
        }
    };

    /**
     * Map injectively any one element type to another by means of a functor.
     * @tparam X the type to convert from
     * @tparam Y the type to convert to
     */
    template <class X, class Y> struct map {
        using functor_t = Y(X);
        using input_t = X;
        using output_t = Y;

        std::function<functor_t> functor;

        template <size_t I, class F> constexpr void emit(const X x, const F *flow) const {
            if constexpr (I + 1 >= F::length || std::is_void_v<Y>) {
                functor(std::move(x));
            } else {
                flow->template attractor<I+1>().template emit<I+1, F>(functor(std::move(x)), flow);
            }
        }
    };

    /**
     * Apply the given functor to every element statically assignable to the given type.
     * @tparam X the type
     */
    template <class X> struct tap {
        using functor_t = void(X);
        using input_t = X;
        using output_t = X;

        std::function<functor_t> functor;

        template <size_t I, class F> constexpr void emit(const X x, const F *flow) const {
            functor(std::move(x));
            if constexpr (I + 1 < F::length) {
                flow->template attractor<I+1>().template emit<I+1, F>(std::move(x), flow);
            }
        }
    };
}

#endif //ALEMBIC_MAP_H
