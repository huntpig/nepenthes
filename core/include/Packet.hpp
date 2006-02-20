/********************************************************************************
 *                              Nepenthes
 *                        - finest collection -
 *
 *
 *
 * Copyright (C) 2005  Paul Baecher & Markus Koetter
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * 
 *             contact nepenthesdev@users.sourceforge.net  
 *
 *******************************************************************************/

/* $Id$ */

#ifndef HAVE_PACKET_HPP
#define HAVE_PACKET_HPP

namespace nepenthes
{

	class Packet
	{
	public:
		Packet(char *pszData,unsigned int iLen);
		~Packet();
		char *getData();
		unsigned int getLength();
		bool cut(unsigned int offset);

	protected:
		char    *m_Data;
		unsigned int m_Length;
	};

	class UDPPacket : public Packet
	{
	public:
		UDPPacket(unsigned long ip, unsigned short port, char *pszData,unsigned int iLen);
		~UDPPacket();
		unsigned long getHost();
		unsigned short getPort();
	protected:
		unsigned long m_Host;
		unsigned short m_Port;
	};


}

#endif
