#include <iostream>
#include "Network/Server.h"
#include "Network/Client.h"
#include "Network/Networker.h"
#include "Player.h"

#include <thread>
#include <unordered_map>

#define SERVER_ID UINT16_MAX

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);

	Networker::initialize();
	std::cout << "Do you want to start a server or a client: ";
	std::string answer;
	std::cin >> answer;

	if (answer == "server") {
		Server server(7777);
		std::unordered_map<uint16_t, std::shared_ptr<Entity>> entities;
		server.onConnect([](ENetPeer* peer) {
			std::string message = "[SERVER]: " + Networker::hostToIp(peer->address.host) + ":" + std::to_string(peer->address.port) + " connected.";
			std::cout << message << "\n";
			std::cout << "[SERVER]: Connected peer ID: " << peer->incomingPeerID << "\n";
		});
		server.onReceive([&server, &entities](ENetPeer* peer, void* data, size_t size) {
			PacketType type = Serializator::desiriealizePacketType(data, size);
			std::cout << "[SERVER]: Packet type: " << static_cast<int>(type) << "\n";
			if (type == PacketType::Join) {
				for (const auto& [id, entity] : entities) {
					Packet<Entity> packet = {PacketType::Join, id, *(entity)};
					size_t _size;
					void* buffer = Serializator::serialize(packet, _size);
					server.send(peer, buffer, _size);
					free(buffer);
				}
				Packet<Entity> entity = Serializator::deserialize<Entity>(data, size);
				entities[entity.id] = std::make_shared<Entity>(entity.data);
				std::string newMessage = "[SERVER]: " + entity.data.getName() + " joined the server.";
				std::cout << newMessage << "\n";
				server.broadcast(data, size);
				return;
			} else if (type == PacketType::Leave) {
				Packet<uint16_t> id = Serializator::deserialize<uint16_t>(data, size);
				std::string newMessage = "[SERVER]: " + entities[id.data]->getName() + " left the server.";
				std::cout << newMessage << "\n";
				server.broadcast(data, size);
				entities.erase(id.data);
				return;
			} else if (type == PacketType::Chat) {
				Packet<std::string> packet = Serializator::deserialize<std::string>(data, size);
				const std::string& message = packet.data;
				// Check if the message is a command.
				if (message[0] == '/') {
					if (message == "/list") {
						std::string list = "[SERVER]: ";
						for (const auto& [_, entity] : entities) {
							list += entity->getName() + ", ";
						}
						list.pop_back();
						list.pop_back();
						Packet<std::string> listPacket = {PacketType::Chat, SERVER_ID, list};
						size_t _size;
						void* buffer = Serializator::serialize(listPacket, _size);
						server.send(peer, buffer, _size);
						free(buffer);
						return;
					} else if (message == "/disconnect") {
						std::string disconnectMessage = "[SERVER]: " + entities[peer->incomingPeerID]->getName() + " disconnected.";
						std::cout << disconnectMessage << "\n";
						Packet<uint16_t> leavePacket = {PacketType::Leave, peer->incomingPeerID, peer->incomingPeerID};
						size_t _size;
						void* buffer = Serializator::serialize(leavePacket, _size);
						server.broadcast(buffer, _size);
						entities.erase(peer->incomingPeerID);
						server.disconnect(peer);
						free(buffer);
						return;
					} else {
						std::string errorMessage = "[SERVER]: Invalid command.";
						Packet<std::string> errorPacket = {PacketType::Chat, SERVER_ID, errorMessage};
						size_t _size;
						void* buffer = Serializator::serialize(errorPacket, _size);
						server.send(peer, buffer, _size);
						free(buffer);
						return;
					}
				}
				std::string newMessage = "[" + entities[packet.id]->getName() + "]: " + packet.data;
				std::cout << newMessage << "\n";
				server.broadcast(data, size);
				return;
			} else if (type == PacketType::Move) {
				Packet<Vector2D> position = Serializator::deserialize<Vector2D>(data, size);
				entities[position.id]->setPosition(position.data);
				server.broadcast(data, size);
				return;
			}
		});
		server.onDisconnect([&server, &entities](ENetPeer* peer) {
			std::string message = "[SERVER]: " + entities[peer->incomingPeerID]->getName() + " disconnected.";
			std::cout << message << "\n";
			entities.erase(peer->incomingPeerID);
			Packet<uint16_t> packet = {PacketType::Leave, peer->incomingPeerID, peer->incomingPeerID};
			size_t size;
			void* buffer = Serializator::serialize(packet, size);
			server.broadcast(buffer, size);
		});
		while (true) {
			server.update();
		}
	} else if (answer == "client") {
		std::string name;
		std::cout << "Enter your name: ";
		std::cin >> name;

		Client client("localhost", 7777);
		std::shared_ptr<Player> player;
		client.onConnect([&name, &client, &player](uint16_t id) {
			std::cout << "[INFO]: Connected to the server.\n";
			player = std::make_shared<Player>(id, name, Vector2D(0, 0));
			Packet<Entity> packet = {PacketType::Join, player->getId(), *(player)};
			size_t size;
			void* buffer = Serializator::serialize(packet, size);
			client.send(buffer, size);
			free(buffer);
		});
		client.connect(); // Make sure to call this function after setting the on connect callback.
		player->onSend([&client](const void* data, size_t size) {
			client.send(data, size);
		});
		client.onDisconnect([]() {
			std::cout << "[INFO]: Disconnected from the server.\n";
		});
		client.onReceive([&player](void* data, size_t size) {
			PacketType type = Serializator::desiriealizePacketType(data, size);
			if (type == PacketType::Join) {
				Packet<Entity> entity = Serializator::deserialize<Entity>(data, size);
				player->onJoin(entity.id, std::make_shared<Entity>(entity.data));
				std::cout << "[INFO]: " << entity.data.getName() << " joined the server with id " << entity.id << ".\n";
			} else if (type == PacketType::Leave) {
				Packet<uint16_t> id = Serializator::deserialize<uint16_t>(data, size);
				std::cout << "[INFO]: " << player->getEntity(id.data)->getName() << " left the server.\n";
				player->onLeave(id.data);
			} else if (type == PacketType::Chat) {
				Packet<std::string> message = Serializator::deserialize<std::string>(data, size);
				if (message.id == SERVER_ID) {
					std::cout << message.data << "\n";
					return;
				}
				player->onChat(message.id, message.data);
			} else if (type == PacketType::Move) {
				Packet<Vector2D> position = Serializator::deserialize<Vector2D>(data, size);
				player->onMove(position.id, position.data);
			}
		});
		std::thread terminalThread([&player]() {
			while (true) {
				player->update();
			}
		});
		while (true) {
			client.update();
		}
	} else {
		std::cout << "Invalid answer.\n";
	}
	Networker::deinitialize();
}