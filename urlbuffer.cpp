/* Copyright (C) 2011 uberspot
 *
 * Compiling: znc-buildmod urlbuffer.cpp
 * Api key: 5ce86e7f95d8e58b18931bf290f387be
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation (http://www.gnu.org/licenses/gpl.txt).
*/ 

#define CURL_STATICLIB 
#include <curl/curl.h>
/* #include <curl/types.h> // removed in arch linux */
#include <curl/easy.h>
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
	size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
	void CheckLineForLink(const CString& sMessage, const CString& sChannel, const CString& sNick);
	void CheckLineForTrigger(const CString& sMessage, const CString& sChannel, const CString& sNick); 
	
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
		if(it->first == "settingbla") {
			CString sChanName = it->first; 
			CString right = it->second; 
			settings[sChanName].push_back(right);
		}
	}
}

void CUrlBufferModule::CheckLineForLink(const CString& sMessage, const CString& sChannel, const CString& sNick){
	VCString words;
	CString space = " ", empty="";
	sMessage.Split(space, words, false, empty, empty, true, true);
	for (size_t a = 0; a < words.size(); a++) {
			const CString& word = words[a];
			if(word.Left(4) == "http"){
				//if you find one download it, save it in the www directory and keep new link in buffer ;
				CURL *curl;
				const char *url = word.c_str();
				FILE *fp;
				CURLcode res; 
				char outfilename[FILENAME_MAX] = "/var/www/urlbuffer/temp";
				curl = curl_easy_init();
				if (curl) {
					fp = fopen(outfilename,"wb");
					curl_easy_setopt(curl, CURLOPT_URL, url ); //link 
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CUrlBufferModule::write_data);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
					res = curl_easy_perform(curl);
					curl_easy_cleanup(curl);
					fclose(fp);
				}
				lastUrls.push_back("http://uberspot.ath.cx/urlbuffer/" + word);
				//reupload on imgur curl -F "image=@file.png" -F "key=YOUR_API_KEY" http://api.imgur.com/2/upload.xml  OR curl -d "image=http://example.com/example.jpg" -d "key=YOUR_API_KEY" http://api.imgur.com/2/upload.json
				
			}
	}
}

size_t CUrlBufferModule::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
		size_t written;
		written = fwrite(ptr, size, nmemb, stream);
		return written;
}
	
void CUrlBufferModule::CheckLineForTrigger(const CString& sMessage, const CString& sChannel, const CString& sNick){
	//search for trigger in message
	//if one is found
	//return the last x buffered links
}

MODULEDEFS(CUrlBufferModule, "Module that caches images from links posted on irc channels.")

