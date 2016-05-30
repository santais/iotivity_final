/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "networkhandler.h"

#include <unistd.h>

#include "logger.h"
#include "easysetup.h"
#include "oic_string.h"

// ioctl libraries

#include <stdio.h>

#include <string.h> /* for strncpy */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#define LOG_TAG "LINUX ES"

const char *gSsid = "EasySetup123";
const char *gPass = "EasySetup123";
char *gIpAddress;
ESEnrolleeNetworkEventCallback gNetworkEventCb;

static bool g_connectedToWiFi = false;

pthread_t g_wpaSupplicantThread;

void *wpaSupplicantThread(void *no)
{
    printf("Connecting to network up the wifi\n");
    
    system("sudo wpa_supplicant -Dnl80211 -iwlan0 -c/etc/wpa.conf");
}

void createWpaConf()
{
    char buf[150];
    // Remove previous wpa config files
    system("sudo rm /etc/wpa.conf");
    snprintf(buf, sizeof buf, "%s%s%s%s%s", "sudo sh -c 'wpa_passphrase \"", gSsid, "\" ", gPass, " \> /etc/wpa.conf'");
    printf("%s\n", buf);
    system(buf);
}

static bool ESConnectToWiFi()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        printf("error\n");
        //exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name,"wlan0")==0)&&(ifa->ifa_addr->sa_family==AF_INET))
        { //eno16777736
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n",ifa->ifa_name );
            printf("\t  Address : <%s>\n", host);

            g_connectedToWiFi = true;
            gNetworkEventCb(ES_OK);
        }
    }

    freeifaddrs(ifaddr);

    if(g_connectedToWiFi)
    {
	
        return true;
    }
    return false;
}

/*
 * All the functions defined in this file are stub functions to be implemented by the end user of
 * Easysetup Enrollee applications.
 */
static void ESActivateWifi()
{
    if(g_wpaSupplicantThread)
    {
        printf("\n\n\tThread already exist! Cancelling\n\n");
        pthread_cancel(g_wpaSupplicantThread);
    }

    if(pthread_create(&g_wpaSupplicantThread, NULL, wpaSupplicantThread, NULL))
    {
	printf("Thread creation failed \n");
        return false;
    }

   // Optain an IP address and do something
    system("sudo iw wlan0 link");
    system("sudo dhclient wlan0");

    return true;
}

static bool start()
{
    OIC_LOG(INFO, LOG_TAG, "START");
    createWpaConf();
    ESActivateWifi();
    while(!ESConnectToWiFi())
    {
        printf("Unable to connect to network. Retrying...\n");
        sleep(2);
    }
    return true;
}

bool ConnectToWiFiNetwork(const char *ssid, const char *pass,
                                                        ESEnrolleeNetworkEventCallback cb)
{
    OIC_LOG_V(INFO, LOG_TAG, "ConnectToWiFiNetwork %s %s",ssid,pass);
    printf("Inside ConncetToWifiNetwork \n");
    gPass = pass;
    gSsid = ssid;
    gNetworkEventCb = cb;
    return(start());
}

ESResult getCurrentNetworkInfo(OCConnectivityType targetType, NetworkInfo *info)
{
    if (targetType == CT_ADAPTER_IP)
    {
        info->type = CT_ADAPTER_IP;
        info->ipaddr = gIpAddress;
        if (strlen(gSsid) <= MAXSSIDLEN)
        {
            OICStrcpy(info->ssid, sizeof(info->ssid), gSsid);
            return ES_OK;
        }
        else
        {
            return ES_ERROR;
        }
    }

    return ES_ERROR;
}

