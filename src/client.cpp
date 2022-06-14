#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>

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

struct sockaddr_in serveraddr;

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
	
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons(1053);

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

void cb_func(evutil_socket_t fd, short what, void* arg)
{
	const char* data = (const char*)arg;
	printf("Got an event on socket %d:%s%s%s%s [%s]\n",
		(int)fd,
		(what & EV_TIMEOUT) ? " timeout" : "",
		(what & EV_READ) ? " read" : "",
		(what & EV_WRITE) ? " write" : "",
		(what & EV_SIGNAL) ? " timeout" : "",
		data);

	if (what & EV_WRITE)
	{
		// About MSG_CONFIRM: https://stackoverflow.com/questions/16594387/why-should-i-use-or-not-use-msg-confirm
		const char* msg = "Hello from client";
		sendto(fd, msg, strlen(msg), 0, (const sockaddr*)&serveraddr, sizeof(serveraddr));
	}

	if (what & EV_TIMEOUT)
	{

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
