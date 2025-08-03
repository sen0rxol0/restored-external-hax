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

IOUSBDeviceDescriptionRef IOUSBDeviceDescriptionCreateWithType(CFAllocatorRef, CFStringRef);

io_service_t get_service(const char *name, unsigned int retry)
{
	io_service_t service;
	CFDictionaryRef match = IOServiceMatching(name);

  for (i = 0; i < retry; i++) {
      CFRetain(match);
      service = IOServiceGetMatchingService(kIOMasterPortDefault, match);
      if (!service) {
          printf("Didn't find, trying again\n");
          sleep(1);
      } else {
          break;
      }
  }

	CFRelease(match);
	return service;
}

/* reversed from restored_external */
int
init_mux(void)
{
    int i;
    CFNumberRef n;
    CFMutableDictionaryRef dict;
    IOUSBDeviceDescriptionRef desc;
    IOUSBDeviceControllerRef controller;
    io_service_t service;

    desc = IOUSBDeviceDescriptionCreateWithType(kCFAllocatorDefault, CFSTR("standardMuxOnly"));
    if (!desc) {
        return -1;
    }
    IOUSBDeviceDescriptionSetSerialString(desc, CFSTR("ramdisk tool " __DATE__ " " __TIME__ ));

    controller = 0;
    while (IOUSBDeviceControllerCreate(kCFAllocatorDefault, &controller)) {
        printf("Unable to get USB device controller\n");
        sleep(3);
    }
    IOUSBDeviceControllerSetDescription(controller, desc);

    CFRelease(desc);
    CFRelease(controller);

    dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    i = 7;
    n = CFNumberCreate(NULL, kCFNumberIntType, &i);
    CFDictionarySetValue(dict, CFSTR("DebugLevel"), n);
    CFRelease(n);

    service = get_service("AppleUSBDeviceMux", 3);

    if (!service) {
        return -1;
    }

    i = IORegistryEntrySetCFProperties(service, dict);
    IOObjectRelease(service);
    CFRelease(dict);

    return i;
}

void
disable_watchdog()
{
  io_service_t service = get_service("IOWatchDogTimer", 3);

  if (service) {
    uint32_t zero = 0;
    CFNumberRef n = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &zero);
    IORegistryEntrySetCFProperties(service, n);
    IOObjectRelease(service);
  } else {
    printf("unable to create matching dictionary for class IOWatchDogTimer\n");
  }
}

int
main(int argc, char *argv[])
{

    disable_watchdog();

    if (init_mux()) {
        printf("USB init FAIL\n");
    } else {
        printf("USB init succeeded, continuing ...\n");
    }

    sleep(1);
    printf("\n\t\t\t\t\tStarted  H A K E\n");
    printf("\t\tCredits: @xerub, @daytonhasty\n");
    printf("\t\tModded by: @sen0rxol0\n");
    printf("\n\tCompiled: " __DATE__ " " __TIME__ "\n\n");
    sleep(5);
    printf(" #######  ##    ##\n");
    printf("##     ## ##   ## \n");
    printf("##     ## ##  ##  \n");
    printf("##     ## #####   \n");
    printf("##     ## ##  ##  \n");
    printf("##     ## ##   ## \n");
    printf(" #######  ##    ##\n");
    printf("\nConnect with an SSH tunnel on port 22\n\n");
    sleep(3);
    char *execve_params[] = { "micro_inetd", "22", "/sbin/sshd", "-i", NULL };
    // char *execve_params[] = { "micro_inetd", "22", "/usr/local/bin/dropbear", "-i", NULL };
    // char *execve_params[] = { "micro_inetd", "22", "/bin/bash", "-c", "/usr/local/bin/dropbear -i", NULL };
    return main2(4, execve_params);
}
