#include "Player.h"

#include <utility>

#ifdef _WIN32
#include <conio.h>
#define KEY_ESC 27
#else
#include <ncurses.h>
#endif

Player::Player(uint16_t id, std::string name, const Vector2D &position) : Entity(std::move(name), position), m_id(id) {}

void Player::update() {
#ifdef _WIN32
	if (_kbhit() && !m_textMode) {
		int key = _getch();
		if (key == 't') {
			m_textMode = true;
			return;
		}
		switch (key) {
			case 'w':
				m_position.y--;
				break;
			case 's':
				m_position.y++;
				break;
			case 'a':
				m_position.x--;
				break;
			case 'd':
				m_position.x++;
				break;
		}

		// send position to server
		if (m_sendCallback) {
			Packet<Vector2D> packet = {PacketType::Move, m_id, m_position};
			size_t size;
			void* buffer = Serializator::serialize(packet, size);
			m_sendCallback(buffer, size);
		}
	}
	else if (m_textMode) {
		if (_kbhit()) {
			int key = _getch();
			if (key == KEY_ESC) {
				m_textMode = false;
				return;
			}
		}
		std::string text;
		std::getline(std::cin, text);
		if (m_sendCallback) {
			Packet<std::string> packet = {PacketType::Chat, m_id, text};
			size_t size;
			void* buffer = Serializator::serialize(packet, size);
			m_sendCallback(buffer, size);
		}
	}
#else
 	// TODO: Implement it for Linux
	int key = getch();
	switch (key) {
	case 'w':
		m_position.y--;
		break;
	case 's':
		m_position.y++;
		break;
	case 'a':
		m_position.x--;
		break;
	case 'd':
		m_position.x++;
		break;
	}
#endif

	{
		std::lock_guard<std::mutex> lock(m_entitiesMutex);
		for (const auto &[id, entity]: m_entities) {
			entity->update();
		}
	}
}

void Player::onSend(std::function<void(const void *, size_t)> callback) {
	m_sendCallback = std::move(callback);
}

void Player::onJoin(uint16_t id, std::shared_ptr<Entity> entity) {
	std::lock_guard<std::mutex> lock(m_entitiesMutex);
	m_entities[id] = std::move(entity);
}

void Player::onLeave(uint16_t id) {
	std::lock_guard<std::mutex> lock(m_entitiesMutex);
	if (id == m_id) {
		// TODO: Handle player leave
	}
	m_entities.erase(id);
}

void Player::onChat(uint16_t id, const std::string& message) {
	std::cout << m_entities[id]->getName() << ": " << message << "\n";
}

void Player::onMove(uint16_t id, const Vector2D& position) {
	std::lock_guard<std::mutex> lock(m_entitiesMutex);
	m_entities[id]->setPosition(position);
}

std::shared_ptr<Entity> Player::getEntity(uint16_t id) {
	return m_entities[id];
}
