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

#ifndef ALEMBIC_FLOW_H
#define ALEMBIC_FLOW_H

#include <cstdlib>
#include <type_traits>
#include <concepts>
#include <functional>

namespace alembic {

    template <class A> struct attractor_traits { };

    /**
     * This concept is currently unused.
     * @tparam A
     */
    template <class A> concept attractor_type = requires() {
        typename A::input_t;
        typename A::output_t;
    } || requires {
        typename attractor_traits<A>;
    };

    template <attractor_type A> struct attractor_traits<A> {
        using attractor_t = A;
        using input_t = typename A::input_t;
        using output_t = typename A::output_t;
    };

    template <class A, class X, class F, size_t I, class = void> struct attractor_takes: std::false_type { };
    template <class A, class X, class F, size_t I> struct attractor_takes<A, X, F, I, std::enable_if_t<std::is_invocable_v<decltype(&A::template emit<I, F>), A&, X, F*>>>: std::true_type { };
    template <class A, class X, class F, size_t I> struct attractor_takes<A, X, F, I, std::enable_if_t<std::is_invocable_v<decltype(&A::template emit<I, F, X>), A&, X, F*>>>: std::true_type { };

    /**
     * Determine if an attractor would accept an element at a specific type in a given flow
     * @tparam A the attractor type
     * @tparam X the element type
     * @tparam F the flow type
     * @tparam I the index at which the attractor occurs in the flow
     */
    template <class A, class X, class F, size_t I> constexpr bool attractor_takes_v = attractor_takes<A, X, F, I>::value;

    /**
     * Represents a sequence of attractors
     * @tparam A the types of attractors
     */
    template <attractor_type ...A> struct flow {
        using flow_types = std::tuple<A...>;

        constexpr static size_t length = sizeof...(A);

        const std::tuple<A...> attractors;

        constexpr flow(A ..._attractors): attractors(_attractors...) { }
        constexpr flow(std::tuple<A...> _attractors): attractors(_attractors) { }

        constexpr ~flow() = default;

        /**
         * Add an attractor to the end of the flow
         * @tparam R the type of attractor to add
         * @param r the instance of the attractor
         * @return a new flow
         */
        template <attractor_type R> constexpr flow<A..., R> adjoin(const R &&r) {
            return flow<A..., R>(std::tuple_cat(attractors, std::make_tuple(std::move(r))));
        }

        template <attractor_type R> constexpr flow<A..., R> operator>>(const R &&r) {
            return adjoin(std::move(r));
        }

        /**
         * Get an attractor from the flow
         * @tparam I the index at which the attractor occurs
         * @return an attractor reference
         */
        template <size_t I> constexpr auto attractor() const {
            return std::get<I>(attractors);
        }

    };

    /**
     * Determine if a general attractor with default parameters would accept the given element. Delegates to `attractor_takes_v`
     * and assumes a singleton flow.
     * @tparam A the attractor type
     * @tparam X the element type
     */
    template <class A, class X> constexpr bool attractor_default_takes_v = attractor_takes_v<A, X, alembic::flow<X>, 0>;

    template <attractor_type L, attractor_type R> constexpr flow<L, R> operator>>(const L &&l, const R &&r) {
        return flow(l, r);
    }
}

#endif //ALEMBIC_FLOW_H
