#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <thread>

/**
 * @brief Create a new socket for a TCP connection.
 * 
 * @return int - The socket descriptor, or -1 if creation fails.
 */

int createSocket() {
    int soc = socket(AF_INET, SOCK_STREAM, 0); // Create a new socket
    if (soc < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }
    return soc;
}

/**
 * @brief Connects to the server using the provided socket, IP, and port.
 * 
 * @param soc The socket descriptor.
 * @param ip The IP address of the server.
 * @param port The port number of the server.
 * @return True if connection is successful, false otherwise.
 */

 bool connectToServer(int soc, const char* ip, int port) {
    sockaddr_in server;
    server.sin_family = AF_INET; // Set address family to AF_INET=(IPv4)
    server.sin_port = htons(port); // Set the port number using host byte order
    if (inet_pton(AF_INET, ip, &server.sin_addr) <= 0) { // Convert IP address to binary form
        std::cerr << "Invalid address/ Address not supported" << std::endl; // Print error message
        return false; // Return false
    }
    if (connect(soc, (sockaddr*)&server, sizeof(server)) < 0) { // Connect to the server
        std::cerr << "Connection Failed" << std::endl; // Print error message
        return false; // Return false
    }
    return true; // Return true if connection is successful
}

/**
* @param soc The socket descriptor.
* @param data The data to be sent.
* @return True if the data was sent successfully, false otherwise.
*/

bool sendData(int soc, const std::string& data) {
    ssize_t sent = send(soc, data.c_str(), data.length(), 0); //Send the data through the socket

    if (sent < 0) { // Check if the data was sent successfully
        std::cerr << "Failed to send data" << std::endl;
        return false;
    }
    return true;
}

/**
* @brief Receives data from the socket and returns it as a string.
* @param soc The socket descriptor.
* @return The received data as a string.
*/

void receiveData(int soc) {

    while(true){
        char buffer[1024];  // Buffer to store received data
        memset(buffer, 0, sizeof(buffer));  // Clear the buffer
        int bytesReceived = recv(soc, buffer, sizeof(buffer) - 1, 0);  // Receive data from the socket
        if (bytesReceived <= 0) {  // Check for receive error
            std::cerr << "Failed closed" << std::endl;  // Print error message
            break;  // Return empty string on error
        }
        std::cout<<buffer <<std::endl;  // Return received data as string
    }
    
}

int main() {

    std::string clientName;
    std::cout<<"Enter your name: ";
    std::getline(std::cin, clientName);

    const char* serverIP = "127.0.0.1"; // Server IP address
    int serverPort = 11111; // Server port

    int soc = createSocket();
    if (soc == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1; // Socket creation failed
    }

    if (!connectToServer(soc, serverIP, serverPort)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return -1; // Connection to server failed
    }

    send(soc, clientName.c_str(), clientName.length(), 0); //sends the client name to the server

    std::thread t(receiveData, soc); // Start receiving data in a separate thread

    std:: string message;

    while(true){
        std::getline(std::cin, message);
        if (message == "exit()") {
            break;
        }
        if (!sendData(soc, message)) {
            break; // Sending message failed
        }
    }
    
    if(t.joinable()){ // Check if the thread is still running
        t.join(); // Wait for the thread to finish
    }

    close(soc); // Close the socket
    return 0;
}
