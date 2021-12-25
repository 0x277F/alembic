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

#ifndef ALEMBIC_ATTRACTORS_H
#define ALEMBIC_ATTRACTORS_H

#include "flow.h"

namespace alembic {
    /**
     * Filter elements based on a predicate.
     * @tparam X the type of elements to filter.
     */
    template <class X> struct filter {
        static constexpr auto attractor_name = "filter";

        const std::function<bool(X)> predicate;

        template <size_t I, class F> constexpr void emit(const X x, const F *flow) const {
            if constexpr (I + 1 < F::length) {
                if (predicate(std::move(x))) {
                    flow->template attractor<I + 1>().template emit<I+1, F>(std::move(x), flow);
                }
            }
        }
    };

    /**
     * Map one element type to another using a `static_cast`. `static_map` should only be necessary in the case of
     * explicit-only conversions.
     * @tparam X the type to convert from
     * @tparam Y the type to convert to
     */
    template <class X, class Y> struct static_map {
        static constexpr auto attractor_name = "static_map";

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
        static constexpr auto attractor_name = "map";
        using functor_t = Y(X);

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
        static constexpr auto attractor_name = "tap";
        using functor_t = void(X);

        std::function<functor_t> functor;

        template <size_t I, class F> constexpr void emit(const X x, const F *flow) const {
            functor(std::move(x));
            if constexpr (I + 1 < F::length) {
                flow->template attractor<I+1>().template emit<I+1, F>(std::move(x), flow);
            }
        }
    };

    /**
     * Partition a flow, emitting an element to a subflow then discarding the result and passing the original element to the
     * next attractor.
     * @tparam H the first attractor in the subflow
     * @tparam A trailing attractor types in the subflow
     */
    template <class H, class ...A> struct part {
        static constexpr auto attractor_name = "part";

        alembic::flow<H, A...> subflow;

        part(const flow<H, A...> &&_subflow): subflow(std::move(_subflow)) { }
        part(const H &&_attractor): subflow { std::move(_attractor) } { }

        template <size_t I, class F, class X> constexpr void emit(const X &&x, const F *flow) const {
            subflow.template attractor<0>().template emit<0, decltype(subflow)>(x, &subflow);
            if constexpr(I + 1 < F::length) {
                flow->template attractor<I + 1>().template emit<I + 1, F>(std::move(x), flow);
            }
        }
    };

    /**
     * Find the next attractor in the flow that accepts the given type, and emits to that attractor specifically. The program is ill-formed
     * if a `seek` is not followed by an attractor that accepts each element emitted to the flow.
     */
    struct seek {
        static constexpr auto attractor_name = "seek";

        template <size_t I, class F, class X, class = void> struct find_next: std::enable_if_t<I+1 < F::length, find_next<I+1, F, X>> { };
        template <size_t I, class F, class X> struct find_next<I, F, X, std::enable_if_t<alembic::attractor_takes_v<std::tuple_element_t<I, typename F::flow_types>, X, F, I>>>: std::integral_constant<size_t, I> { };

        template <size_t I, class F, class X> constexpr void emit(const X x, const F *flow) const {
            if constexpr (I + 1 < F::length) {
                constexpr size_t target = find_next<I+1, F, X>::value;
                flow->template attractor<target>().template emit<target, F>(std::move(x), flow);
            }
        }
    };

    /**
     * Like `part` but dynamic.
     * @tparam Cont the container type to use
     */
    template <class X, class Cont = std::vector<std::function<bound_flow_t<X>>>> struct burst {
        static constexpr auto attractor_name = "burst";
        Cont subflows;

        template <class ...F> constexpr burst(F ..._flows): subflows({ bind_flow(_flows)... }) { }

        inline void inner_emit(X x) const {
            std::for_each(std::begin(subflows), std::end(subflows), [&](auto a){ std::invoke(a, x); });
        }

        template <size_t I, class F> constexpr void emit(const X x, const F *flow) const {
            inner_emit(x);

            if constexpr(I + 1 < F::length) {
                flow->template attractor<I + 1>().template emit<I + 1, F>(std::move(x), flow);
            }
        }
    };
}

#endif //ALEMBIC_ATTRACTORS_H
