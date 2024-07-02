#pragma once

#include <sstream>
#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/tuple.hpp"
#include "cereal/types/memory.hpp"

#include "Network/Packet.h"

// Example of usage:
// #include <vector>
// #include <cereal/types/vector.hpp>
// struct Test {
// 	int a;
// 	float b;
// 	std::vector<int> c;
// 	void print() {
// 		std::cout << a << ", " << b << "\n";
// 		for (int i : c) {
// 			std::cout << i << "-";
// 		}
// 		std::cout << "\n";
// 	}
// 	template<class Archive>
// 	void serialize(Archive& archive) {
// 		archive(a, b, c);
// 	}
// };
// Test original = {1, 2.0f, {3, 4, 5}};
// size_t size;
// void* buffer = Serializator::serialize(original, size);
// Test deserialized = Serializator::deserialize<Test>(buffer, size);
// original.print();
// deserialized.print();
// free(buffer);

class Serializator {
public:
	template<typename T>
	static void* serialize(const Packet<T>& obj, size_t& size) {
		std::ostringstream oss;
		{
			cereal::BinaryOutputArchive oarchive(oss);
			oarchive(obj);
		}

		std::string serializedData = oss.str();
		size = serializedData.size();
		void* buffer = malloc(size);
		memcpy(buffer, serializedData.data(), size);

		return buffer;
	}

	template<typename T>
	static Packet<T> deserialize(void* buffer, size_t size) {
		std::string serializedData(static_cast<char*>(buffer), size);
		std::istringstream iss(serializedData);
		Packet<T> obj;
		{
			cereal::BinaryInputArchive iarchive(iss);
			iarchive(obj);
		}
		return obj;
	}

	static PacketType desiriealizePacketType(void* buffer, size_t size) {
		std::string serializedData(static_cast<char*>(buffer), size);
		std::istringstream iss(serializedData);
		PacketType packetType;
		{
			cereal::BinaryInputArchive iarchive(iss);
			iarchive(packetType);
		}
		return packetType;
	}
};
