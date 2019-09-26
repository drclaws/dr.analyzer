#pragma once

#include "BuffObject.hpp"


const bool DataTransportInit();

void DataTransportStop();

void SendBuff(BuffObject *info);
