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
     */
    template <class Pred> struct filter {
        static constexpr auto attractor_name = "filter";

        const Pred predicate;

        explicit constexpr filter(Pred &&_predicate): predicate(_predicate) { }

        template <size_t I, class F, class X> requires std::predicate<Pred, X> constexpr void emit(X &&x, const F *flow) const {
            if constexpr (I + 1 < F::length) {
                if (predicate(std::forward<X>(x))) {
                    flow->template attractor<I + 1>().template emit<I+1, F>(std::forward<X>(x), flow);
                }
            }
        }
    };

    /**
     * Map injectively any one element type to another by means of a functor.
     */
    template <class Func> struct map {
        static constexpr auto attractor_name = "map";

        Func functor;

        explicit constexpr map(Func &&_functor): functor(_functor) { }

        template <size_t I, class F, class X> requires std::is_invocable_v<Func, X> constexpr void emit(X &&x, const F *flow) {
            if constexpr (I + 1 >= F::length || std::is_void_v<std::invoke_result_t<Func, X>>) {
                functor(std::forward<X>(x));
            } else {
                flow->template attractor<I+1>().template emit<I+1, F>(functor(std::forward<X>(x)), flow);
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
        part(const H &&_attractor): part ({ std::move(_attractor) }) { }

        template <size_t I, class F, class X> constexpr void emit(X &&x, const F *flow) const {
            subflow.template attractor<0>().template emit<0, decltype(subflow)>(std::forward<X>(x), &subflow);
            if constexpr(I + 1 < F::length) {
                flow->template attractor<I + 1>().template emit<I + 1, F>(std::forward<X>(x), flow);
            }
        }
    };

    template <class H, class ...A> struct join {
        static constexpr auto attractor_name = "join";

        alembic::flow<H, A...> subflow;

        join(const flow<H, A...> &&_subflow): subflow(std::move(_subflow)) { }
        join(const H &&_attractor): join ({ std::move(_attractor) }) { }

        template <size_t I, class F, class X> constexpr void emit(X &&x, const F *flow) const {
            auto captured = subflow >> map { [flow]<class T>(T &&t){
                if constexpr (I + 1 < F::length) {
                    flow->template attractor<I + 1>().template emit<I + 1, F>(std::forward<T>(t), flow);
                }
            } };
            captured.template attractor<0>().template emit<0, decltype(captured)>(std::forward<X>(x), &captured);
        }
    };

    /**
     * Find the next attractor in the flow that accepts the given type, and emits to that attractor specifically. The program is ill-formed
     * if a `seek` is not followed by an attractor that accepts each element emitted to the flow.
     */
    struct seek {
        static constexpr auto attractor_name = "seek";

        template <size_t I, class F, class X> constexpr void emit(X &&x, const F *flow) const {
            if constexpr (I + 1 < F::length) {
                constexpr auto target = find_next<I+1, F, X>::value;
                if constexpr (target) {
                    flow->template attractor<target>().template emit<target, F>(std::forward<X>(x), flow);
                }
            }
        }
    };

    /**
     * Like `part` but dynamic.
     * @tparam Cont the container type to use
     */
    template <class Y, class Cont = std::vector<bound_flow<Y>>> struct burst {
        static constexpr auto attractor_name = "burst";
        Cont subflows;

        template <class ...F> constexpr burst(F ..._flows): subflows({ bind_flow(_flows)... }) { }

        template <class X> requires std::is_convertible_v<X, Y> constexpr void inner_emit(X &&x) const {
            std::for_each(std::begin(subflows), std::end(subflows), [&](const bound_flow<Y> &a){ std::invoke(a.emitter, std::forward<X>(x)); });
        }

        template <size_t I, class F, class X> requires std::is_convertible_v<X, Y> constexpr void emit(X &&x, const F *flow) const {
            inner_emit(std::forward<X>(x));

            if constexpr(I + 1 < F::length) {
                flow->template attractor<I + 1>().template emit<I + 1, F>(std::forward<X>(x), flow);
            }
        }
    };

    /**
     * Collects N values convertible to Y, then passes them to the next element in the flow as an `std::array<Y, N>`.
     */
    template <class Y, size_t N> struct collect_n {
        static constexpr auto attractor_name = "collect_n";

        std::array<Y, N> values;
        size_t i = 0;

        template <size_t I, class F, class X> requires std::is_convertible_v<X, Y> inline void emit(X &&x, const F *flow) {
            values[i++] = std::forward<X>(x);
            if (i == N) {
                i = 0;
                if constexpr(I + 1 < F::length) {
                    flow->template attractor<I + 1>().template emit<I + 1, F>(values, flow);
                }
            }
        }
    };

    template <class Reducer> struct reduce {
        static constexpr auto attractor_name = "reduce";

        Reducer reducer;
        reduce(Reducer &&_reducer): reducer(_reducer) { }

        template <size_t I, class F, class X> requires std::is_invocable_v<Reducer, X> void emit(X &&x, const F *flow) {
            auto opt = std::invoke(reducer, std::forward<X>(x));
            if (I + 1 < F::length && opt.has_value()) {
                flow->template attractor<I + 1>().template emit<I + 1, F>(std::move(opt.value()), flow);
            }
        }
    };

    template <class T> concept iterable_type = requires (T t) {
        std::begin(t);
        std::end(t);
    };

    /**
     * Flattens a container type and emits each of its elements to the flow. The expressions `std::begin(v)` and `std::end(v)` must be
     * well-formed for a container `v`.
     */
    struct flat {
        static constexpr auto attractor_name = "flat";

        template <size_t I, class F, iterable_type X> constexpr void emit(X &&x, const F *flow) const {
            std::for_each(std::begin(x), std::end(x), [flow](auto v){
                if constexpr(I + 1 < F::length) {
                    flow->template attractor<I + 1>().template emit<I + 1, F>(v, flow);
                }
            });
        }
    };
}

#endif //ALEMBIC_ATTRACTORS_H
