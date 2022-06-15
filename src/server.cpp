#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>

#include <vector>

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

struct Address
{
	Address()
		: type(AddressFamily::None)
	{
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
			memcpy(&sin->sin_addr, address.ipv4, sizeof(address.ipv4));

			break;
		}
		case AF_INET6:
		{
			type = AddressFamily::IPv6;

			const sockaddr_in6* sin6 = reinterpret_cast<const sockaddr_in6*>(sa);
			port = sin6->sin6_port;
			memcpy(&sin6->sin6_addr, address.ipv6, sizeof(address.ipv6));
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

	/*/Address& operator=(const Address& other)
	{
		x = other.x;
		return *this;
	}*/

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

	union { uint8_t ipv4[4]; uint8_t ipv6[8]; } address;
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

#include <array>
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

ClientInfo ClientInfos[MAX_CLIENTS];
int main(int argc, char** argv)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int error = WSAStartup(wVersionRequested, &wsaData);
	if (0 != error) { return 0; }

	///


	///

	std::vector<int> ports = { 1053, 5353 };
	evpp::udp::Server server;

	server.SetMessageHandler([](evpp::EventLoop* loop, evpp::udp::MessagePtr& msg) {
		// In order of frequency

		uint8_t mode = msg->ReadByte();

		uint16_t clientIdentifier = FindClientIdentifier(msg->remote_addr());

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
			if (clientIdentifier == InvalidIdentifier)
			{
				std::cout << "client already connected" << std::endl;

				// TODO: send connection accepted
				evpp::udp::MessagePtr msg = std::make_shared<evpp::udp::Message>(msg->sockfd());
				msg->AppendInt8(2); // mode
				msg->AppendInt8(1); // status code
				evpp::udp::SendMessage(msg);
				return;
			}

			// TODO challenge

			/*
			If the connection request is from a new client and we have a slot free,
			assign the client to a free slot and respond with connection accepted.
			*/
			uint16_t freeIdentifier = FindFreeClientIdentifier();
			if (freeIdentifier != InvalidIdentifier)
			{
				ClientAddress[freeIdentifier] = Address(msg->remote_addr());
				ClientConnected[freeIdentifier] = true;

				// Send connection accepted
				evpp::udp::MessagePtr msg = std::make_shared<evpp::udp::Message>(msg->sockfd());
				msg->AppendInt8(2); // mode
				msg->AppendInt8(1); // status code
				evpp::udp::SendMessage(msg);
			}

			/*
			If the server is full, reply with connection denied.
			*/
			else
			{
				// Server is full, send connection denied
				evpp::udp::MessagePtr msg = std::make_shared<evpp::udp::Message>(msg->sockfd());
				msg->AppendInt8(2); // mode
				msg->AppendInt8(0); // status code
				evpp::udp::SendMessage(msg);
			}
		}
		else
		{
			std::cout << "handling unknown package type" << std::endl;
		}
		// std::cout << "MSG:" << msg->NextAllString() << " - fd: " << msg->sockfd() << '\n';
		// evpp::udp::SendMessage(msg);
		});

	server.Init(ports);
	server.Start();

	while (!server.IsStopped()) { usleep(1); }

	return 0;
}