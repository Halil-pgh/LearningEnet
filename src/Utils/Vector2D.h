#pragma once

struct Vector2D {
	int x = 0;
	int y = 0;

	Vector2D() = default;
	Vector2D(int x, int y) : x(x), y(y) {}

	Vector2D operator+(const Vector2D& other) const {
		return {x + other.x, y + other.y};
	}

	Vector2D operator-(const Vector2D& other) const {
		return {x - other.x, y - other.y};
	}

	Vector2D operator*(int scalar) const {
		return {x * scalar, y * scalar};
	}

	template<class Archive>
	void serialize(Archive& archive) {
		archive(x, y);
	}
};
