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

class CUrlBufferModule : public CModule {
private: 
	TSettings settings;
	
	void SaveSettings();
	void LoadSettings();
	
public:
	MODCONSTRUCTOR(CUrlBufferModule) {}

	bool OnLoad(const CString& sArgs, CString& sErrorMsg);
	~CUrlBufferModule();
	EModRet OnBroadcast(CString& sMessage);
	EModRet OnRaw(CString& sLine);
	EModRet OnUserRaw(CString& sLine);
	EModRet OnUserCTCPReply(CString& sTarget, CString& sMessage);
	EModRet OnCTCPReply(CNick& Nick, CString& sMessage);
	EModRet OnUserCTCP(CString& sTarget, CString& sMessage);
	EModRet OnPrivCTCP(CNick& Nick, CString& sMessage);
	EModRet OnChanCTCP(CNick& Nick, CChan& Channel, CString& sMessage) ;
	EModRet OnUserMsg(CString& sTarget, CString& sMessage);
	EModRet OnPrivMsg(CNick& Nick, CString& sMessage);
	EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage);
	void OnModCommand(const CString& sCommand);
	void CheckLineForLink(const CString& sMessage, const CString& sChannel, const CString& sNick);
	void CheckLineForTrigger(const CString& sMessage, const CString& sChannel, const CString& sNick);
};

bool CUrlBufferModule::OnLoad(const CString& sArgs, CString& sErrorMsg) {
	LoadSettings();
	return true;
}

CUrlBufferModule::~CUrlBufferModule() {}

CUrlBufferModule::EModRet CUrlBufferModule::OnBroadcast(CString& sMessage) {
	PutModule("------ [" + sMessage + "]");
	sMessage = "======== [" + sMessage + "] ========";
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnRaw(CString& sLine) {
	//sLinePutModule("OnRaw() [" + sLine + "]");
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnUserRaw(CString& sLine) {
	//sLinePutModule("UserRaw() [" + sLine + "]");
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnUserCTCPReply(CString& sTarget, CString& sMessage) {
	PutModule("[" + sTarget + "] userctcpreply [" + sMessage + "]");
	sMessage = "\037" + sMessage + "\037";
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnCTCPReply(CNick& Nick, CString& sMessage) {
	PutModule("[" + Nick.GetNick() + "] ctcpreply [" + sMessage + "]");
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnUserCTCP(CString& sTarget, CString& sMessage) {
	PutModule("[" + sTarget + "] userctcp [" + sMessage + "]");
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnPrivCTCP(CNick& Nick, CString& sMessage) {
	PutModule("[" + Nick.GetNick() + "] privctcp [" + sMessage + "]");
	sMessage = "\002" + sMessage + "\002";
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnChanCTCP(CNick& Nick, CChan& Channel, CString& sMessage) {
	PutModule("[" + Nick.GetNick() + "] chanctcp [" + sMessage + "] to [" + Channel.GetName() + "]");
	sMessage = "\00311,5 " + sMessage + " \003";
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnUserMsg(CString& sTarget, CString& sMessage) {
	PutModule("[" + sTarget + "] usermsg [" + sMessage + "]");
	sMessage = "\0034" + sMessage + "\003";
	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnPrivMsg(CNick& Nick, CString& sMessage) {
	PutModule("[" + Nick.GetNick() + "] privmsg [" + sMessage + "]");
	sMessage = "\002" + sMessage + "\002";

	return CONTINUE;
}

CUrlBufferModule::EModRet CUrlBufferModule::OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) {
	if (sMessage == "!ping") {
		PutIRC("PRIVMSG " + Channel.GetName() + " :PONG?");
	}
	sMessage = "x " + sMessage + " x";
	PutModule(sMessage);

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
		if(it->first == "setting1") {
			const CString sChanName = it->first.substr(5); 
			CString right = it->second; 
			settings[sChanName].push_back(right);
		}
	}
}

void CheckLineForLink(const CString& sMessage, const CString& sChannel, const CString& sNick){
	//search for link (image link for start)
	//if you find one download it, save it in the www directory and keep new link in buffer
}

void CheckLineForTrigger(const CString& sMessage, const CString& sChannel, const CString& sNick){
	//search for trigger in message
	//if one is found
	//return the last x buffered links
}
	
MODULEDEFS(CUrlBufferModule, "Module that caches images from links posted on irc channels.")

