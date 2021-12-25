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
#include <alembic/flux.h>

TEST(flux_test, double_part_stream) {
    double y = 0.;
    double z = 0.;
    auto f = alembic::map<double, double> { [](auto &&x) { return x / 2; } }
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
}

TEST(flux_test, flux_stream) {
    char out[12];
    alembic::flux<const char *> f;
    f.attach(alembic::flow(alembic::tap<const char *> { [&out](auto &&x) -> void { strcpy(out, x); } }))
        .emit("swordfish");
    EXPECT_STREQ(out, "swordfish");
}

TEST(flux_test, exception) {
    char out[6];
    alembic::flux<const char *> f;
    f.attach(alembic::flow(alembic::tap<const char *> { [](auto &&x) -> void { throw std::runtime_error("hello"); } }))
        .except(alembic::flow(alembic::tap<const std::exception &> { [&out](auto &ex) { strcpy(out, ex.what()); }}))
        .emit("hi");
    EXPECT_STREQ(out, "hello");
}
