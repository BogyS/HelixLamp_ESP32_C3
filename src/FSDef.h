#pragma once

#include <FS.h>
#include <LittleFS.h>
#define FILESYSTEM LittleFS

void GetFSBytes( size_t& total, size_t& used );
bool StartFS( bool formatFlag ); 
