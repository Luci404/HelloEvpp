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
void writecb(struct bufferevent*, void*);
void readcb(struct bufferevent*, void*);

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
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(1053);

	evutil_socket_t sockfd;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	event* ev1, *ev2;
	timeval five_seconds = { 5, 0 };
	// event_base* base = event_base_new();

	ev1 = event_new(base, sockfd, EV_TIMEOUT | EV_READ | EV_PERSIST, cb_func, (char*)"Reading event");
	ev2 = event_new(base, sockfd, EV_WRITE | EV_PERSIST, cb_func, (char*)"Writing event");

	event_add(ev1, &five_seconds);
	event_add(ev2, nullptr);
	event_base_dispatch(base);


	return 0;
}

#include <vector>

void cb_func(evutil_socket_t fd, short what, void* arg)
{
	const char* data = (const char*)arg;
	/*printf("Got an event on socket %d:%s%s%s%s [%s]\n",
		(int)fd,
		(what & EV_TIMEOUT) ? " timeout" : "",
		(what & EV_READ) ? " read" : "",
		(what & EV_WRITE) ? " write" : "",
		(what & EV_SIGNAL) ? " timeout" : "",
		data);*/


	/*struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(fd, (struct sockaddr*)&sin, &len) != -1)
		printf("port number %d - %i\n", ntohs(sin.sin_port), cnt);
		*/

	if (what & EV_WRITE)
	{
		// About MSG_CONFIRM: https://stackoverflow.com/questions/16594387/why-should-i-use-or-not-use-msg-confirm

		std::vector<uint8_t> msg;
		msg.push_back(2); // mode
		sendto(fd, (const char*)msg.data(), msg.size(), 0, (const sockaddr*)&servaddr, sizeof(servaddr));
	}

	if (what & EV_READ)
	{
		// recvmmsg could improve performance at the cost of a significantly more complex interface
		//int readn = ::recvfrom(thread->fd(), (char*)recv_msg->WriteBegin(), recv_buf_size_, 0, recv_msg->mutable_remote_addr(), &addr_len);
		std::array<uint8_t, 1024> buffer;
		memset(&buffer, 0, sizeof(buffer));

		// Warn: MSG_WAITALL should block until all data has been received. From the manual page on recv - https://linux.die.net/man/2/recv
		/*
		https://linux.die.net/man/3/recvfrom
		[...] If the address argument is not a null pointer and the protocol provides the source address of messages,
		the source address of the received message shall be stored in the sockaddr structure pointed to by the address argument,
		and the length of this address shall be stored in the object pointed to by the address_len argument.
		*/
		// int addrlen = sizeof(servaddr);
		int n = recvfrom(fd, (char*)&buffer, 1024, 0, nullptr, nullptr);
		if (n == SOCKET_ERROR)
		{
			std::cout << "Error reading socket" << std::endl;
			return;
		}
		cnt++;

		// TODO: Security check if is coming from server

		uint8_t mode = reinterpret_cast<uint8_t>(&buffer);
		if (mode == 0)
		{
			std::cout << "handling unreliable package" << std::endl;
		}
		else if (mode == 1)
		{
			std::cout << "handling reliable package" << std::endl;
		}
		else if (mode == 2)
		{
			uint8_t res = reinterpret_cast<uint8_t>(&buffer + sizeof(uint8_t));

			if (res == 0) // connection denied
			{
				std::cout << "handling connection denied" << std::endl;
			}
			else if (res == 1) // connection accepted
			{
				std::cout << "handling connection accepted" << std::endl;
				uint8_t myid = reinterpret_cast<uint8_t>(&buffer + (sizeof(uint8_t) * 2));
				std::cout << "myid: " << (int)myid << std::endl;
			}
			else
			{
				std::cout << "handling unknown status code" << std::endl;
			}
		}
		else
		{
			std::cout << "handling unknown mode: " << (int)mode << std::endl;
		}
	}
}

void writecb(bufferevent*, void*)
{	
	std::cout << "writecb" << std::endl;
}

void readcb(bufferevent*, void*)
{
	std::cout << "readrb" << std::endl;
}
	