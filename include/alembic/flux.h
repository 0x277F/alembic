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
#include "attractors_builtin.h"
#include "util.h"

namespace alembic {

    /**
     * Represents the point at which elements might be emitted to a flow.
     * @tparam X the type of element being admitted to the head of the flow
     */
    template <class X> class flux {
        burst<X> main_burst;
        burst<const std::exception &> exception_burst;

    public:
        /**
         * Emit an element to all attached flows.
         * @param x the element ot emit
         * @return the same flux
         */
        const flux<X> &emit(X x) const {
            try {
                main_burst.inner_emit(x);
            } catch (const std::exception &ex) {
                exception_burst.inner_emit(ex);
            }
            return *this;
        }

        /**
         * Attaches the given flow to the flux.
         * @param flow the flow to attach
         * @param count a pointer with which a number that can be used as a parameter to `detach` may be returned.
         * @return the same flux
         */
        template <class ...A> flux<X> &attach(flow<A...> &&flow, size_t *count = nullptr) {
            main_burst.subflows.emplace_back(bind_flow(flow));
            if (count) {
                *count = main_burst.subflows.size();
            }
            return *this;
        }

        /**
         * Attaches a flow to the exception handling chain. Any `std::exception` thrown during an `emit` will be caught and
         * passed to this flow.
         * @param flow the flow to attach
         * @return the same flux
         */
        template <class ...A> flux<X> &except(flow<A...> &&flow, size_t *count = nullptr) {
            exception_burst.subflows.template emplace_back(bind_flow(flow));
            if (count) {
                *count = exception_burst.subflows.size();
            }
            return *this;
        }

        /**
         * Detaches a flow from the flux.
         * @param count an identifier returned in a call to `attach`
         * @return the same flux
         */
        flux<X> &detach(size_t count) {
            std::erase(main_burst.subflows, main_burst.subflows[count - 1]);
        }

        /**
         * Detaches a flow from the exception handling chain.
         * @param count an identifier returned in a call to `except`
         * @return the same flux
         */
        flux<X> &detach_except(size_t count) {
            std::erase(exception_burst.subflows, exception_burst.subflows[count - 1]);
        }
    };
}

#endif //ALEMBIC_FLUX_H
