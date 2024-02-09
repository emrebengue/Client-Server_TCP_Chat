#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>


/**
* Create a socket using IPv4 and TCP protocol.
*
* @return the created socket, or -1 if failed to create socket
**/

int createSocket() {


    int soc = socket(AF_INET, SOCK_STREAM, 0);

    if(soc < 0){
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }
    return soc;
}

/**
 * Establishes a connection with a server using the provided socket, IP address, and port number.
 * 
 * @param soc The socket to use for the connection
 * @param ip The IP address of the server
 * @param port The port number of the server
 * @return True if the connection is established successfully, otherwise false
 **/

bool serverConnection(int soc, const char* ip, int port) {

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server.sin_addr);

    int con = connect(soc, (sockaddr*)&server, sizeof(server));

    if(con < 0){
        std::cerr << "Failed to establish connection" << std::endl;
        return false;
    }

    return true;
}

/**
 * Sends data over the specified socket.
 * 
 * @param soc The socket to send data over
 * @param data The data to be sent
 * @return true if the data was sent successfully, false otherwise
 **/

bool sendData(int soc, std::string& data) {

    ssize_t sent = send(soc, data.c_str(), data.length(), 0);
    if(sent < 0){
        std::cerr << "Failed to send data" << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Receive data from the specified socket
 * 
 * @param soc The socket to receive data from
 * @return The received data as a string
 **/
 
std::string receiveData(int soc) {

    char buffer[1024]; 
    memset(buffer, 0, sizeof(buffer));

    // Attempt to receive data from the socket
    int received = recv(soc, buffer, sizeof(buffer) - 1, 0); 
    if (received < 0) {
        std::cerr << "Failed to receive data" << std::endl;
        return ""; 
    }

    return std::string(buffer, received);
}


int main() {
    
    return 0;
}