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

#ifndef HAVE_DNSHANDLER_HPP
#define HAVE_DNSHANDLER_HPP

#include <string>

using namespace std;

namespace nepenthes
{
	class DNSResult;

	class DNSHandler
	{
		public:
		virtual ~DNSHandler(){};
        virtual bool dnsResolved(DNSResult *result)=0;
		virtual bool dnsFailure(DNSResult *result)=0;

		virtual string getDNSHandlerName()
		{
			return m_DNSHandlerName;
		}

	protected:
		string m_DNSHandlerName;
	};
}

#endif
