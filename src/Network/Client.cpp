#include "Client.h"

#include <iostream>
#include <utility>

Client::Client(const char *host, uint16_t port) {
	client = enet_host_create(nullptr, 1, 2, 0, 0);
	if (client == nullptr) {
		std::cout << "An error occurred while creating the client.\n";
		exit(EXIT_FAILURE);
	}

	ENetAddress serverAddress{};
	enet_address_set_host(&serverAddress, host);
	serverAddress.port = port;
	server = enet_host_connect(client, &serverAddress, 2, 0);
	if (server == nullptr) {
		std::cout << "No available server for initiating an ENet connection.\n";
		exit(EXIT_FAILURE);
	}

	// Ping the host every 5 seconds to keep the connection alive.
	enet_peer_ping_interval(server, 5000);
}

Client::~Client() {
	ENetEvent event;
	enet_peer_disconnect(server, 0);
	while (enet_host_service(client, &event, 3000) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
					   event.packet->dataLength,
					   event.packet->data,
					   event.peer->data,
					   event.channelID);
				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("Disconnection succeeded.\n");
				connected = false;
				break;

			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				printf("Disconnection failed.\n");
				break;

			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}
	if (!connected) {
		enet_peer_reset(server);
	}
	enet_host_destroy(client);
}

void Client::update() {
	ENetEvent event;
	while (enet_host_service(client, &event, 1000) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				receiveCallback(event.packet->data, event.packet->dataLength);
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				disconnectCallback();
				connected = false;
				break;

			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				disconnectCallback();
				connected = false;
				break;

			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}
}

void Client::send(const void *data, size_t size) {
	ENetPacket* packet = enet_packet_create(data, size, ENET_PACKET_FLAG_RELIABLE); // Not sure if reliable is the best option.
	enet_peer_send(server, 0, packet);
	enet_host_flush(client);
}

void Client::onReceive(std::function<void(void *, size_t)> callback) {
	receiveCallback = std::move(callback);
}

void Client::onConnect(std::function<void(uint16_t)> callback) {
	connectCallback = std::move(callback);
}

void Client::onDisconnect(std::function<void()> callback) {
	disconnectCallback = std::move(callback);
}

void Client::connect() {
	ENetEvent event;
	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		std::cout << "Connection to the server succeeded.\n";
		connected = true;
		connectCallback(event.peer->outgoingPeerID);
	} else {
		std::cout << "Connection to the server failed.\n";
		enet_peer_reset(server);
	}
}
