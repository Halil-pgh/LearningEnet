#include "Server.h"

#include <iostream>
#include <thread>

Server::Server(uint16_t port) {
	address.host = ENET_HOST_ANY;
	address.port = port;
	host = enet_host_create(&address, 32, 2, 0, 0);
	if (host == nullptr) {
		std::cout << "An error occurred while creating the server.\n";
		exit(EXIT_FAILURE);
	}
	std::cout << "Server started on port " << port << ".\n";
}

Server::~Server() {
	enet_host_destroy(host);
}

void Server::update() {
	ENetEvent event;
	while (enet_host_service(host, &event, 1000) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				// printf("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
				connectCallback(event.peer);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				/* printf("A packet of length %zu containing %s was received from %s on channel %u.\n",
					   event.packet->dataLength,
					   event.packet->data,
					   event.peer->data,
					   event.channelID); */
				receiveCallback(event.peer, event.packet->data, event.packet->dataLength);
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				// printf("%s disconnected.\n", event.peer->data);
				disconnectCallback(event.peer);
				event.peer->data = NULL;
				break;

			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				// printf("%s disconnected due to timeout.\n", event.peer->data);
				disconnectCallback(event.peer);
				event.peer->data = NULL;
				break;

			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}
}

void Server::send(ENetPeer *peer, const void *data, size_t size) {
	ENetPacket *packet = enet_packet_create(data, size, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void Server::broadcast(const void *data, size_t size) {
	ENetPacket *packet = enet_packet_create(data, size, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(host, 0, packet);
}

void Server::onReceive(std::function<void(ENetPeer *, void *, size_t)> callback) {
	receiveCallback = callback;
}

void Server::onConnect(std::function<void(ENetPeer *)> callback) {
	connectCallback = callback;
}

void Server::onDisconnect(std::function<void(ENetPeer *)> callback) {
	disconnectCallback = callback;
}

void Server::disconnect(ENetPeer *peer) {
	enet_peer_disconnect(peer, 0);
	ENetEvent event;
	bool disconnected = false;
	if (enet_host_service(host, &event, 3000) > 0 && event.type == ENET_EVENT_TYPE_DISCONNECT) {
		disconnected = true;
	}
	if (!disconnected) {
		enet_peer_reset(peer);
	}
}
