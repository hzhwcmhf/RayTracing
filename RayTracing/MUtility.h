#pragma once

#include <random>

extern std::default_random_engine random_engine;

inline int random_range(int a, int b)
{
	return std::uniform_int_distribution<int>(a, b)(random_engine);
}

inline int random_pro(double p)
{
	return std::discrete_distribution<int>{1 - p, p}(random_engine);
}

inline double normal_distribution(double u, double stdv, double &pro)
{
	double x = std::normal_distribution<double>(u, stdv)(random_engine);
	pro *= 1 / sqrt(2 * PI) / stdv * exp(-(x - u) * (x - u) / 2 / stdv / stdv);
	return x;
}