#pragma once

#include <enet/enet.h>
#include <functional>

class Client {
public:
	Client(const char *host, uint16_t port);
	~Client();
	void connect();

	void update();
public:
	void send(const void *data, size_t size);
	void onReceive(std::function<void(void*, size_t)> callback);
	void onConnect(std::function<void(uint16_t)> callback);
	void onDisconnect(std::function<void()> callback);

private:
	ENetHost *client = nullptr;
	ENetPeer *server = nullptr;
	bool connected = false;

	std::function<void(void*, size_t)> receiveCallback;
	std::function<void(uint16_t)> connectCallback;
	std::function<void()> disconnectCallback;
};
