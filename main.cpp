/*
 * main.cpp
 *
 * MIT License (MIT)
 * Copyright (c) 2014 Mike Teehan <mike.teehan@gmail.com>
 */

#include <iostream>
#include "IrcBot.h"

using namespace std;

int main()
{
	IrcBot bot = IrcBot("irc.freenode.org", "6667", "lgc-weekly", "ShatBot", "ShatBot Maximus");
	bot.start();

	return 0;
}
