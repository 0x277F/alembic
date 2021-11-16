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
#include <alembic/map.h>
#include <alembic/filter.h>
#include <alembic/part.h>
#include <alembic/flux.h>

TEST(stream_test, part_stream) {
    double y = 0.;
    double z = 0.;
    alembic::flow f = alembic::map<double, double> { [](auto &&x) { return x / 2; } }
            >> alembic::part {
                alembic::filter<double> { [](auto &&x){ return x > 5; } }
                >> alembic::map<double, void> { [&y](auto &&x){ y = x; } }
            }
            >> alembic::map<double, void> { [&z](auto &&x) { z = x; } };
    f.attractor<0>().emit<0>(1., &f);
    EXPECT_DOUBLE_EQ(y, 0.);
    EXPECT_DOUBLE_EQ(z, 1./2);

    f.attractor<0>().emit<0>(20., &f);
    EXPECT_DOUBLE_EQ(y, 10.);
    EXPECT_DOUBLE_EQ(z, 10.);

    alembic::flux<double> flux;
    std::cout << "flow " << flux.attach(f) << " ";
    flux.emit(1);
}
