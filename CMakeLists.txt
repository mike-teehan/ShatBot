project(shatbot)

add_executable(shatbot main.cpp IrcBot.cpp)

install(TARGETS shatbot RUNTIME DESTINATION bin)
