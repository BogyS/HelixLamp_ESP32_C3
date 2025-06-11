#include "FSDef.h"
#include "Config.h"
#include "serial_config.h"

void GetFSBytes( size_t& total, size_t& used )
{
    total = FILESYSTEM.totalBytes();
    used = FILESYSTEM.usedBytes();
}
bool StartFS( bool formatFlag )
{
    if (FILESYSTEM.begin())
        return true;
    serial_println("ERROR on mounting filesystem!");
    if( formatFlag )
    {
        serial_println("It will be formmatted!");
        FILESYSTEM.format();
    }
    return false;
}
