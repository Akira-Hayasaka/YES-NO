/*
 *  HTTPClient.cpp
 *  AppManager
 *
 *  Created by Makira on 11/01/31.
 *  Copyright 2011 yesMAYBEno. All rights reserved.
 *
 */

#include "HTTPClient.h"

void HTTPClient::setup(){

	action_url = "http://localhost:8888/getSMS.of";
	ofAddListener(httpUtils.newResponseEvent, this, &HTTPClient::getResponse);
	httpUtils.start();
	
	counter = new TimedCounter(5);
	counter->startCount();	

};

void HTTPClient::update(){

	if(counter->isCounting()){
		counter->update();
		if (counter->isCountComplete()) {
			sendRequest();
			counter->startCount();
		}
	}

};

void HTTPClient::sendRequest(){

	ofxHttpForm form;
	form.action = action_url;
	form.method = OFX_HTTP_POST;
	
	time_t t;
	time(&t);
	char gmttime[256];
	strftime(gmttime, 255, "%Y¥/%m¥/%d¥/%H¥/%I¥/%S", gmtime(&t));
	string timestr = gmttime;
	timestr = str_replace(timestr, "¥", "");			
	
	form.addFormField("time", timestr);
	httpUtils.addForm(form);	

};

void HTTPClient::getResponse(ofxHttpResponse & response){

	responseStr.clear();
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
		smsMsgs.push_back(sms);
		
		xml.popTag();
	}
	xml.popTag();
	xml.popTag();	
	
//	for (int i = 0; i < smsMsgs.size(); i++) {
//		cout << "Answer: "+smsMsgs[i].answer << endl;
//		cout << "Time: "+smsMsgs[i].time << endl;
//	}

};


string HTTPClient::str_replace(const string &source,
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