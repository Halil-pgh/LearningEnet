#include "Entity.h"

#include <iostream>

Entity::Entity(std::string name, const Vector2D &position) : m_name(std::move(name)), m_position(position) {}

void Entity::update() {
	float elapsedTime = m_timer.elapsed();
	if (elapsedTime >= 5000.0f) {
		std::cout << m_name << " is at " << m_position.x << ", " << m_position.y << "\n";
		m_timer.reset();
	}
}
