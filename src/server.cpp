#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>

#include <vector>
#include <array>

const uint32_t MAX_CLIENTS = 128;

static const uint16_t InvalidIdentifier = UINT16_MAX;

struct ClientInfo
{
public:
	uint16_t Identifier;

public:
	ClientInfo()
		: Identifier(InvalidIdentifier)
	{
	}

	ClientInfo(uint16_t identifier)
		: Identifier(identifier)
	{}

	bool IsConnected() const
	{
		return Identifier != InvalidIdentifier;
	}
};

uint16_t incrementalIdentifier = 0;

enum class AddressFamily
{
	None,
	IPv4,
	IPv6,
};

class NotImplementedException : public std::logic_error
{
public:
	NotImplementedException() : std::logic_error("Function not yet implemented") { };
};

#include <sstream>

struct Address
{
	Address()
		: type(AddressFamily::None), port(0)
	{
		memset(&address, 0, sizeof(address));
	}

	Address(const sockaddr* sa)
	{
		memset(&address, 0, sizeof(address));

		switch (sa->sa_family)
		{
		case AF_INET:
		{
			type = AddressFamily::IPv4;

			const sockaddr_in* sin = reinterpret_cast<const sockaddr_in*>(sa);
			port = sin->sin_port;
			memcpy(&address.ipv4, &sin->sin_addr, sizeof(address.ipv4));

			break;
		}
		case AF_INET6:
		{
			type = AddressFamily::IPv6;

			const sockaddr_in6* sin6 = reinterpret_cast<const sockaddr_in6*>(sa);
			port = sin6->sin6_port;
			memcpy(&address.ipv6, &sin6->sin6_addr, sizeof(address.ipv6));
		}
		default:
		{
			type = AddressFamily::None;

			// Error?
			assert(false);
		}
		}
	}

	Address(const char* address)
	{
		throw NotImplementedException();
	}

	bool operator==(const Address& other)
	{
		switch (type)
		{
		case AddressFamily::None:
		{
			// Not sure about this?
			return type == other.type
				&& port == other.port;
		}
		case AddressFamily::IPv4:
		{
			return type == other.type
				&& port == other.port
				&& address.ipv4 == other.address.ipv4;
		}
		case AddressFamily::IPv6:
		{
			return type == other.type
				&& port == other.port
				&& address.ipv6 == other.address.ipv6;
		}
		default:
		{
			assert(false);
			return false;
		}
		}
	}

	std::string ToString() const
	{
		switch(type)
		{
		case AddressFamily::None:
		{
			return "None";
		}
		case AddressFamily::IPv4:
		{
			std::stringstream ss;
			ss << (int)address.ipv4[0] << '.' << (int)address.ipv4[1] << '.';
			ss << (int)address.ipv4[2] << '.' << (int)address.ipv4[3];
			ss << ':' << (int)port;
			return ss.str();
		}
		case AddressFamily::IPv6:
		{
			std::stringstream ss;
			ss << (int)address.ipv6[0] << ':' << (int)address.ipv6[1] << ':';
			ss << (int)address.ipv6[2] << ':' << (int)address.ipv6[3] << ':';
			ss << (int)address.ipv6[4] << ':' << (int)address.ipv6[5];
			ss << ':' << (int)port;
			return ss.str();
		}
		default:
		{
			assert(false);
			return false;
		}
		}
	}

	union { std::array<uint8_t, 4> ipv4; std::array<uint8_t, 8> ipv6; } address;
	uint16_t port;
	AddressFamily type;
};


/*
* https://github.com/networkprotocol/netcode/blob/master/netcode.c
static void test_address()
{
	{
		struct netcode_address_t address;
		check(netcode_parse_address("", &address) == NETCODE_ERROR);
		check(netcode_parse_address("[", &address) == NETCODE_ERROR);
		check(netcode_parse_address("[]", &address) == NETCODE_ERROR);
		check(netcode_parse_address("[]:", &address) == NETCODE_ERROR);
		check(netcode_parse_address(":", &address) == NETCODE_ERROR);
		check(netcode_parse_address("1", &address) == NETCODE_ERROR);
		check(netcode_parse_address("12", &address) == NETCODE_ERROR);
		check(netcode_parse_address("123", &address) == NETCODE_ERROR);
		check(netcode_parse_address("1234", &address) == NETCODE_ERROR);
		check(netcode_parse_address("1234.0.12313.0000", &address) == NETCODE_ERROR);
		check(netcode_parse_address("1234.0.12313.0000.0.0.0.0.0", &address) == NETCODE_ERROR);
		check(netcode_parse_address("1312313:123131:1312313:123131:1312313:123131:1312313:123131:1312313:123131:1312313:123131", &address) == NETCODE_ERROR);
		check(netcode_parse_address(".", &address) == NETCODE_ERROR);
		check(netcode_parse_address("..", &address) == NETCODE_ERROR);
		check(netcode_parse_address("...", &address) == NETCODE_ERROR);
		check(netcode_parse_address("....", &address) == NETCODE_ERROR);
		check(netcode_parse_address(".....", &address) == NETCODE_ERROR);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("107.77.207.77", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV4);
		check(address.port == 0);
		check(address.data.ipv4[0] == 107);
		check(address.data.ipv4[1] == 77);
		check(address.data.ipv4[2] == 207);
		check(address.data.ipv4[3] == 77);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("127.0.0.1", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV4);
		check(address.port == 0);
		check(address.data.ipv4[0] == 127);
		check(address.data.ipv4[1] == 0);
		check(address.data.ipv4[2] == 0);
		check(address.data.ipv4[3] == 1);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("107.77.207.77:40000", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV4);
		check(address.port == 40000);
		check(address.data.ipv4[0] == 107);
		check(address.data.ipv4[1] == 77);
		check(address.data.ipv4[2] == 207);
		check(address.data.ipv4[3] == 77);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("127.0.0.1:40000", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV4);
		check(address.port == 40000);
		check(address.data.ipv4[0] == 127);
		check(address.data.ipv4[1] == 0);
		check(address.data.ipv4[2] == 0);
		check(address.data.ipv4[3] == 1);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("fe80::202:b3ff:fe1e:8329", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV6);
		check(address.port == 0);
		check(address.data.ipv6[0] == 0xfe80);
		check(address.data.ipv6[1] == 0x0000);
		check(address.data.ipv6[2] == 0x0000);
		check(address.data.ipv6[3] == 0x0000);
		check(address.data.ipv6[4] == 0x0202);
		check(address.data.ipv6[5] == 0xb3ff);
		check(address.data.ipv6[6] == 0xfe1e);
		check(address.data.ipv6[7] == 0x8329);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("::", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV6);
		check(address.port == 0);
		check(address.data.ipv6[0] == 0x0000);
		check(address.data.ipv6[1] == 0x0000);
		check(address.data.ipv6[2] == 0x0000);
		check(address.data.ipv6[3] == 0x0000);
		check(address.data.ipv6[4] == 0x0000);
		check(address.data.ipv6[5] == 0x0000);
		check(address.data.ipv6[6] == 0x0000);
		check(address.data.ipv6[7] == 0x0000);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("::1", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV6);
		check(address.port == 0);
		check(address.data.ipv6[0] == 0x0000);
		check(address.data.ipv6[1] == 0x0000);
		check(address.data.ipv6[2] == 0x0000);
		check(address.data.ipv6[3] == 0x0000);
		check(address.data.ipv6[4] == 0x0000);
		check(address.data.ipv6[5] == 0x0000);
		check(address.data.ipv6[6] == 0x0000);
		check(address.data.ipv6[7] == 0x0001);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("[fe80::202:b3ff:fe1e:8329]:40000", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV6);
		check(address.port == 40000);
		check(address.data.ipv6[0] == 0xfe80);
		check(address.data.ipv6[1] == 0x0000);
		check(address.data.ipv6[2] == 0x0000);
		check(address.data.ipv6[3] == 0x0000);
		check(address.data.ipv6[4] == 0x0202);
		check(address.data.ipv6[5] == 0xb3ff);
		check(address.data.ipv6[6] == 0xfe1e);
		check(address.data.ipv6[7] == 0x8329);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("[::]:40000", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV6);
		check(address.port == 40000);
		check(address.data.ipv6[0] == 0x0000);
		check(address.data.ipv6[1] == 0x0000);
		check(address.data.ipv6[2] == 0x0000);
		check(address.data.ipv6[3] == 0x0000);
		check(address.data.ipv6[4] == 0x0000);
		check(address.data.ipv6[5] == 0x0000);
		check(address.data.ipv6[6] == 0x0000);
		check(address.data.ipv6[7] == 0x0000);
	}

	{
		struct netcode_address_t address;
		check(netcode_parse_address("[::1]:40000", &address) == NETCODE_OK);
		check(address.type == NETCODE_ADDRESS_IPV6);
		check(address.port == 40000);
		check(address.data.ipv6[0] == 0x0000);
		check(address.data.ipv6[1] == 0x0000);
		check(address.data.ipv6[2] == 0x0000);
		check(address.data.ipv6[3] == 0x0000);
		check(address.data.ipv6[4] == 0x0000);
		check(address.data.ipv6[5] == 0x0000);
		check(address.data.ipv6[6] == 0x0000);
		check(address.data.ipv6[7] == 0x0001);
	}
}
*/

// http://www.blog.matejzavrsnik.com/map_vs_unordered_map_performance.html
std::array<Address, MAX_CLIENTS> ClientAddress;
std::array<bool, MAX_CLIENTS> ClientConnected;

uint16_t FindFreeClientIdentifier()
{
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!ClientConnected[i])
		{
			return i;
		}
	}

	return InvalidIdentifier;
}

uint16_t FindClientIdentifier(const Address& address)
{
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (ClientConnected[i] && ClientAddress[i] == address)
		{
			return i;
		}
	}

	return InvalidIdentifier;
}

bool IsClientConnected(uint16_t clientIdentifier)
{
	return ClientConnected[clientIdentifier];
}

/// 

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <array>

#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

void cb_func(evutil_socket_t fd, short what, void* arg);

struct sockaddr_in servaddr;

int cnt = 0;

int main(int argc, char** argv)
{
	// WSAStartup
	WSADATA wsadata;

	int wsastartupResult = WSAStartup(MAKEWORD(2, 2), &wsadata);

	if (wsastartupResult != 0)
	{
		std::cout << "Error: Failed to initialize winsock API." << std::endl;
		return false;
	}

	///

	const char** methods = event_get_supported_methods();
	printf("Starting libevent %f. Available methods are: \n", event_get_version());
	for (int i = 0; methods[i] != nullptr; ++i) { printf(" - %s\n", methods[i]); }

	///

	event_base* base = event_base_new();
	if (!base)
	{
		printf("couldn't get an event_base! / Could not initialize libevent");
		return 1;
	}
	else
	{
		printf("Using Libevent with backend method %s.", event_base_get_method(base));

		// This is only true on linux?
		event_method_feature features = (event_method_feature)event_base_get_features(base);
		if ((features & EV_FEATURE_ET)) { printf("  Edge-triggered events are supported."); }
		if ((features & EV_FEATURE_O1)) { printf("  O(1) event notifiaction is supported."); }
		if ((features & EV_FEATURE_FDS)) { printf("  ALL FD types are supported"); }
		printf("");
	}

	///

	evutil_socket_t sockfd;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(1053);

	if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	std::cout << "bound" << std::endl;

	event* ev1, * ev2;
	timeval five_seconds = { 5, 0 };
	// event_base* base = event_base_new();

	ev1 = event_new(base, sockfd, EV_TIMEOUT | EV_READ | EV_PERSIST, cb_func, (char*)"Reading event");
	ev2 = event_new(base, sockfd, EV_WRITE | EV_PERSIST, cb_func, (char*)"Writing event");

	event_add(ev1, &five_seconds);
	event_add(ev2, nullptr);
	event_base_dispatch(base);

	return 0;
}

struct Packet
{
	void* data;
	uint16_t size;
	uint16_t recvaddrsize;
	sockaddr* recvaddr;
};

std::array<std::shared_ptr<Packet>, 1024> packets;

int cursor = 0;

void send(std::shared_ptr<Packet> packet)
{
	packets[cursor] = packet;
	cursor = (cursor + 1) % 1024;
}

void cb_func(evutil_socket_t fd, short what, void* arg)
{
	if (what & EV_WRITE)
	{
		for (std::shared_ptr<Packet>& packet : packets)
		{
			if (packet == nullptr)
			{
				return;
			}

			sendto(fd, (const char*)packet->data, packet->size, 0, packet->recvaddr, packet->recvaddrsize);
			packet == nullptr;
		}
	}

	if (what & EV_READ)
	{
		std::array<uint8_t, 1024> buffer;
		memset(&buffer, 0, sizeof(buffer));
		sockaddr_in from;
		int addrlen = sizeof(from);
		int n = recvfrom(fd, (char*)&buffer, 1024, 0, (sockaddr*)&from, &addrlen);
		if (n == SOCKET_ERROR)
		{
			std::cout << "Error reading socket" << WSAGetLastError() << std::endl;
			return;
		}

		uint8_t mode = reinterpret_cast<uint8_t>(&buffer);

		Address address = Address((sockaddr*)&from); // TODO Add compare for *sockaddr in Address class
		
		uint16_t clientIdentifier = FindClientIdentifier(address);

		// In order of frequency
		// if unreliable
		if (mode == 0)
		{
			std::cout << "handling unreliable package" << std::endl;
			// Forward to packet handlers

			if (clientIdentifier == InvalidIdentifier)
			{
				std::cout << "client was not connected" << std::endl;
				return;
			}
		}
		// else if reliable
		else if (mode == 1)
		{
			std::cout << "handling reliable package" << std::endl;


			if (clientIdentifier == InvalidIdentifier)
			{
				std::cout << "client was not connected" << std::endl;
				return;
			}
			// Forward to packet handlers
		}
		// else if connection type
		else if (mode == 2)
		{
			// WARN: This connection protocol, although fast, very unsecure. This will have to be changed before production.
			// Read: https://gafferongames.com/post/client_server_connection/

			/*
			If the sender corresponds to the address of a client that is already connected, also reply with connection accepted.
			This is necessary because the first response packet may not have gotten through due to packet loss. If we don’t resend this response,
			the client gets stuck in the connecting state until it times out.
			*/
			if (clientIdentifier != InvalidIdentifier)
			{
				// Valid identifier
				std::cout << "client already connected" << std::endl;

				// TODO: send connection accepted
				std::shared_ptr<Packet> res = std::make_shared<Packet>();
				res->recvaddr = (sockaddr*)&from;
				res->recvaddrsize = addrlen;
				res->size = sizeof(uint16_t);
				res->data = malloc(res->size);
				memset(res->data, 0b0000001000000001, sizeof(uint16_t)); // mode, status code
				send(res);
				return;
			}

			// TODO challenge

			/*
			If the connection request is from a new client and we have a slot free,
			assign the client to a free slot and respond with connection accepted.
			*/
			uint16_t freeIdentifier = FindFreeClientIdentifier();
			std::cout << "Free: " << freeIdentifier << std::endl;
			if (freeIdentifier != InvalidIdentifier)
			{
				std::cout << "client connected" << std::endl;

				ClientAddress[freeIdentifier] = address;
				std::cout << "retretretre: " << ClientAddress[freeIdentifier].ToString() << std::endl;
				ClientConnected[freeIdentifier] = true;

				// Send connection accepted
				std::shared_ptr<Packet> res = std::make_shared<Packet>();
				res->recvaddr = (sockaddr*)&from;
				res->recvaddrsize = addrlen;
				res->size = sizeof(uint16_t);
				res->data = malloc(res->size);
				memset(res->data, 0b0000001000000001, sizeof(uint16_t)); // mode, status code
				send(res);
				//evpp::udp::SendMessage(res);
			}

			/*
			If the server is full, reply with connection denied.
			*/
			else
			{
				std::cout << "server full connected" << std::endl;

				// Server is full, send connection denied
				std::shared_ptr<Packet> res = std::make_shared<Packet>();
				res->recvaddr = (sockaddr*)&from;
				res->recvaddrsize = addrlen;
				res->size = sizeof(uint16_t);
				res->data = malloc(res->size);
				memset(res->data, 0b0000001000000000, sizeof(uint16_t)); // mode, status code
				send(res);
				//evpp::udp::SendMessage(res);
			}
		}
		else
		{
			std::cout << "handling unknown package mode" << (int)mode << std::endl;
		}
		// std::cout << "MSG:" << msg->NextAllString() << " - fd: " << msg->sockfd() << '\n';
		// evpp::udp::SendMessage(msg);
	}


}