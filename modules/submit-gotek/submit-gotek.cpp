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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "submit-gotek.hpp"
#include "gotekCTRLDialougue.hpp"
#include "gotekDATADialougue.hpp"


#include "Download.hpp"
#include "DownloadBuffer.hpp"
#include "Utilities.hpp"
#include "SubmitManager.hpp"
#include "LogManager.hpp"
#include "DownloadBuffer.hpp"
#include "Config.hpp"
#include "SocketManager.hpp"

#include "DNSManager.hpp"
#include "DNSResult.hpp"

using namespace nepenthes;

#ifdef STDTAGS 
#undef STDTAGS 
#endif
#define STDTAGS l_sub | l_hlr

Nepenthes *g_Nepenthes;
GotekSubmitHandler *g_GotekSubmitHandler;

GotekSubmitHandler::GotekSubmitHandler(Nepenthes *nepenthes)
{

	m_ModuleName        = "submit-gotek";
	m_ModuleDescription = "send files to a gote";
	m_ModuleRevision    = "$Rev$";
	m_Nepenthes = nepenthes;

	m_SubmitterName = "submit-file";
	m_SubmitterDescription = "store with md5sum as name in /tmp";

	g_Nepenthes = nepenthes;
	g_GotekSubmitHandler = this;
}

GotekSubmitHandler::~GotekSubmitHandler()
{

}

bool GotekSubmitHandler::Init()
{
	logPF();

	if ( m_Config == NULL )
	{
		logCrit("%s","I need a config\n");
		return false;
	}

	try
	{
		m_GotekHost = m_Config->getValString("submit-gotek.host");
        m_GotekPort = (uint16_t) m_Config->getValInt("submit-gotek.port");

		m_User = m_Config->getValString("submit-gotek.user");
		m_CommunityKey = (unsigned char *)m_Config->getValString("submit-gotek.communitykey");

    } catch ( ... )
	{
		logCrit("%s","Error setting needed vars, check your config\n");
		return false;
	}

	g_Nepenthes->getDNSMgr()->addDNS(this,(char *)m_GotekHost.c_str(), NULL);
    
	m_ModuleManager = m_Nepenthes->getModuleMgr();
	REG_SUBMIT_HANDLER(this);

	m_CTRLSocket = NULL;
	return true;
}

bool GotekSubmitHandler::Exit()
{
	return true;
}

void GotekSubmitHandler::Submit(Download *down)
{
	logPF();
	GotekContext *ctx = new GotekContext;

	logWarn("GOTEK Submission %s \n",down->getMD5Sum().c_str());
	ctx->m_EvCID = 0;//down->getEvCID();
	ctx->m_FileSize = down->getDownloadBuffer()->getSize();
	ctx->m_FileBuffer = (unsigned char *)malloc(ctx->m_FileSize);
	memcpy(ctx->m_FileBuffer,down->getDownloadBuffer()->getData(),ctx->m_FileSize);
	memcpy(ctx->m_SHA512Hash,down->getSHA512(),64);

	g_Nepenthes->getUtilities()->hexdump(ctx->m_SHA512Hash,64);


	m_Goten.push_back(ctx);


    if (m_CTRLSocket != NULL)
	{
		unsigned char request[1+64+8];
		memset(request,0,1+64+8);
		request[0] = 0x01;
		memcpy(request+1,ctx->m_SHA512Hash,64);
		memcpy(request+1+64,&ctx->m_EvCID,8);

		m_CTRLSocket->doWrite((char *)request,1+64+8);
	}else
	{
		// shit happens for now
		logCrit("CTRLSocket is %x\n",m_CTRLSocket);
	}

	return;
}


void GotekSubmitHandler::Hit(Download *down)
{
	Submit(down);
	return;
}


string GotekSubmitHandler::getUser()
{
	return m_User;
}

unsigned char* GotekSubmitHandler::getCommunityKey()
{
	return m_CommunityKey;
}

void GotekSubmitHandler::setSessionKey(uint64_t key)
{
	logInfo("Gotek Session key is 0x%08x%08x\n", (uint32_t)(key >> 32), (uint32_t)(key & 0xffffffff));
	m_Sessionkey = key;
}


bool GotekSubmitHandler::dnsResolved(DNSResult *result)
{
	logInfo("url %s resolved %i\n",result->getDNS().c_str(), result->getIP4List().size());

	list <uint32_t> resolved = result->getIP4List();
	uint32_t host = resolved.front();

	Socket *socket = g_Nepenthes->getSocketMgr()->connectTCPHost(0,host,m_GotekPort,14400);
	socket->addDialogue(new gotekCTRLDialogue(socket));

	m_GotekHostIP = host;
	return true;
}

bool GotekSubmitHandler::dnsFailure(DNSResult *result)
{

	return true;

}

void GotekSubmitHandler::setSocket(Socket *s)
{
	m_CTRLSocket = s;
}


bool GotekSubmitHandler::popGote()
{
	logPF();
	m_Goten.pop_front();
	return true;
}

bool GotekSubmitHandler::sendGote()
{
	logPF();
	GotekContext *ctx = m_Goten.front();

	Socket *socket = g_Nepenthes->getSocketMgr()->connectTCPHost(0,m_GotekHostIP,m_GotekPort,30);
	socket->addDialogue(new gotekDATADialogue(socket,ctx));
	popGote();
	return true;
}


extern "C" int32_t module_init(int32_t version, Module **module, Nepenthes *nepenthes)
{
	if(version == MODULE_IFACE_VERSION)
	{
		*module = new GotekSubmitHandler(nepenthes);
		return 1;
	} else
	{
		return 0;
	}
}