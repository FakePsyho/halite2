makefiles_are_stupid:
	g++ -std=gnu++14 -O2 MyBot.cpp -o bot.exe hlt/hlt_in.cpp hlt/location.cpp hlt/map.cpp