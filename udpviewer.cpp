#include "CImg.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define PORT	 8080 

using namespace cimg_library;


void* viewer() {
    CImg<unsigned char> img(640,480,1,3,0);
    CImgDisplay disp(img,"UDP Image Viewer");
    const unsigned char white[] = { 255,255,255 }, red[] = { 128,0,0 };
    int sockfd; 
	struct sockaddr_in servaddr, cliaddr; 
	int len, n; 
	
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(PORT); 
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
    unsigned int bytes = 0, err = 0, sync = 0, frames = 0;
    while (!disp.is_closed() && !disp.is_keyQ() && !disp.is_keyESC()) {

	    n = recvfrom(sockfd, (unsigned char *)img.data()+bytes,img.size()-bytes, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				( socklen_t *) &len); 

        
        // check for sync pkt (len = 4), reset bytes
        if ( n == 4 ) {
            if ( sync == 1 && bytes != 0 ) {
                err++;
                printf("[%u] : ERROR Sync pkt receieved with buffer at %u/%u bytes\n",err,bytes,(unsigned int) img.size());
            }
            bytes = 0;
            sync = 1;
            continue;
        }

        bytes += n;

        //printf("[%u] : Receieved %u bytes, %u/%u\n",pkts++,n,bytes,img.size());
        
        // Display if full frame is recieved
        if ( bytes == img.size() ) {
            frames++;
            img.draw_text(5,5," %u frames/s\n %ux%u\n ip %s\n port %d\n %u bytes/udp\n %u frames received\n %u frame errors",white,red,0.5f,13,(unsigned int)disp.frames_per_second(),img.width(),img.height(),inet_ntoa(cliaddr.sin_addr),(int) ntohs(cliaddr.sin_port),n,frames,err);
            disp.display(img);
            bytes = 0;
        }

    }

    return 0;
}

void* bar() {
    CImg<unsigned char> img(640,480,1,3,0); 
                  
	int sockfd; 
	struct sockaddr_in	 servaddr; 

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(PORT); 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	

    cimg_forXY(img,x,y) 
        if (x < 160) {
            img.fillC(x,y,0,0,0,0);
        } else if ( x < 320 ) {
            img.fillC(x,y,0,255,0,0);
        } else if ( x < 480 ) {
            img.fillC(x,y,0,0,255,0);
        } else {
            img.fillC(x,y,0,0,0,255);
        }
    //img.display();
    
    while (1) {

        // send a frame sync packet
        if (sendto(sockfd, (unsigned char *) "sync", 4, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
	        perror("sendto failed");
	        return 0;
        }

        // Send single image
        unsigned int bytes = 0;
        while (bytes < img.size()) {
            if (sendto(sockfd, (unsigned char *) img.data()+bytes, 2048, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
	            perror("sendto failed");
	            return 0;
            }
            bytes += 2048;
            usleep(1);
        }
        //img.display();
        usleep(50000);
    }
    return 0;
}

/*---------------------------

  Main procedure

  --------------------------*/
int main(int argc, char **argv) {
  cimg_usage("View UDP camera streams or generate UDP bar test stream");
    
  const bool genbar = cimg_option("-bar",false,"Generate a bar pattern");
  const unsigned int h = cimg_option("-height",480,"Image height");
  const unsigned int w = cimg_option("-width",640,"Image witdh");
  const unsigned int p = cimg_option("-port",8080,"UDP Port");
  const char* ip = cimg_option("-port","127.0.0.1","Server IP address");

  if ( genbar )
      bar();
  else
    viewer();

  // Exit demo
  //-----------
  std::exit(0);
  return 0;
}
