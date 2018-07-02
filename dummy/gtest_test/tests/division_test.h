#ifndef DIVISION_TEST_H
#define DIVISION_TEST_H

#include "gtest/gtest.h"
#include "division.h"

TEST(DivisionTest, HandlesPositiveAndNegativeNumbers) {
	float x[] = {4.0, 5.0, 6.54, 1289.4532, 3453453.1, 4.1};
	float y[] = {2.0, 2.0, 2.3, 2341.3332, 333.333, 1.0};
	float result[] = {4.0 / 2.0, 5.0 / 2.0, 6.54 / 2.3, 1289.4532 / 2341.3332,
					  3453453.1 / 333.333, 4.1 / 1.0};

	int size = sizeof(x) / sizeof(x[0]);

    for (int i = 0; i < size; i++) {
    	EXPECT_FLOAT_EQ(result[i], Division::apply(x[i], y[i]));
    }
}

TEST(DivisionTest, HandlesDivisionByZero) {
    ASSERT_THROW(Division::apply(5, 0), std::overflow_error);
}

#endif