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

#ifndef ALEMBIC_FLUX_H
#define ALEMBIC_FLUX_H

#include <exception>
#include <tuple>
#include "attractors_builtin.h"

namespace alembic {

    /**
     * Represents the point at which elements might be emitted to a flow.
     * @tparam X the type of element being admitted to the head of the flow
     */
    template <class ...X> class flux {
        std::tuple<burst<X>...> main_burst;
        burst<const std::exception_ptr> exception_burst;

        template <class From, class T, class ...U> struct first_type_convertible: std::conditional_t<std::is_convertible_v<From, T>, std::type_identity<T>, first_type_convertible<From, U...>> { };
        template <class From, class T> struct first_type_convertible<From, T>: std::enable_if_t<std::is_convertible_v<From, T>, std::type_identity<T>> { };

    public:
        /**
         * Emit an element to all attached flows.
         * @param x the element ot emit
         * @return the same flux
         */
        template <class T> const flux<X...> &emit(T &&t) const {
            static_assert((std::is_convertible_v<T, X> || ...), "cannot emit type from flux");

            using burst_type_t = typename first_type_convertible<T, X...>::type;
            try {
                std::get<burst<burst_type_t>>(main_burst).inner_emit(std::forward<T>(t));
            } catch (...) {
                exception_burst.inner_emit(std::current_exception());
            }
            return *this;
        }

        /**
         * Attaches the given flow to the flux.
         * @param flow the flow to attach
         * @param remove_tag a pointer in which a remove tag that can be used as a parameter to `detach` may be returned.
         * @return the same flux
         */
        template <attractor_type ...A> flux<X...> &attach(flow<A...> flow, removal_tag_t *remove_tag = nullptr) {
            auto tag = new removal_tag_t;

            (std::get<burst<X>>(main_burst).subflows.push_back(bind_flow<X>(flow, tag)), ...);

            if (remove_tag) {
                *remove_tag = tag;
            }
            return *this;
        }

        template <attractor_type A> flux<X...> &attach(A attractor, removal_tag_t *remove_tag = nullptr) {
            return attach(std::move(flow(attractor)), remove_tag);
        }

        /**
         * Attaches a flow to the exception handling chain. Any uncaught exception thrown during an `emit` will be caught and
         * passed as an `std::exception_ptr` to this flow.
         * @param flow the flow to attach
         * @tparam Ex exception type to catch.
         * @return the same flux
         */
        template <attractor_type ...A> flux<X...> &except(flow<A...> flow, removal_tag_t *remove_tag = nullptr) {
            auto tag = new removal_tag_t;
            exception_burst.subflows.push_back(bind_flow<const std::exception_ptr>(flow, tag));
            if (remove_tag) {
                *remove_tag = tag;
            }
            return *this;
        }

        template <attractor_type A> flux<X...> &except(A attractor, removal_tag_t *remove_tag = nullptr) {
            return except(std::move(flow(attractor)), remove_tag);
        }

        /**
         * Detaches a flow from the flux.
         * @param remove_tag an identifier returned in a call to `attach`
         * @return the same flux
         */
        flux<X...> &detach(removal_tag_t &remove_tag) {
            (std::erase_if(std::get<burst<X>>(main_burst).subflows, [remove_tag]<class U>(bound_flow<U> f){ return remove_tag == f.remove_tag; }), ...);
            return *this;
        }

        /**
         * Detaches a flow from the exception handling chain.
         * @param remove_tag an identifier returned in a call to `except`
         * @return the same flux
         */
        flux<X...> &detach_except(removal_tag_t &remove_tag) {
            std::erase_if(exception_burst.subflows, [remove_tag]<class T>(bound_flow<T> f){ return remove_tag == f.remove.tag; });
            return *this;
        }
    };
}

#endif //ALEMBIC_FLUX_H
