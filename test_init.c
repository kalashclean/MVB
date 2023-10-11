/**
 * @file
 */

#include <stdio.h>
#include "mvbc_app_interface.h"

/**
 * Main entry for test application
 *
 * @param argc
 * @param argv
 *
 * @return 0
 */

int main(int argc, char* argv[])
{
	int rc = -1;
    int major, minor, patch;
    int pld_firmware_version;
    char MVBC_Dev[20] = "n/a";

    printf("MVBC Lib Test\n");

    rc = mvbc_get_library_version(&major, &minor, &patch);
    printf("Library version: %d.%d.%d RC[%d]\n", major, minor, patch,rc);

    rc = mvbc_get_pld_firmware_version(&pld_firmware_version);
    printf("PLD firmware version: %d RC[%d]\n", pld_firmware_version,rc);

    if (argc > 1)
    {
    	rc = mvbc_init(argv[1]);
    }
    else
    {
    	rc = mvbc_init(NULL);
    }
    return rc;
}
