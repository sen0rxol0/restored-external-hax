#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include "IOKit/IOKitLib.h"
#include "IOKit/IOTypes.h"
#include "IOUSBDeviceControllerLib.h"

#include "micro_inetd.c"
char *execve_params[] = { "micro_inetd", "22", "/bin/bash", "-c", "/usr/local/bin/dropbear -i", NULL };
// char *execve_params[] = { "micro_inetd", "22", "/usr/local/bin/dropbear", "-i", NULL };
// char *execve_params[] = { "micro_inetd", "22", "/sbin/sshd", "-i", NULL };

IOUSBDeviceDescriptionRef IOUSBDeviceDescriptionCreateWithType(CFAllocatorRef, CFStringRef);

io_service_t
get_service(const char *name, unsigned int retry)
{
    io_service_t service;
    CFDictionaryRef match = IOServiceMatching(name);

    while (1)
		{
        CFRetain(match);
        service = IOServiceGetMatchingService(kIOMasterPortDefault, match);
        if (service || !retry--)
            break;
        printf("Didn't find %s, trying again\n", name);
        sleep(1);
    }

    CFRelease(match);
    return service;
}

/* reversed from restored_external */
int
init_usbmux(void)
{
    int i;
    CFNumberRef n;
		io_service_t service;
    CFMutableDictionaryRef dict;
    IOUSBDeviceDescriptionRef desc;
    IOUSBDeviceControllerRef controller;

    desc = IOUSBDeviceDescriptionCreateWithType(kCFAllocatorDefault, CFSTR("standardMuxOnly"));

		if (desc)
		{
				IOUSBDeviceDescriptionSetSerialString(desc, CFSTR("ramdisk tool " __DATE__ " " __TIME__ ));

				controller = 0;
				while (IOUSBDeviceControllerCreate(kCFAllocatorDefault, &controller))
				{
						printf("Unable to get USB device controller\n");
						sleep(3);
				}
				IOUSBDeviceControllerSetDescription(controller, desc);

				CFRelease(desc);
				CFRelease(controller);

				service = get_service("AppleUSBDeviceMux", 10);

				if (service)
				{
					dict = CFDictionaryCreateMutable(NULL, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

					i = 7;
					n = CFNumberCreate(NULL, kCFNumberIntType, &i);
					CFDictionarySetValue(dict, CFSTR("DebugLevel"), n);
					CFRelease(n);

					i = IORegistryEntrySetCFProperties(service, dict);
					CFRelease(dict);
					IOObjectRelease(service);
					return i;
				}
    }

		return -1;
}

void
disable_watchdog()
{
		io_service_t service = get_service("IOWatchDogTimer", 0);
		if (service)
		{
				 int zero = 0;
				 CFNumberRef n = CFNumberCreate(NULL, kCFNumberSInt32Type, &zero);
				 IORegistryEntrySetCFProperties(service, n);
				 CFRelease(n);
				 IOObjectRelease(service);
		}
}

int
main(int argc, char *argv[])
{

    disable_watchdog();

    if (init_usbmux()) {
        printf("USB init FAIL\n");
    } else {
        printf("USB init succeeded, continuing ...\n");
    }

    printf("\n\t\t\t\t\tStarted  H A K E\n");
    printf("\nCompiled " __DATE__ " " __TIME__ "\n\n");
    printf(" #######  ##    ##\n");
    printf("##     ## ##   ## \n");
    printf("##     ## ##  ##  \n");
    printf("##     ## #####   \n");
    printf("##     ## ##  ##  \n");
    printf("##     ## ##   ## \n");
    printf(" #######  ##    ##\n");
    printf("\nConnect with an SSH tunnel on port 22\n\n");
    sleep(3);
    return main2(4, execve_params);
}
