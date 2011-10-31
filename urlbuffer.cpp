/* Copyright (C) 2011 uberspot
 *
 * Compiling: znc-buildmod urlbuffer.cpp 
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

typedef map<const CString, VCString> TSettings;
typedef vector<CString> SUrls;

class CUrlBufferModule : public CModule {
private: 
	TSettings settings;
	SUrls lastUrls; 
	
	void SaveSettings();
	void LoadSettings(); 
	void CheckLineForLink(const CString& sMessage);
	void CheckLineForTrigger(const CString& sMessage, const CString& sChannel, const CString& sNick); 
	CString getStdoutFromCommand(string cmd);
public:
	MODCONSTRUCTOR(CUrlBufferModule) {}

	bool OnLoad(const CString& sArgs, CString& sErrorMsg);
	~CUrlBufferModule(); 
	EModRet OnUserMsg(CString& sTarget, CString& sMessage);
	EModRet OnPrivMsg(CNick& Nick, CString& sMessage);
	EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage);
	void OnModCommand(const CString& sCommand);
	
};

bool CUrlBufferModule::OnLoad(const CString& sArgs, CString& sErrorMsg) {
	LoadSettings(); 
	return true;
}

CUrlBufferModule::~CUrlBufferModule() {}

CUrlBufferModule::EModRet CUrlBufferModule::OnUserMsg(CString& sTarget, CString& sMessage) {
	//PutModule("[" + sTarget + "] usermsg [" + sMessage + "]");
	//sMessage = "\0034" + sMessage + "\003";
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnPrivMsg(CNick& Nick, CString& sMessage) { 
	CheckLineForLink("http://www.fclbuilders.com/images/port_images/mids/cpp-main.jpg blabla .");//works
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) {
	if (sMessage == "!ping") {
		PutIRC("PRIVMSG " + Channel.GetName() + " :PONG?");
	}

	return CONTINUE;
}

void CUrlBufferModule::OnModCommand(const CString& sCommand) {
	if (strcasecmp(sCommand.c_str(), "TIMERS") == 0) {
		return;
	}
}

void CUrlBufferModule::SaveSettings() {
	ClearNV();

	for(TSettings::const_iterator itc = settings.begin(); itc != settings.end(); itc++){
		CString sName =  itc->first;
		CString sData;

		for(VCString::const_iterator itt = itc->second.begin(); itt != itc->second.end(); itt++){
			sData += *itt + "";
		}
		if(itc==settings.end())
			SetNV(sName, sData, true); //write the changes to disk after the last set
		else
			SetNV(sName, sData, false);
	}

}

void CUrlBufferModule::LoadSettings() {
	for(MCString::iterator it = BeginNV(); it != EndNV(); it++) {
		if(it->first == "settingbla") {
			CString sChanName = it->first; 
			CString right = it->second; 
			settings[sChanName].push_back(right);
		}
	}
}

void CUrlBufferModule::CheckLineForLink(const CString& sMessage){
	VCString words;
	CString space(" "), empty(""), output;
	sMessage.Split(space, words, false, empty, empty, true, true);
	for (size_t a = 0; a < words.size(); a++) {

		const CString& word = words[a];
		if(word.Left(4) == "http"){
			//if you find one download it, save it in the www directory and keep new link in buffer ; 
			
			const char *url = word.c_str(); 
			std::stringstream ss;
			ss << "wget -O /tmp/test -q " << url;
			output = getStdoutFromCommand(ss.str());
			ss.str("");
			ss << "curl -d \"image=" << url << "\" -d \"key=5ce86e7f95d8e58b18931bf290f387be\" http://api.imgur.com/2/upload.xml | sed -n 's/.*<original>\\(.*\\)<\\/original>.*/\\1/p'";
			output = getStdoutFromCommand(ss.str());
			PutModule("upload output: " + output);
			lastUrls.push_back(output);
		}
	}
} 
	
CString CUrlBufferModule::getStdoutFromCommand(string cmd) {
	string data="";
	char buffer[128];
	cmd.append(" 2>&1");	
	
	FILE* stream = popen(cmd.c_str(), "r");
	if (stream == NULL || !stream || ferror(stream)){
                PutModule("Error");
		return CString("");
	}
	while (!feof(stream)){
		if (fgets(buffer, 128, stream) != NULL){
			data.append(buffer);
		}
	}
	
	pclose(stream);
	return CString(data);
}

void CUrlBufferModule::CheckLineForTrigger(const CString& sMessage, const CString& sChannel, const CString& sNick){
	//search for trigger in message
	//if one is found
	//return the last x buffered links
}

MODULEDEFS(CUrlBufferModule, "Module that caches images from links posted on irc channels.")

