#pragma once

#include "BuffObject.hpp"


// Initializes thread for data transporting process
const bool DataTransportInit();

// Indicates to stop data transporting and wait for thread exit
void DataTransportStop();

// Adds buff to queue for sending
void SendBuff(BuffObject *info);
