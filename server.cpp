#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <algorithm>
#include <vector>
#include <mutex>


std::vector<int> clientSockets; // List of connected client sockets
std::mutex clientsMutex; // Mutex to protect access to clientSockets

/**
 * @brief Create a new socket for a TCP connection.
 * 
 * @return int - The socket descriptor, or -1 if creation fails.
 **/
 
int createSocket() {

    // Create a new socket
    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }
    return soc;
}

/**
 * Binds the socket to the specified port and listens for incoming connections.
 * 
 * @param soc The socket file descriptor
 * @param port The port number to bind to
 * @return true if the socket is successfully bound and listening, false otherwise
 **/

bool bindAndListen(int soc, int port) {

    // Set up the server address
    sockaddr_in serverAddr; // Set up the server address
    memset(&serverAddr, 0, sizeof(serverAddr)); // Clear the server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on any network interface
    
    // Bind the socket to the server address
    if (bind(soc, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) { // Bind the socket to the server address
        std::cerr << "Failed to bind" << std::endl;
        return false;
    }

    if (listen(soc, 10) < 0) { // Listen for incoming connections with a backlog of 10
        std::cerr << "Failed to listen" << std::endl;
        return false;
    }

    return true;
}

/**
* Receives data from the client socket. 

* @param clientSocket The socket descriptor.
* @return The received data as a string, or an empty string if there was an error.
**/

std::string receiveData(int clientSocket) {

    char buffer[1024];  // Buffer to store received data
    memset(buffer, 0, sizeof(buffer));  // Clear the buffer
    int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);  // Receive data from client
    if (received < 0) {
        std::cerr << "Failed to receive data" << std::endl;  // Print error message
        return "";  // Return empty string
    }
    return std::string(buffer, received);  // Return received data as string
}

/**
 * Sends data to the client socket.
 * 
 * @param clientSocket The socket to which data will be sent
 * @param data The data to be sent
 * @return true if the data was sent successfully, false otherwise
 **/

bool sendData(int clientSocket, const std::string& data) {
    ssize_t sent = send(clientSocket, data.c_str(), data.length(), 0); // Send data
    if (sent < 0) {
        std::cerr << "Failed to send data" << std::endl; // Print error message
        return false; // Return false if failed to send data
    }
    return true; // Return true if data was sent successfully
}

/**
 * @brief Broadcasts a message to all client sockets, excluding the specified socket.
 * 
 * @param message The message to be broadcasted.
 * @param excludeSocket The socket to exclude from receiving the message.
 **/

void broadcastMessage(const std::string& message, int excludeSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex); // Protect access to clientSockets
    for (int socket : clientSockets) {
        if (socket != excludeSocket) { // Making sure message is not sent to the excluded socket (sender)
            send(socket, message.c_str(), message.length(), 0);
        }
    }
}

/**
 * @param clientSocket The socket descriptor.
 **/

void handleClient(int clientSocket) {
    char nameBuffer[1024]; // Buffer to store client's name
    int nameLength = recv(clientSocket, nameBuffer, sizeof(nameBuffer) - 1, 0); // Receive client's name
    if (nameLength > 0) {
        nameBuffer[nameLength] = '\0'; // Null-terminate the received name
        std::string welcomeMessage = std::string(nameBuffer) + " connected.";
        broadcastMessage(welcomeMessage, clientSocket); // Broadcast new client's name
    }

    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); // Receive data from client
        if (bytesReceived <= 0) { // Check for receive error or end of connection
            std::cerr << "Chat closed" << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string message = std::string(nameBuffer) + ": " + std::string(buffer); // Construct message
        broadcastMessage(message, clientSocket); // Broadcast message to all clients
    }
    
    std::lock_guard<std::mutex> lock(clientsMutex); // Protect access to clientSockets

    // Remove the client socket from the list
    clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
    
    close(clientSocket);
}

int main() {
    int soc = createSocket();
    if (soc == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }
    std::cout << "Server started" << std::endl;

    if (!bindAndListen(soc, 11111)) {
        return -1;
    }

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(soc, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSocket < 0) {
            std::cerr << "Accept error" << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket); // Add new client to the list
        }

        std::thread t(handleClient, clientSocket);
        t.detach(); // Detach the thread to handle multiple clients simultaneously
    }

    close(soc);
    return 0;
}
