#pragma once

#include <string>
#include "enet/enet.h"

#include "Utils/Vector2D.h"
#include "Utils/Timer.h"

class Entity {
public:
	Entity(std::string name, const Vector2D& position);
	Entity() = default;
	virtual ~Entity() = default;

	virtual void update();
	inline void setPosition(const Vector2D& position) { m_position = position; }
	inline const Vector2D& getPosition() const { return m_position; }
	inline const std::string& getName() const { return m_name; }

	template<class Archive>
	void serialize(Archive& archive) {
		archive(m_name, m_position);
	}

protected:
	std::string m_name;
	Vector2D m_position;
	Timer m_timer;
};
