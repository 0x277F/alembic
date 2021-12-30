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

#include <gtest/gtest.h>
#include <alembic/attractors_builtin.h>
#include <alembic/flow.h>

TEST(builtins, concept_check) {
    struct test_struct { };

    static_assert(alembic::attractor_type<alembic::filter<test_struct>>);
    static_assert(alembic::attractor_type<alembic::map<decltype([](int x){ })>>);
    static_assert(alembic::attractor_type<alembic::part<alembic::filter<test_struct>>>);
    static_assert(alembic::attractor_type<alembic::seek>);
    static_assert(alembic::attractor_type<alembic::burst<test_struct>>);
    static_assert(alembic::attractor_type<alembic::collect_n<int, 5>>);
    static_assert(alembic::attractor_type<alembic::flat>);

    EXPECT_FALSE(alembic::attractor_type<test_struct>);
}
