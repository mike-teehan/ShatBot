/*
 * IrcBot.cpp
 *
 * MIT License (MIT)
 * Copyright (c) 2014 Mike Teehan <mike.teehan@gmail.com>
 */

#include "IrcBot.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#define MAXDATASIZE 256

using namespace std;
using boost::algorithm::split;
using boost::algorithm::is_any_of;
using boost::assign::list_of;

IrcBot::IrcBot(string host, string port, string chan, string nick, string real) {
	_host = host;
	_port = port;
	_chan = chan;
	_nick = nick;
	_real = real;
}

IrcBot::~IrcBot() {
	close(_socket);
}

void IrcBot::connectSocket() {
	struct addrinfo hints, *servinfo;

	//Ensure that servinfo is clear
	memset(&hints, 0, sizeof hints); // make sure the struct is empty

	//setup hints
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	//Setup the structs if error print why
	int res;
	if( (res = getaddrinfo(_host.c_str(), _port.c_str(), &hints, &servinfo) ) != 0)
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));

	//setup the socket
	_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if(_socket == -1)
		perror("client: socket");

	//Connect
	if(connect(_socket, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(_socket);
		perror("Client Connect");
	}

	//We dont need this anymore
	freeaddrinfo(servinfo);
}

void IrcBot::start() {

	connectSocket();

	if(_socket <= 0)
		exit(1);

	//Recv some data
	int numbytes;
	char buf[MAXDATASIZE];
	string incoming_str;
	string buf_str;
	string line_str;
	vector<string> lines;

	int count = 0;
	while(true) {
		//Recv & print Data
		numbytes = recv(_socket, buf, MAXDATASIZE - 1, MSG_DONTWAIT);
		if(numbytes < 0 && errno != EAGAIN)
			break;
		if(numbytes == 0)
			break;
		if(numbytes == -1 && errno == EAGAIN) {
			usleep(1000);
			continue;
		}
		buf[numbytes]='\0';
		incoming_str = string(buf);
		buf_str = buf_str + incoming_str;

		lines.clear();
		while(true) {
			int pos = buf_str.find("\r\n", 0);
			if(pos != string::npos) {
				lines.push_back(buf_str.substr(0, pos) );
				buf_str.erase(0, pos + 2);
			} else
				break;
		}

		//Pass buf to the message handeler
		for(vector<string>::const_iterator lit = lines.begin(); lit != lines.end(); ++lit) {
			cout << "> " + *lit << endl;
			processLine(*lit);
			count++;
		}

	}
	cout << "----------------------CONNECTION CLOSED---------------------------" << endl;
	cout << timeNow() << endl;
}

bool IrcBot::isConnected(string buf) {
//returns true if "/MOTD" is found in the input strin
	//If we find /MOTD then its ok join a channel
	if (buf.find("/MOTD", 0) != string::npos)
		return true;
	else
		return false;
}

string IrcBot::timeNow()
{//returns the current date and time
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	return asctime(timeinfo);
}


bool IrcBot::sendData(string raw) {
	string msg = raw + "\r\n";
	//Send some data
	int bytes_sent = send(_socket, msg.c_str(), msg.length(), 0);

	if (bytes_sent == 0)
		return false;
	else {
		cout << "< " + msg.substr(0, msg.length() - 2) << endl;
		return true;
	}
}

void IrcBot::processLine(string buf)
{
	/*
	 * TODO: add you code to respod to commands here
	 * the example below replys to the command hi scooby
	 */
	if(buf.find("PING", 0) == 0) {
		sendData("PONG" + buf.substr(4, string::npos) );
		return;
	}
	if(buf.find("test1", 0) != string::npos) {
		sendData("PRIVMSG #" + _chan + " :test one");
		return;
	}
	if(buf.find("test2", 0) != string::npos) {
		sendData("PRIVMSG #" + _chan + " :test two");
		return;
	}
	if(buf.find("test3", 0) != string::npos) {
		sendData("PRIVMSG #" + _chan + " :test three");
		return;
	}
	if(buf.find(":shatnames", 0) != string::npos) {
		sendData("NAMES #" + _chan);
		return;
	}
	if(buf.find("Found your hostname", 0) != string::npos) {
		sendData("NICK " + _nick);
		sendData("USER " + _nick + " 0 * :" + _real);
		return;
	}
	if(buf.find(_nick + " MODE " + _nick, 0) != string::npos) {
		sendData("JOIN #" + _chan);
		return;
	}
	size_t names_list_pos = buf.find(_nick + " = #" + _chan, 0);
	if(names_list_pos != string::npos) {
		size_t colon_pos = buf.find(":", names_list_pos);
		vector<string> tokens;
		string names = buf.substr(colon_pos + 1, string::npos);
		split(tokens, names, is_any_of(" ") );
		_chan_members = tokens;
		for(vector<string>::const_iterator cmi = _chan_members.begin(); cmi != _chan_members.end(); ++cmi)
			cout << "member: " << *cmi << endl;
	}
	if(buf.find("shatgreet", 0) != string::npos) {
		string sg = "Hello ";
		for(vector<string>::const_iterator cmi = _chan_members.begin(); cmi != _chan_members.end(); ++cmi) {
			if(cmi == _chan_members.begin() )
				sg = sg + *cmi;
			else if(cmi == --_chan_members.end() )
				sg = sg + ", and " + *cmi;
			else
				sg = sg + ", " + *cmi;
		}
		sendData("PRIVMSG #" + _chan + " :" + sg);
	}
}
