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
#include <alembic/flow.h>
#include <alembic/attractors_builtin.h>

struct test_struct { };

TEST(attractor_traits, AttractorDefaultTakes) {
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::map<decltype([](int x){ })>, int>));
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::map<decltype([](int x){ })>, double>));
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::filter<decltype([](int x){ return false; })>, int>));
    EXPECT_FALSE((alembic::attractor_default_takes_v<alembic::filter<decltype([](int x){ return false; })>, int*>));
    EXPECT_FALSE((alembic::attractor_default_takes_v<alembic::map<decltype([](test_struct x){ })>, int>));
}

TEST(attractor_traits, SeekFindNext) {
    auto flow = alembic::map { [](const auto &&x) { return x / 2; } }
            >> alembic::seek { }
            >> alembic::filter([](const test_struct &&x){ return false; })
            >> alembic::map  { [](const auto &&x) { std::cout << x; } };
    EXPECT_EQ((alembic::find_next<2, decltype(flow), double>::value), 3);
}

TEST(attractor_traits, SeekFindPrev) {
    auto flow = alembic::map { [](const auto &&x) { return x / 2; } }
            >> alembic::seek { }
            >> alembic::filter([](const test_struct &&x){ return false; })
            >> alembic::map  { [](const auto &&x) { std::cout << x; } };

    EXPECT_EQ((alembic::find_prev<2, decltype(flow), const double&&>::value), 1);
}
