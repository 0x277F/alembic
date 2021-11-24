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

#ifndef ALEMBIC_SEEK_H
#define ALEMBIC_SEEK_H

namespace alembic {
    /**
     * Find the next attractor in the flow that accepts the given type, and emits to that attractor specifically. The program is ill-formed
     * if a `seek` is not followed by an attractor that accepts each element emitted to the flow.
     */
    struct seek {
        template <size_t I, class F, class X, class = void> struct find_next: std::enable_if_t<I+1 < F::length, find_next<I+1, F, X>> { };
        template <size_t I, class F, class X> struct find_next<I, F, X, std::enable_if_t<alembic::attractor_takes_v<std::tuple_element_t<I, typename F::flow_types>, X, F, I>>>: std::integral_constant<size_t, I> { };

        template <size_t I, class F, class X> constexpr void emit(const X x, const F *flow) const {
            if constexpr (I + 1 < F::length) {
                constexpr size_t target = find_next<I+1, F, X>::value;
                flow->template attractor<target>().template emit<target, F>(std::move(x), flow);
            }
        }
    };
}

#endif //ALEMBIC_SEEK_H
