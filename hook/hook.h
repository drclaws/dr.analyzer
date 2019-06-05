#pragma once

// Prepares connection and info gathering
int StartGathering();

// Stops info gathering and closes connection
int StopGathering();

void GatherFileInfo(void *fileHandle);

void GatherLibraryInfo(void *libHandle);

void WarningFileNameToBig();

void WarningLibraryNameToBig();
