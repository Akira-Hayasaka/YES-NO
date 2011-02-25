/*
 *  HTTPSMSClient.cpp
 *  AppManager
 *
 *  Created by Makira on 11/01/31.
 *  Copyright 2011 yesMAYBEno. All rights reserved.
 *
 */

#include "HTTPSMSClient.h"

void HTTPSMSClient::setup(){

	action_url = "http://localhost:8888/getSMS.of";
	ofAddListener(httpUtils.newResponseEvent, this, &HTTPSMSClient::getResponse);
	httpUtils.start();
	
	counter = new TimedCounter(10);
	counter->startCount();	

};

void HTTPSMSClient::update(){

	if(counter->isCounting()){
		counter->update();
		if (counter->isCountComplete()) {
			sendRequest();
			counter->startCount();
		}
	}

};

void HTTPSMSClient::sendRequest(){

	ofxHttpForm form;
	form.action = action_url;
	form.method = OFX_HTTP_POST;
	
	time_t t;
	time(&t);
	char gmttime[256];
	strftime(gmttime, 255, "%Y¥/%m¥/%d¥/%H¥/%I¥/%S", gmtime(&t));
	string timestr = gmttime;
	timestr = str_replace(timestr, "¥", "");			
	reqestTime = timestr;
	
	form.addFormField("time", timestr);
	httpUtils.addForm(form);	

};

void HTTPSMSClient::getResponse(ofxHttpResponse & response){

	responseStr.clear();
	
	time_t t;
	time(&t);
	char gmttime[256];
	strftime(gmttime, 255, "%Y¥/%m¥/%d¥/%H¥/%I¥/%S", gmtime(&t));
	string timestr = gmttime;
	timestr = str_replace(timestr, "¥", "");		
	recieveTime = timestr;
	
	thisTimeYess.clear();
	thisTimeNos.clear();
	
	xml.clear();
	xml.loadFromBuffer(response.responseBody);
	
	// debug block
//	{
//		string test;
//		xml.copyXmlToString(test);
//		cout << test << endl;
//		
//		xml.pushTag("TangibleHUBYesNoResponce");
//		xml.pushTag("SMSs");
//		int tansmss = xml.getNumTags("SMS");
//		cout << "TangibleHUBYesNoResponce:SMSs:SMS = " + ofToString(tansmss) << endl;
//		xml.popTag();
//		xml.popTag();
//	}
	
	xml.pushTag("TangibleHUBYesNoResponce");
	xml.pushTag("SMSs");
	int numTag = xml.getNumTags("SMS");
	for (int i = 0; i < numTag; i++) {
		xml.pushTag("SMS", i);
		string ans = xml.getValue("Answer", "error");
		string time = xml.getValue("Time", "error");
		responseStr += " "+ans+" "+time+" ";
		
		smsMsg sms;
		sms.answer = ans;
		sms.time = time;
		
		if ("YES" == sms.answer) {
			totalYess.push_back(sms);
			thisTimeYess.push_back(sms);
		}else {
			totalNos.push_back(sms);
			thisTimeNos.push_back(sms);
		}
		
		xml.popTag();
	}
	xml.popTag();
	xml.popTag();	

	UpdateInfo upInfo = calcUpdateInfo();
	ofNotifyEvent(onSMSRecieved, upInfo);
	
};

UpdateInfo HTTPSMSClient::calcUpdateInfo() {
	
	UpdateInfo upInfo;
	
	float totalYes = totalYess.size();
	float totalNo = totalNos.size();
	float thisTimeYes = thisTimeYess.size();
	float thisTimeNo = thisTimeNos.size();
	
	float totalSMS = totalYes+totalNo;
	float totalSMSThisTime = thisTimeYes+thisTimeNo;
	float raTotalYes = (totalYes<=0)?0.0:totalYes/totalSMS;
	float raTotalNo = (totalNo<=0)?0.0:totalNo/totalSMS;
	float raThisTimeYes = (thisTimeYes<=0)?0.0:thisTimeYes/totalSMSThisTime;
	float raThisTimeNo = (thisTimeNo<=0)?0.0:thisTimeNo/totalSMSThisTime;
	
	smsMsg sms;
	if (test < 0) {
		sms.YesOrNo = HTTPSMSClient::YES;
		sms.answer = "YES";
	}else {
		sms.YesOrNo = HTTPSMSClient::NO;
		sms.answer = "NO";
	}
	upInfo.sms = sms;
	
	upInfo.ratioTotalYes = raTotalYes;
	upInfo.ratioTotalNo = raTotalNo;
	upInfo.ratioThisTimeYes = raThisTimeYes;
	upInfo.ratioThisTimeNo = raThisTimeNo;
	upInfo.numTotalYes = totalYes;
	upInfo.numTotalNo = totalNo;
	upInfo.numYes = thisTimeYes;
	upInfo.numNo = thisTimeNo;
	upInfo.numDiffYes = totalYes-totalNo;
	upInfo.numDiffNo = totalNo-totalYes;
	upInfo.requesttime = reqestTime;
	
//	cout << "thisTimeYes recieved = " + ofToString(upInfo.numYes) << endl;	
//	cout << "totalYes recieved = " + ofToString(upInfo.numTotalYes) << endl;
	
	return upInfo;
	
}

void HTTPSMSClient::emulateSMS() {
	
	ofxHttpResponse response;

	string res = "";
	string start = "<TangibleHUBYesNoResponce><SMSs>";
	
	time_t t;
	time(&t);
	char gmttime[256];
	strftime(gmttime, 255, "%Y¥/%m¥/%d¥/%H¥/%I¥/%S", gmtime(&t));
	string timestr = gmttime;
	timestr = str_replace(timestr, "¥", "");		
	
	string yes = "<SMS><Answer>YES</Answer><Time>" + timestr + "</Time></SMS>";
	string no = "<SMS><Answer>NO</Answer><Time>" + timestr + "</Time></SMS>";
	string contents = "";
	int thisreq = 1;//ofRandom(1, 20);
	for (int i = 0; i < thisreq; i++) {
		
		test = ofRandomf();
		if (test < 0) {
			contents += yes;
		}else {
			contents += no;
		}
	}
	
	string end = "</SMSs></TangibleHUBYesNoResponce>";
	res = start+contents+end;
	
	response.responseBody = res;
	getResponse(response);
	
}

string HTTPSMSClient::str_replace(const string &source,
							   const string &pattern,
							   const string &placement){

    string result;
    string::size_type pos_before = 0;
    string::size_type pos = 0;
    string::size_type len = pattern.size();
    while ((pos = source.find(pattern, pos)) != string::npos) {
        result.append(source, pos_before, pos - pos_before);
        result.append(placement);
        pos += len;
        pos_before = pos;
    }
    result.append(source, pos_before, source.size() - pos_before);
    return result;

};	