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

#ifndef ALEMBIC_PART_H
#define ALEMBIC_PART_H

namespace alembic {
    /**
     * Partition a flow, emitting an element to a subflow then discarding the result and passing the original element to the
     * next attractor.
     * @tparam H the first attractor in the subflow
     * @tparam A trailing attractor types in the subflow
     */
    template <class H, class ...A> struct part {
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
}

#endif //ALEMBIC_PART_H
