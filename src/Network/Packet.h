#pragma once

#include <cstdint>

enum class PacketType : uint8_t {
	Move, // For Vector2D
	Chat, // For std::string
	Join, // For Entity
	Leave // Empty packet
};

template <typename T>
struct Packet {
	PacketType type;
	uint16_t id; // Client ID, who's sending the packet, given by the server
	T data;

	template<class Archive>
	void serialize(Archive& archive) {
		archive(type, id, data);
	}
};