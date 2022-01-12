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
     * Structs must satisfy this concept in order to be used as an attractor
     */
    template <class A> concept attractor_type = requires {
        { A::attractor_name } -> std::convertible_to<const char *>;
    } || requires {
        { attractor_traits<A>::attractor_name } -> std::convertible_to<const char *>;
    };

    template <attractor_type A, class X, class F, size_t I, class = void> struct attractor_takes: std::false_type { };
    template <attractor_type A, class X, class F, size_t I> struct attractor_takes<A, X, F, I, std::enable_if_t<std::is_invocable_v<decltype(&A::template emit<I, F>), A&, X&&, F*>>>: std::true_type { };
    template <attractor_type A, class X, class F, size_t I> struct attractor_takes<A, X, F, I, std::enable_if_t<std::is_invocable_v<decltype(&A::template emit<I, F, X>), A&, X&&, F*>>>: std::true_type { };

    /**
     * Determine if an attractor would accept an element at a specific type in a given flow
     * @tparam A the attractor type
     * @tparam X the element type
     * @tparam F the flow type
     * @tparam I the index at which the attractor occurs in the flow
     */
    template <attractor_type A, class X, class F, size_t I> constexpr bool attractor_takes_v = attractor_takes<A, X, F, I>::value;

    template <size_t I, class F, class X, class = void> struct find_next: std::conditional_t<I+1 < F::length, find_next<I+1, F, X>, std::bool_constant<false>> { };
    template <size_t I, class F, class X> struct find_next<I, F, X, std::enable_if_t<alembic::attractor_takes_v<std::tuple_element_t<I, typename F::flow_types>, X, F, I>>>: std::integral_constant<size_t, I> { };

    template <size_t I, class F, class X, class = void> struct find_prev: std::conditional_t<I-1 >= 0, find_prev<I-1, F, X>, std::bool_constant<false>> { };
    template <size_t I, class F, class X> struct find_prev<I, F, X, std::enable_if_t<alembic::attractor_takes_v<std::tuple_element_t<I, typename F::flow_types>, X, F, I>>>: std::integral_constant<size_t, I> { };

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
    template <attractor_type A, class X> constexpr bool attractor_default_takes_v = attractor_takes_v<A, X, ::alembic::flow<A>, 0>;

    template <attractor_type L, attractor_type R> constexpr flow<L, R> operator>>(const L &&l, const R &&r) {
        return flow(l, r);
    }

    using removal_tag_t = void *;

    template <class X> struct bound_flow {
        std::function<void(X)> emitter;
        removal_tag_t remove_tag;
    };

    /**
     * Bind the emit function at the head of the given flow. This can be used to pass around a function pointer instead
     * of the templated `flow`.
     * @tparam A attractor types
     * @param flow the flow to bind
     * @return a function callable with the single argument corresponding to the `x` parameter of the flow's first attractor
     */
    template <class X, attractor_type ...A> constexpr bound_flow<X> bind_flow(flow<A...> &flow, removal_tag_t remove_tag = nullptr) {
        constexpr size_t index = find_next<0, ::alembic::flow<A...>, X>::value;
        using Att = std::tuple_element_t<index, typename ::alembic::flow<A...>::flow_types>;
        return {
                std::bind(&Att::template emit<index, ::alembic::flow<A...>, X>,
                          flow.template attractor<index>(),
                          std::placeholders::_1,
                          &flow),
                remove_tag
        };
    }
}

#endif //ALEMBIC_FLOW_H
