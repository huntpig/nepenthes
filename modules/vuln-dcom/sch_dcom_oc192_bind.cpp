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

#include <netinet/in.h>

#include "LogManager.hpp"
#include "Message.hpp"
#include "sch_dcom_oc192_bind.hpp"
#include "Socket.hpp"
#include "Nepenthes.hpp"
#include "Utilities.hpp"
#include "DialogueFactoryManager.hpp"
#include "SocketManager.hpp"


#ifdef STDTAGS 
#undef STDTAGS 
#endif
#define STDTAGS l_sc | l_hlr

using namespace nepenthes;

OC192Bind::OC192Bind(ShellcodeManager *shellcodemanager)
{
	m_ShellcodeManager = shellcodemanager;
	m_ShellcodeHandlerName = "OC192Bind";
	m_ShellcodeHandlerDescription = "handles oc192 dcom bindshell";
	m_pcre = NULL;
}

OC192Bind::~OC192Bind()
{

}

bool OC192Bind::Init()
{
	logPF();
	const char *oc192bindpcre = ".*(\\x46\\x00\\x58\\x00\\x4E\\x00\\x42\\x00\\x46\\x00\\x58\\x00\\x46\\x00\\x58\\x00\\x4E\\x00\\x42\\x00\\x46\\x00\\x58\\x00\\x46\\x00\\x58\\x00\\x46\\x00\\x58\\x00\\x46\\x00\\x58\\x00.*\\x6a\\x6d\\xca\\xdd\\xe4\\xf0\\x90\\x80\\x2f\\xa2\\x04).*";

    


	logInfo("pcre is %s \n",oc192bindpcre);
    
	const char * pcreEerror;
	int pcreErrorPos;
	if((m_pcre = pcre_compile(oc192bindpcre, PCRE_DOTALL, &pcreEerror, &pcreErrorPos, 0)) == NULL)
	{
		logCrit("OC192Bind could not compile pattern \n\t\"%s\"\n\t Error:\"%s\" at Position %u", 
				oc192bindpcre, pcreEerror, pcreErrorPos);
		return false;
	}
	return true;
}

bool OC192Bind::Exit()
{
	if(m_pcre != NULL)
    	free(m_pcre);
	return true;

}

sch_result OC192Bind::handleShellcode(Message **msg)
{
	logPF();
	logSpam("Shellcode is %i bytes long \n",(*msg)->getMsgLen());
	char *shellcode = (*msg)->getMsg();
	unsigned int len = (*msg)->getMsgLen();

	int piOutput[10 * 3];
	int iResult; 

//	(*msg)->getSocket()->getNepenthes()->getUtilities()->hexdump((unsigned char *)shellcode,len);




	if ((iResult = pcre_exec(m_pcre, 0, (char *) shellcode, len, 0, 0, piOutput, sizeof(piOutput)/sizeof(int))) > 0)
	{
//		g_Nepenthes->getUtilities()->hexdump((unsigned char *)shellcode,len);
		const char * pCode;
		unsigned short usPort;
		unsigned long ulPort;

		pcre_get_substring((char *) shellcode, piOutput, iResult, 1, &pCode);

		ulPort = * ((unsigned long *) &pCode[471]) ^ 0x9432BF80;

		memcpy(&usPort,(char *)&ulPort+1,2);
		usPort = ntohs(usPort);
        logInfo("Detected oc192 listenshell shellcode, :%u \n", usPort);
		pcre_free_substring(pCode);

		Socket *socket;
		if ((socket = g_Nepenthes->getSocketMgr()->bindTCPSocket(0,usPort,60,30)) == NULL)
		{
			logCrit("%s","Could not bind socket %u \n",usPort);
			return SCH_DONE;
		}
		
		DialogueFactory *diaf;
		if ((diaf = g_Nepenthes->getFactoryMgr()->getFactory("WinNTShell DialogueFactory")) == NULL)
		{
			logCrit("%s","No WinNTShell DialogueFactory availible \n");
			return SCH_DONE;
		}

		socket->addDialogueFactory(diaf);
        return SCH_DONE;
	}
	return SCH_NOTHING;
}
