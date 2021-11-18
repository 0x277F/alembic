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

#include <vector>
#include "flow.h"

namespace alembic {

    /**
     * Represents the point at which elements might be emitted to a flow.
     * @tparam X the type of element being admitted to the head of the flow
     * @tparam Vec the storage backing for functions which accept a reference to the element and emit them to the flow.
     */
    template <class X, class Vec = std::vector<std::function<void(const X&&)>>> class flux {
    protected:
        Vec vec;

    public:
        /**
         * Emit an element to all attached flows.
         * @param x the element ot emit
         * @return the same flux
         */
        flux<X, Vec> &emit(const X &&x) {
            std::for_each(vec.begin(), vec.end(), [x](auto &a){ a(std::move(x)); });
            return *this;
        }

        /**
         * Attach a new flow to the flux.
         * @tparam H the first attractor type in the flow
         * @tparam A trailing attractor types in the flow
         * @param f the flow reference
         * @return an identifier for the flow which may be used to detach it later
         */
        template <class H, class ...A> size_t attach(const flow<H, A...> &f) {
            const auto head = f.template attractor<0>();
            vec.emplace_back(std::bind(&H::template emit<0, flow<H, A...>>, head, std::placeholders::_1, &f));
            return vec.size();
        }

        template <attractor_type H> size_t attach(const H &h) {
            return attach(flow(h));
        }

        /**
         * Detach a flow from the flux.
         * @param i the identifier obtained with `attach`
         * @return whether or not it was successful
         */
        bool detach(const size_t i) {
            return vec.erase(vec.begin() + i);
        }
    };
}

#endif //ALEMBIC_FLUX_H
