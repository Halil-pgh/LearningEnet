#pragma once

#include <enet/enet.h>
#include <functional>

class Server {
public:
	explicit Server(uint16_t port);
	~Server();

	void update();
public:
	void send(ENetPeer *peer, const void *data, size_t size);
	void broadcast(const void *data, size_t size);
	void disconnect(ENetPeer *peer);

	void onReceive(std::function<void(ENetPeer*, void*, size_t)> callback);
	void onConnect(std::function<void(ENetPeer*)> callback);
	void onDisconnect(std::function<void(ENetPeer*)> callback);

private:
	ENetAddress address{};
	ENetHost *host = nullptr;

	std::function<void(ENetPeer*, void*, size_t)> receiveCallback;
	std::function<void(ENetPeer*)> connectCallback;
	std::function<void(ENetPeer*)> disconnectCallback;
};
