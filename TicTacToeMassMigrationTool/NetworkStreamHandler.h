#pragma once
#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>

#include "BaseTypes.h"

ByteVector getData(std::string IP, uint32_t port);
void sendData(std::string IP, uint32_t port, ByteVector data);