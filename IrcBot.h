/*
 * IrcBot.h
 *
 * MIT License (MIT)
 * Copyright (c) 2014 Mike Teehan <mike.teehan@gmail.com>
 */
 
#ifndef IRCBOT_H_
#define IRCBOT_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

class IrcBot
{
public:
	IrcBot(string host, string port, string chan, string nick, string real);
	virtual ~IrcBot();

	void start();

private:
	int		_socket; //the socket descriptor

	string	_host;
	string	_port;
	string	_chan;
	string	_nick;
	string	_real;
	vector<string>	_chan_members;
	
	void connectSocket();
	bool isConnected(string buf);
	string timeNow();
	bool sendData(string msg);
	void processLine(string buf);
};

#endif /* IRCBOT_H_ */
