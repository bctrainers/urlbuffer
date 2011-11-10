/* Copyright (C) 2011 uberspot
*
* Compiling: znc-buildmod urlbuffer.cpp 
* Dependencies: curl, wget, sed and a unix environment.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 3 as published
* by the Free Software Foundation (http://www.gnu.org/licenses/gpl.txt).
*/ 

#include "main.h"
#include "User.h"
#include "Nick.h"
#include "Modules.h"
#include "Chan.h" 
#include "FileUtils.h"
#include <pthread.h>
#include <climits>

#define MAX_EXTS 6
#define MAX_CHARS 16

class CUrlBufferModule : public CModule 
{
private: 
	VCString lastUrls;
	
	unsigned int linkNum;
	CString target;
	CString wcommand;
	static const string supportedExts[MAX_EXTS];
	static const char unSupportedChars[MAX_CHARS];

	static void* download(void *ptr);
	static inline CString getStdoutFromCommand(const string& cmd);
	inline void LoadDefaults();
	inline CString convertTime(const CString& str)
	{
		time_t curtime;
		tm* timeinfo;
		char buffer[1024];

		time(&curtime);
		curtime += (time_t) (m_pUser->GetTimezoneOffset() * 60 * 60);
		timeinfo = localtime(&curtime);

		if (!strftime(buffer, sizeof(buffer), str.c_str(), timeinfo))
		{
			return "";
		}
		return CString(buffer);
	}
	inline bool isValidExtension(CString ext)
	{
		ext.MakeLower();
		for(int i=0; i< MAX_EXTS; i++)
		{
			if( ext == supportedExts[i])
			{
				return true;
			}
		}
		return false;
	}
	inline bool isValidDir(const string& dir)
	{
		for(int i=0; i< MAX_CHARS; i++)
		{
			if (dir.find(unSupportedChars[i]) !=string::npos)
				return false;
		}
		return true;
	}
	static void* sendLinks(void *ptr);
	inline void CheckLineForLink(const CString& sMessage, const CString& sOrigin);
	inline void CheckLineForTrigger(const CString& sMessage, const CString& sTarget); 
public:
	MODCONSTRUCTOR(CUrlBufferModule) {}

	bool OnLoad(const CString& sArgs, CString& sErrorMsg);
	~CUrlBufferModule(); 
	EModRet OnUserMsg(CString& sTarget, CString& sMessage);
	EModRet OnPrivMsg(CNick& Nick, CString& sMessage);
	EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage);
	void OnModCommand(const CString& sCommand);
	
};

const string CUrlBufferModule::supportedExts[MAX_EXTS] = 
				{"jpg", "png", "gif", "jpeg", "bmp", "tiff"} ;

const char CUrlBufferModule::unSupportedChars[MAX_CHARS] = 
				{'|', ';', '!', '@', '#', '(', ')', '<', '>', '"',  '\'', '`', '~', '=', '&', '^'};

bool CUrlBufferModule::OnLoad(const CString& sArgs, CString& sErrorMsg) 
{
	LoadDefaults();
	return true;
}

CUrlBufferModule::~CUrlBufferModule() {}

CUrlBufferModule::EModRet CUrlBufferModule::OnUserMsg(CString& sTarget, CString& sMessage) 
{
	CheckLineForLink(sMessage, "");
	CheckLineForTrigger(sMessage, m_pUser->GetIRCNick().GetNick());
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnPrivMsg(CNick& Nick, CString& sMessage) 
{ 
	CheckLineForLink(sMessage, Nick.GetNick());
	CheckLineForTrigger(sMessage, Nick.GetNick());
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) 
{
	CheckLineForLink(sMessage, Nick.GetNick());
	CheckLineForTrigger(sMessage, Nick.GetNick());
	return CONTINUE;
}

void CUrlBufferModule::OnModCommand(const CString& sCommand) 
{
	CString command = sCommand.Token(0).AsLower().Trim_n();
	
	if (command == "help") 
	{
		CTable CmdTable;

		CmdTable.AddColumn("Command");
		CmdTable.AddColumn("Description");

		CmdTable.AddRow();
		CmdTable.SetCell("Command", "ENABLE");
		CmdTable.SetCell("Description", "Activates link buffering.");

		CmdTable.AddRow();
		CmdTable.SetCell("Command", "DISABLE");
		CmdTable.SetCell("Description", "Deactivates link buffering.");

		CmdTable.AddRow();
		CmdTable.SetCell("Command", "ENABLELOCAL");
		CmdTable.SetCell("Description", "Enables downloading of each link to local directory.");

		CmdTable.AddRow();
		CmdTable.SetCell("Command", "DISABLELOCAL");
		CmdTable.SetCell("Description", "Disables downloading of each link to local directory.");

		CmdTable.AddRow();
		CmdTable.SetCell("Command", "DIRECTORY <#directory>");
		CmdTable.SetCell("Description", "Sets the local directory where the links will be saved.");
		
		CmdTable.AddRow();
		CmdTable.SetCell("Command", "CLEARBUFFER");
		CmdTable.SetCell("Description", "Empties the link buffer.");
		
		CmdTable.AddRow();
		CmdTable.SetCell("Command", "BUFFERSIZE <#size>");
		CmdTable.SetCell("Description", "Sets the size of the link buffer. Only integers >=0.");
		
		CmdTable.AddRow();
		CmdTable.SetCell("Command", "SHOWSETTINGS");
		CmdTable.SetCell("Description", "Prints all the settings.");

		CmdTable.AddRow();
		CmdTable.SetCell("Command", "HELP");
		CmdTable.SetCell("Description", "This help.");

		PutModule(CmdTable);
		return;
	}else if (command == "enable") 
	{
		SetNV("enable","true",true);
		PutModule("Enabled buffering");
	}else if (command == "disable") 
	{
		SetNV("enable","false",true);
		PutModule("Disabled buffering");
	}else if (command == "enablelocal")
	{
		if(GetNV("directory") == "")
		{
			PutModule("Directory is not set. First set a directory and then enable local caching");
		}
		SetNV("enablelocal","true",true);
		PutModule("Enabled local caching");
	}else if (command == "disablelocal")
	{
		SetNV("enablelocal", "false", true);
		PutModule("Disabled local caching");
	}else if (command == "directory") 
	{
		CString dir=sCommand.Token(1).Replace_n("//", "/").TrimRight_n("/") + "/";
		if (!isValidDir(dir))
		{
			PutModule("Error in directory name. Avoid using: | ; ! @ # ( ) < > \" ' ` ~ = & ^ <space> <tab>");
			return;
		}
		// Check if file exists and is directory
		if (dir.empty() || !CFile::Exists(dir) || !CFile::IsDir(dir, false))
		{
			PutModule("Invalid log path or no write access to ["+ sCommand.Token(1) +"].");
			return;
		} 
		SetNV("directory", dir, true);
		PutModule("Directory for local caching set to " + GetNV("directory"));
	}else if (command == "clearbuffer")
	{
		lastUrls.clear();
	}else if (command == "buffersize")
	{
		unsigned int bufSize = sCommand.Token(1).ToUInt();
		if(bufSize==0 || bufSize==UINT_MAX)
		{
			PutModule("Error in buffer size. Use only integers >= 0.");
			return;
		}
		SetNV("buffersize", CString(bufSize), true);
		PutModule("Buffer size set to " + GetNV("buffersize")); 
	}else if (command == "showsettings")
	{
		for(MCString::iterator it = BeginNV(); it != EndNV(); it++)
		{
			PutModule(it->first.AsUpper() + " : " + it->second);
		}
	}else
	{
		PutModule("Unknown command! Try HELP.");
		return;
	}
}

void CUrlBufferModule::LoadDefaults() 
{
	if(GetNV("enable")==""){
		SetNV("enable", "true", true);
	}
	if(GetNV("enablelocal")==""){
		SetNV("enablelocal", "false", true);
	}
	if(GetNV("buffersize")== ""){
		SetNV("buffersize", "5", true);
	}
}

void CUrlBufferModule::CheckLineForLink(const CString& sMessage, const CString& sOrigin)
{
	if(sOrigin != m_pUser->GetIRCNick().GetNick() && GetNV("enable").ToBool() )
	{	
		VCString words;
		CString output;
		sMessage.Split(" ", words, false,"", "", true, true);
		for (size_t a = 0; a < words.size(); a++) 
		{
			const CString& word = words[a];
			if(word.Left(4) == "http")
			{            
				//if you find an image download it, save it in the www directory and keep the new link in buffer
				VCString tokens;
				word.Split("/", tokens, false, "", "", true, true);
				string name = tokens[tokens.size()-1];
				word.Split(".", tokens, false, "", "", true, true);

				if(isValidExtension( tokens[tokens.size()-1] ))
				{

					std::stringstream ss;
					if( GetNV("enablelocal").ToBool())
					{
						CString dir = GetNV("directory") + convertTime(CString("%d-%m-%Y")) + "/";
						if(!CFile::Exists(dir) && !CFile::IsDir(dir, false))
						{
							CDir::MakeDir(dir, 0755);
						}
						ss << "wget -b -O " << dir.c_str() << name <<" -q " << word.c_str() << " 2>&1";
						pthread_t thread;
						wcommand = ss.str();
						pthread_create( &thread, NULL, &download, this);
					}
					ss.str("");
					ss << "curl -d \"image=" << word.c_str() << "\" -d \"key=5ce86e7f95d8e58b18931bf290f387be\" http://api.imgur.com/2/upload.xml | sed -n 's/.*<original>\\(.*\\)<\\/original>.*/\\1/p' 2>&1";
					output = getStdoutFromCommand(ss.str());
					lastUrls.push_back(output);
				}
			}
		}
	}
}

void *CUrlBufferModule::download(void *ptr) 
{
	CUrlBufferModule *caller = static_cast<CUrlBufferModule*>(ptr);
	getStdoutFromCommand(caller->wcommand);
	return NULL;
}

CString CUrlBufferModule::getStdoutFromCommand(const string& cmd) 
{
	string data="";
	char buffer[128];
	FILE* stream = popen(cmd.c_str(), "r");
	if (stream == NULL || !stream || ferror(stream))
	{ 
		return CString("Error!");
	}
	while (!feof(stream))
	{
		if (fgets(buffer, 128, stream) != NULL)
		{
			data.append(buffer);
		}
	}
	pclose(stream);
	return CString(data);
}

void *CUrlBufferModule::sendLinks(void *ptr)
{
	CUrlBufferModule *caller = static_cast<CUrlBufferModule*> (ptr);
	vector<CString> links = caller->lastUrls;
	unsigned int maxSize = links.size()-1;

	for(unsigned int i=0; i<=maxSize && i<caller->linkNum; i++)
	{
		sleep(2);
		caller->PutIRC("PRIVMSG " + caller->target + " :" + links[maxSize-i]);
	}
	return NULL;
}

void CUrlBufferModule::CheckLineForTrigger(const CString& sMessage, const CString& sTarget)
{
	VCString words;
	sMessage.Split(" ", words, false, "", "", true, true);
	for (size_t a = 0; a < words.size(); a++) 
	{
		CString& word = words[a];	
		if(word.AsLower() == "!showlinks")
		{
			if(lastUrls.empty())
			{
				PutIRC("PRIVMSG " + sTarget + " :No links were found...");
			}else 
			{
				unsigned int maxLinks = GetNV("buffersize").ToUInt();
								
				if (a+1 < words.size())
				{
					unsigned int size = words[a+1].ToUInt();
					if(size!=0 && size<UINT_MAX) //if it was a valid number
					{
						maxLinks = size;
					}
				}
				linkNum = maxLinks;
				target = sTarget;

				pthread_t thread;
				pthread_create( &thread, NULL, &sendLinks, this);
			}
		}
	}
}

MODULEDEFS(CUrlBufferModule, "Module that caches images from links posted on irc channels.")

