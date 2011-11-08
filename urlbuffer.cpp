/* Copyright (C) 2011 uberspot
 *
 * Compiling: znc-buildmod urlbuffer.cpp 
 * Dependencies: curl, wget, sed and a unix enviroment.
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
#include <pthread.h>

#define MAX_EXTS 6
#define MAX_CHARS 16

typedef map<const CString, CString> TSettings;
typedef vector<CString> SUrls;

void *download(void *ptr);
inline CString getStdoutFromCommand(string cmd);

class CUrlBufferModule : public CModule {
private: 
	TSettings settings;
	SUrls lastUrls; 
	
	inline void SaveSettings();
	inline void LoadSettings();
	static const string supportedExts[MAX_EXTS] ;
	static const char unSupportedChars[MAX_CHARS];
	inline bool isValidExtension(CString ext){
		for(int i=0; i< MAX_EXTS; i++){
	        	if( ext.MakeLower() == supportedExts[i]){
				return true;
			}
		}	
		return false;
	}
	inline bool isValidDir(string dir){
		 for(int i=0; i< MAX_CHARS; i++)
                        if (dir.find(unSupportedChars[i]) !=string::npos)
                                return false;
		return true;
	}
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

const string CUrlBufferModule::supportedExts[MAX_EXTS] = {"jpg", "png", "gif", "jpeg", "bmp", "tiff"} ;
const char CUrlBufferModule::unSupportedChars[MAX_CHARS] = {'|', ';', '!', '@', '#', '(', ')', '<', '>', '"',  '\'', '`', '~', '=', '&', '^'};

bool CUrlBufferModule::OnLoad(const CString& sArgs, CString& sErrorMsg) {
	LoadSettings(); 
	return true;
}

CUrlBufferModule::~CUrlBufferModule() {}

CUrlBufferModule::EModRet CUrlBufferModule::OnUserMsg(CString& sTarget, CString& sMessage) {
	CheckLineForLink(sMessage, "");
        CheckLineForTrigger(sMessage, sTarget);
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnPrivMsg(CNick& Nick, CString& sMessage) { 
	CheckLineForLink(sMessage, Nick.GetNick());
	CheckLineForTrigger(sMessage, Nick.GetNick());
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) {
	CheckLineForLink(sMessage, Nick.GetNick());
        CheckLineForTrigger(sMessage, Nick.GetNick());
	return CONTINUE;
}

void CUrlBufferModule::OnModCommand(const CString& sCommand) {
	CString command = sCommand.Token(0).AsLower().Trim_n();
	
	if (command == "help") {
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
	}else if (command == "enable") {
		settings["enable"]="true";
		PutModule("Enabled buffering");
        }else if (command == "disable") {
		settings["enable"]="false";
		PutModule("Disabled buffering");
        }else if (command == "enablelocal"){
		settings["enablelocal"]="true";
		PutModule("Enabled local caching");
	}else if (command == "disablelocal"){
		settings["enablelocal"]="false";
		PutModule("Disabled local caching");
	}else if (command == "directory") {
		CString dir=sCommand.Token(1);
		if (!isValidDir(dir)){
			PutModule("Error in directory name. Avoid using: | ; ! @ # ( ) < > \" ' ` ~ = & ^ <space> <tab>");
			return;
		}
		if(dir.Right(1)!="/") dir = dir + "/";
                settings["directory"]= dir;
		PutModule("Directory for local caching set to " + settings["directory"]);
        }else if (command == "clearbuffer"){
		lastUrls.clear();
	}else if (command == "buffersize"){
		unsigned int bufSize;
		std::istringstream size(sCommand.Token(1));
		if(!(size >> bufSize)){
			PutModule("Error in buffer size. Use only integers >= 0.");
			return;
		}
		settings["buffersize"]= CString(bufSize);
		PutModule("Buffer size set to " + settings["buffersize"]); 
	}else if (command == "showsettings"){
		for(TSettings::const_iterator itc = settings.begin(); itc != settings.end(); itc++){
                        PutModule(itc->first.AsUpper() + " : " + itc->second);
        	}
	}else{
		PutModule("Unknown command! Try HELP.");
		return;
	}
	SaveSettings();
}

void CUrlBufferModule::SaveSettings() {
	ClearNV();
	for(TSettings::const_iterator itc = settings.begin(); itc != settings.end(); itc++){		
		if(itc==settings.end())
			SetNV(itc->first, itc->second, true); //write the changes to disk after the last setting
		else
			SetNV(itc->first, itc->second, false);
	}
}

void CUrlBufferModule::LoadSettings() {
	//set defaults
	settings["enable"]="true";
	settings["enablelocal"]="false";
	settings["buffersize"]="5";
	//overwrite defaults if new settings exist
	for(MCString::iterator it = BeginNV(); it != EndNV(); it++) {
		settings[it->first] = it->second;
	}
}

void CUrlBufferModule::CheckLineForLink(const CString& sMessage, const CString& sOrigin){
	if(sOrigin != m_pUser->GetIRCNick().GetNick() && settings["enable"]=="true" ){
	
		VCString words;
		CString output;
		sMessage.Split(" ", words, false,"", "", true, true);
		for (size_t a = 0; a < words.size(); a++) {
			const CString& word = words[a];
			if(word.Left(4) == "http"){            //if you find an image download it, save it in the www directory and keep the new link in buffer
				VCString tokens;
				word.Split("/", tokens, false, "", "", true, true);
				string name = tokens[tokens.size()-1];
				word.Split(".", tokens, false, "", "", true, true);

				if(isValidExtension( tokens[tokens.size()-1] )){
			    		pthread_t thread;

			    		std::stringstream ss;
			    		ss << "wget -O " << settings["directory"].c_str() << name <<" -q " << word.c_str() ;
					if(settings["enablelocal"]=="true"){ 
						const char* cmd = ss.str().c_str();
						pthread_create( &thread, NULL, download, (void *)cmd);
					}
			    		ss.str("");
			    		ss << "curl -d \"image=" << word.c_str() << "\" -d \"key=5ce86e7f95d8e58b18931bf290f387be\" http://api.imgur.com/2/upload.xml | sed -n 's/.*<original>\\(.*\\)<\\/original>.*/\\1/p'";
			    		output = getStdoutFromCommand(ss.str());
			    		if(settings["enablelocal"]=="true")
                                                pthread_join(thread,NULL);
			    
			    		lastUrls.push_back(output);
				}
		}
	}
	}
} 

void *download(void *ptr){
	char *command;
	command = (char *) ptr;
	getStdoutFromCommand(command);
	return NULL;
}

CString getStdoutFromCommand(string cmd) {
	string data="";
	char buffer[128];
	cmd.append(" 2>&1");	
	
	FILE* stream = popen(cmd.c_str(), "r");
	if (stream == NULL || !stream || ferror(stream)){ 
		return CString("Error!");
	}
	while (!feof(stream)){
		if (fgets(buffer, 128, stream) != NULL){
			data.append(buffer);
		}
	}
	
	pclose(stream);
	return CString(data);
}

void CUrlBufferModule::CheckLineForTrigger(const CString& sMessage, const CString& sTarget){
	VCString words;
	sMessage.Split(" ", words, false, "", "", true, true);
	for (size_t a = 0; a < words.size(); a++) {
                CString& word = words[a];
		
		if(word.AsLower() == "!showlinks"){
			if(lastUrls.size()==0){
				PutIRC("PRIVMSG " + sTarget + " :No links were found...");
			}else {
				unsigned int maxLinks;
				std::istringstream ss(settings["buffersize"]);
                                ss >> maxLinks;
				
				if (a+1 < words.size()){	
					ss.clear();
					ss.str(words[a+1]);
					if((ss >> maxLinks).fail()){}
				} 
				for(unsigned int i=0; i< lastUrls.size() && i < maxLinks; i++){
					PutIRC("PRIVMSG " + sTarget + " :" + lastUrls[i]);
				}
			}
		}
	}
}

MODULEDEFS(CUrlBufferModule, "Module that caches images from links posted on irc channels.")

