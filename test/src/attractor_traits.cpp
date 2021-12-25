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
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::static_map<int, long>, int>));
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::static_map<int, long>, double>)); // this is weird behavior but it makes sense.
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::map<int, long>, int>));
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::map<int, long>, double>));
    EXPECT_TRUE((alembic::attractor_default_takes_v<alembic::filter<int>, int>));
    EXPECT_FALSE((alembic::attractor_default_takes_v<alembic::filter<int>, int*>));
    EXPECT_FALSE((alembic::attractor_default_takes_v<alembic::map<test_struct, bool>, int>));
}

TEST(attractor_traits, SeekFindNext) {
    auto flow = alembic::map<double, double> { [](const auto &&x) { return x / 2; } }
            >> alembic::seek { }
            >> alembic::static_map<test_struct, double> { }
            >> alembic::map<double, void> { [](const auto &&x) { std::cout << x; } };
    EXPECT_EQ((alembic::seek::find_next<2, decltype(flow), double>::value), 3);
}
