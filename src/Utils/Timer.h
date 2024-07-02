#pragma once

#include <chrono>

class Timer {
public:
	Timer() {
		m_start = std::chrono::high_resolution_clock::now();
	}

	float elapsed() const {
		auto end = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
		auto end_ = std::chrono::time_point_cast<std::chrono::microseconds>(end).time_since_epoch().count();
		auto duration = end_ - start;
		return duration * 0.001f;
	}

	void reset() {
		m_start = std::chrono::high_resolution_clock::now();
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};