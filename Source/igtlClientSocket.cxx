/*=========================================================================

  Program:   The OpenIGTLink Library
  Language:  C++
  Web page:  http://openigtlink.org/

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkClientSocket.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "igtlClientSocket.h"
#include <winsock.h>

namespace igtl
{

//-----------------------------------------------------------------------------
ClientSocket::ClientSocket()
{
}

//-----------------------------------------------------------------------------
ClientSocket::~ClientSocket()
{
}

//-----------------------------------------------------------------------------
int ClientSocket::ConnectToServer(const char* hostName, int port)
{
  if (this->m_SocketDescriptor != -1)
    {
    igtlWarningMacro("Client connection already exists. Closing it.");
    this->CloseSocket(this->m_SocketDescriptor);
    this->m_SocketDescriptor = -1;
    }
  
  this->m_SocketDescriptor = this->CreateSocket();
  if (!this->m_SocketDescriptor)
    {
    igtlErrorMacro("Failed to create socket.");
    return -1;
    }

  if (this->Connect(this->m_SocketDescriptor, hostName, port) == -1)
    {
    this->CloseSocket(this->m_SocketDescriptor);
    this->m_SocketDescriptor = -1;

    igtlErrorMacro("Failed to connect to server " << hostName << ":" << port);
    return -1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
int ClientSocket::ConnectToServer(const char* hostName, int port, int msec)
{
	if (this->m_SocketDescriptor != -1)
	{
		igtlWarningMacro("Client connection already exists. Closing it.");
		this->CloseSocket(this->m_SocketDescriptor);
		this->m_SocketDescriptor = -1;
	}

	this->m_SocketDescriptor = this->CreateSocket();
	if (!this->m_SocketDescriptor)
	{
		igtlErrorMacro("Failed to create socket.");
		return -1;
	}

	/// Set the socket to the NON BLOCKING SOCKET
	unsigned long nonBlocking = 1;

	if (ioctlsocket(this->m_SocketDescriptor, FIONBIO, &nonBlocking) != 0)
	{
		igtlErrorMacro("Failed to set the socket to the non-blocking socket " << hostName << ":" << port);
		return -1;
	}

	this->Connect(this->m_SocketDescriptor, hostName, port);
	int ret = this->SelectSocket(this->m_SocketDescriptor, msec);
	if (ret == 0)
	{
		// Timed out.
		igtlErrorMacro("Connection timeout: msec <= " << msec);
		return -1;
	}
	if (ret == -1)
	{
		igtlErrorMacro("Error selecting socket.");
		return -1;
	}

	int error;
	int len = sizeof(error);
	getsockopt(m_SocketDescriptor, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
	if(error)
	{
		igtlErrorMacro("NonBlocking: Error checking to connection. Failed to connection");
		return -1;
	}
	
	/// Set the socket back to the BLOCKING SOCKET
	nonBlocking = 0;
	if (ioctlsocket(this->m_SocketDescriptor, FIONBIO, &nonBlocking) != 0)
	{
		igtlErrorMacro("Failed to set the socket back to the blocking socket " << hostName << ":" << port);
		return -1;
	}

	return 0;
}

//-----------------------------------------------------------------------------
void ClientSocket::PrintSelf(std::ostream& os) const
{
  this->Superclass::PrintSelf(os);
}

} // end of igtl namespace

