#pragma once

#include <string>
#include <functional>
#include <memory>
#include <mutex>

#include "Utils/Serializator.h"
#include "Utils/Vector2D.h"
#include "Entity.h"

class Player : public Entity {
public:
	Player(uint16_t id, std::string name, const Vector2D& position);
	~Player() override = default;

	void update() override;
	void onSend(std::function<void(const void*, size_t)> callback);
	void onJoin(uint16_t id, std::shared_ptr<Entity> entity);
	void onLeave(uint16_t id);
	void onChat(uint16_t id, const std::string& message);
	void onMove(uint16_t id, const Vector2D& position);
	uint16_t getId() const { return m_id; }
	std::shared_ptr<Entity> getEntity(uint16_t id);

private:
	uint16_t m_id;
	std::function<void(const void*, size_t)> m_sendCallback;

private:
	bool m_textMode = false;
	std::unordered_map<uint16_t, std::shared_ptr<Entity>> m_entities;
	std::mutex m_entitiesMutex;
};
