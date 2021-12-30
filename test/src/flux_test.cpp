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

TEST(flux_test, PartFlow) {
    double y = 0.;
    double z = 0.;
    auto f = alembic::map { [](auto &&x) { return x / 2; } }
            >> alembic::part {
                alembic::filter { [](double x){ return x > 5; } }
                >> alembic::map { [&y](auto x){ y = x; } }
            }
            >> alembic::map { [&z](auto x) { z = x; } };
    f.attractor<0>().emit<0>(1., &f);
    EXPECT_DOUBLE_EQ(y, 0.);
    EXPECT_DOUBLE_EQ(z, 1./2);

    f.attractor<0>().emit<0>(20., &f);
    EXPECT_DOUBLE_EQ(y, 10.);
    EXPECT_DOUBLE_EQ(z, 10.);
}

TEST(flux_test, BasicFlux) {
    char out[12];
    alembic::flux<const char *> f;
    f.attach(alembic::map { [&out](const char *x) -> void { strcpy(out, x); } })
        .emit("swordfish");
    EXPECT_STREQ(out, "swordfish");
}

TEST(flux_test, Exception) {
    char out[6];
    alembic::flux<const char *> f;
    f.attach(alembic::map { [](auto x) -> void { throw std::runtime_error("hello"); } })
        .except(alembic::flow(alembic::map { [&out](auto ex) -> void {
            try {
                std::rethrow_exception(ex);
            } catch (const std::runtime_error &ex1) {
                strcpy(out, ex1.what());
            }
        }}))
        .emit("hi");
    EXPECT_STREQ(out, "hello");
}

TEST(flux_test, PolytypeFlux) {
    struct test_struct {
        int n;
        std::string value;
    };

    alembic::flux<double, test_struct> f;

    // uncommenting this should trigger a static assertion failure, but clang inspections don't seem to catch it until compilation.
    // f.emit("swordfish");

    double x = 0.;
    test_struct y;

    f.attach(alembic::map { [&x](const double &d){
        x = d;
        return d;
    } } >> alembic::seek { } >> alembic::map { [&y](const test_struct &s){
        y = s;
        return s;
    } });

    f.emit(test_struct { 2, "swordfish" });

    EXPECT_EQ(y.n, 2);
    EXPECT_STREQ(y.value.c_str(), "swordfish");
    EXPECT_DOUBLE_EQ(x, 0.);

    f.emit(0.451);
    EXPECT_DOUBLE_EQ(x, 0.451);
    EXPECT_EQ(y.n, 2);
    EXPECT_STREQ(y.value.c_str(), "swordfish");
}

TEST(flux_test, FlowRemoval) {
    alembic::flux<int> f;
    alembic::removal_tag_t tag;

    int x = 0;

    f.attach(alembic::map { [&x](auto i){ x = i + 1; } }, &tag);
    f.attach(alembic::map { [](auto i){ return i + 1; } });

    f.emit(92);
    EXPECT_EQ(x, 93);
    f.detach(tag);
    f.emit(54);
    EXPECT_EQ(x, 93);
}

TEST(flux_test, CollectN) {
    alembic::flux<int> f;
    std::array<int, 3> out = { 0, 0, 0 };

    f.attach(alembic::collect_n<int, 3>() >> alembic::map { [&out](auto array){ out = array; } });
    f.emit(-5);
    f.emit(102);
    f.emit(33);

    EXPECT_EQ(out[0], -5);
    EXPECT_EQ(out[1], 102);
    EXPECT_EQ(out[2], 33);
}


TEST(flux_test, FlattenArray) {
    alembic::flux<std::array<int, 4>> f;

    int accumulator = 0;
    f.attach(alembic::flat { } >> alembic::map { [&accumulator](int i){ accumulator += i; } });

    f.emit(std::array { 1, 14, 0, 33 });
    EXPECT_EQ(accumulator, 48);
}

TEST(flux_test, FlattenVector) {
    alembic::flux<std::vector<int> &> f;

    int accumulator = 0;
    f.attach(alembic::flat { } >> alembic::map { [&accumulator](int i){
        accumulator += i;
    } });

    std::vector<int> input = { 10, 20, 30, -8 };
    f.emit(input);
    EXPECT_EQ(accumulator, 52);
}

TEST(flux_test, Reduce) {
    alembic::flux<int> f;

    f.attach(alembic::reduce { [acc = 0](int &&i) mutable {
        acc += i;
        if (acc > 6) {
            return std::optional<int>(acc);
        }
        return std::optional<int>();
    } } >> alembic::map { [](int total){
        EXPECT_EQ(total, 12);
    } });

    f.emit(3);
    f.emit(2);
    f.emit(7);
}
