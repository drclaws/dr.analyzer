#pragma once

#include "GatherInfo.hpp"


// Function of gathering process.
// Must run in special thread.
void GatherThreadFunc();

// Adds info to current buffer
void AddToBuff(GatherInfo* info);

// Indicates to end the gathering
void SetDisconnect();
