/*
  Copyright (C) 2005 Mikael Magnusson, Erik Eliasson
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 * Author(s): Mikael Magnusson <mikma@users.sourceforge.net>
 *            Erik Eliasson <eliasson@it.kth.se>
*/


/* Name
 * 	SipRequest.cxx
 * Author
 *      Mikael Magnusson, mikma@users.sourceforge.net
 *      Erik Eliasson, eliasson@it.kth.se 
 * Purpose
 *      Any SIP request request to libmsip
*/

#include<config.h>

#include<libmsip/SipStack.h>
#include<libmsip/SipRequest.h>
#include<libmsip/SipException.h>
#include<libmsip/SipCommandDispatcher.h>
#include<libmsip/SipMessageContentIM.h>
#include<libmsip/SipHeader.h>
#include<libmsip/SipHeaderRoute.h>
#include<libmsip/SipHeaderEvent.h>
#include<libmsip/SipHeaderFrom.h>
#include<libmsip/SipHeaderTo.h>
#include<libmsip/SipHeaderCallID.h>
#include<libmsip/SipHeaderCSeq.h>
#include<libmsip/SipHeaderMaxForwards.h>
#include<libmsip/SipHeaderUserAgent.h>
#include<libmsip/SipHeaderProxyAuthorization.h>
#include<libmsip/SipHeaderAccept.h>
#include<libmsip/SipHeaderContact.h>
#include<libmsip/SipHeaderReferTo.h>
#include<libmsip/SipHeaderSupported.h>

using namespace std;

MRef<SipRequest*> SipRequest::createSipMessageAck(const string &branch,
		MRef<SipRequest*> origReq,
		MRef<SipResponse*> resp,
		bool provisional)
{
	string method;
	if (provisional){
		method = "PRACK";
	}else{
		method = "ACK";
	}
	MRef<SipRequest*> req = new SipRequest(branch, method);
	req->setUri(origReq->getUri());

	req->addHeader(new SipHeader(new SipHeaderValueMaxForwards(70)));
	
	int noHeaders = origReq->getNoHeaders();
	for (int32_t i=0; i< noHeaders; i++){			//FIX: deep copy?
		MRef<SipHeader *> header = origReq->getHeaderNo(i);
		int headerType = header->getType();
		switch (headerType){
			case SIP_HEADER_TYPE_CSEQ:
				((SipHeaderValueCSeq*) *(header->getHeaderValue(0)))->setMethod(method);
			case SIP_HEADER_TYPE_FROM:
			case SIP_HEADER_TYPE_CALLID:
			case SIP_HEADER_TYPE_ROUTE:
			case SIP_HEADER_TYPE_AUTHORIZATION:
			case SIP_HEADER_TYPE_PROXYAUTHORIZATION:
				req->addHeader(header);
				break;
		}
	}

	req->addHeader( new SipHeader( *resp->getHeaderValueTo() ) );
	
	return req;
}


MRef<SipRequest*> SipRequest::createSipMessageCancel(const string &branch,
							MRef<SipRequest*> inv
// 							string to_uri
							)
{
	MRef<SipRequest*> req = new SipRequest(branch, "CANCEL");
	req->setUri(inv->getUri());

	req->addHeader(new SipHeader( new SipHeaderValueMaxForwards(70)));
	
	MRef<SipHeader *> header;
	int noHeaders = inv->getNoHeaders();
	for (int32_t i=0; i< noHeaders; i++){
		header = inv->getHeaderNo(i);
		int type = header->getType();
		bool add=false;
		switch (type){
			case SIP_HEADER_TYPE_FROM:
				add=true;
				break;
			case SIP_HEADER_TYPE_TO:
				add=true;
				break;
			case SIP_HEADER_TYPE_CSEQ:
				((SipHeaderValueCSeq*)*(header->getHeaderValue(0)))->setCSeq(  ((SipHeaderValueCSeq *)*(header->getHeaderValue(0)))->getCSeq() );
				((SipHeaderValueCSeq*)*(header->getHeaderValue(0)))->setMethod("CANCEL");
				add=true;
				break;
			case SIP_HEADER_TYPE_CALLID:
				add=true;
				break;
			case SIP_HEADER_TYPE_ROUTE:
			case SIP_HEADER_TYPE_AUTHORIZATION:
			case SIP_HEADER_TYPE_PROXYAUTHORIZATION:
				add=true;
				break;
		}
		if (add){
			req->addHeader(header);
		}
	}
	
	
	return req;
}


MRef<SipRequest*> SipRequest::createSipMessageIMMessage(const string& branch,
							const string& callId,
							const SipUri& toUri,
							const SipUri& fromUri,
							int32_t seqNo,
							const string& msg)
{
	MRef<SipRequest*> req = new SipRequest(branch, "MESSAGE");
	req->setUri(toUri);
	req->addDefaultHeaders(fromUri,toUri,"MESSAGE",seqNo,callId);
	req->addHeader(new SipHeader(new SipHeaderValueUserAgent(HEADER_USER_AGENT_DEFAULT)));
	req->setContent(new SipMessageContentIM(msg));
	return req;
}

static void addHeaders( MRef<SipRequest*> req,
		const SipUri &contact,
		MRef<SipStack*> stack
		)
{

	req->addHeader(new SipHeader(new SipHeaderValueContact(contact)));
	req->addHeader(new SipHeader(new SipHeaderValueUserAgent(HEADER_USER_AGENT_DEFAULT)));
	if (stack){
		req->addHeader(new SipHeader(new SipHeaderValueSupported(stack->getAllSupportedExtensionsStr())));
	}
}

MRef<SipRequest*> SipRequest::createSipMessageInvite(const string &branch,
							const string &call_id,
							const SipUri &toUri,
							const SipUri &fromUri,
							const SipUri &contact,
							int32_t seq_no,
							MRef<SipStack*> stack
                )
{
	MRef<SipRequest*> req = new SipRequest(branch,"INVITE");
	req->setUri(toUri);

	req->addDefaultHeaders( fromUri, toUri, "INVITE", seq_no, call_id );
	addHeaders(req, contact, stack);

	return req;
}

MRef<SipRequest*> SipRequest::createSipMessageNotify(const string& branch,
							const string& callId,
							const SipUri& toUri,
							const SipUri& fromUri,
							int32_t seqNo
							)
{
	MRef<SipRequest*> req = new SipRequest(branch, "NOTIFY");
	req->setUri(toUri);
	req->addDefaultHeaders(fromUri, toUri,"NOTIFY",seqNo,callId);
	req->addHeader(new SipHeader(new SipHeaderValueUserAgent(HEADER_USER_AGENT_DEFAULT)));
	req->addHeader(new SipHeader(new SipHeaderValueEvent("presence")));
	return req;
}

MRef<SipRequest*> SipRequest::createSipMessageRegister(const string &branch,
							const string &call_id,
							const SipUri &fromUri,
						       MRef<SipHeaderValueContact *> contactHdr,
						       int32_t seq_no)
{
	const std::string &domain = fromUri.getIp();

	MRef<SipRequest*> req = new SipRequest(branch, "REGISTER","sip:"+domain);

	req->addDefaultHeaders(fromUri,fromUri,"REGISTER",seq_no,call_id);

	req->addHeader(new SipHeader(*contactHdr));
	req->addHeader(new SipHeader(new SipHeaderValueUserAgent(HEADER_USER_AGENT_DEFAULT)));
	req->setContent(NULL);

	return req;
}


MRef<SipRequest*> SipRequest::createSipMessageSubscribe(const string &branch,
							const string &call_id,
							const SipUri &toUri,
							const SipUri &fromUri,
							const SipUri &contact,
							int32_t seq_no)
{
	MRef<SipRequest*> req = new SipRequest(branch, "SUBSCRIBE", toUri.getString() );

	req->setUri(toUri);

	req->addDefaultHeaders(fromUri, toUri,"SUBSCRIBE",seq_no, call_id);
	req->addHeader(new SipHeader(new SipHeaderValueContact(contact)));
	req->addHeader(new SipHeader(new SipHeaderValueEvent("presence")));
	req->addHeader(new SipHeader(new SipHeaderValueAccept("application/xpidf+xml")));
						
	return req;
}

void SipRequest::addDefaultHeaders(const SipUri& fromUri,
		const SipUri& toUri,
		const string& method,
		int seqNo,
		const string& callId)
{
	addHeader(new SipHeader(new SipHeaderValueFrom(fromUri)));
	addHeader(new SipHeader(new SipHeaderValueTo(toUri)));
	addHeader(new SipHeader(new SipHeaderValueCallID(callId)));
	addHeader(new SipHeader(new SipHeaderValueCSeq(method, seqNo)));
	addHeader(new SipHeader(new SipHeaderValueMaxForwards(70)));
}


SipRequest::SipRequest(string branch, const string &method,
		       const string &uri) :
		SipMessage(branch), method(method),
		uri(uri)
{
// 	if( this->uri == "" )
// 		this->uri = "sip:";
}

SipRequest::SipRequest(string &build_from): SipMessage(-1, build_from){
	init(build_from);
}

void SipRequest::init(string &build_from){
	int start = 0;
	int pos;
	int pos2;
	int end = 0;
	//int length = build_from.length();
	string requestLine;

	// Skip white space
	start = build_from.find_first_not_of( ' ', start );
	if( start == (int)string::npos ){
		throw SipExceptionInvalidMessage("SipRequest malformed - first line did not contain any non whitespace character");
	}

	end = build_from.find_first_of( "\r\n", start );
	if( end == (int)string::npos ){
		throw SipExceptionInvalidMessage("SipRequest malformed - only one line");
	}

	requestLine = build_from.substr( start, end - start );
	#ifdef DEBUG_OUTPUT
	cerr << "SipRequest::init - requestLine = " << requestLine << endl;
	#endif
	start = 0;
	end = requestLine.length();

	// Parse method
	pos = requestLine.find( ' ', start );
	if( pos == (int)string::npos ){
		throw SipExceptionInvalidMessage("SipRequest malformed - could not find method");
	}

	method = build_from.substr( start, pos - start );
	#ifdef DEBUG_OUTPUT
	cerr << "SipRequest::init - method = " << method << endl;
	#endif

	// Parse version
	pos2 = requestLine.rfind( ' ', end - 1 );
	if( pos2 == (int)string::npos ){
		throw SipExceptionInvalidMessage("SipRequest malformed - request line did not contain space between method and version");
	}

	string version = requestLine.substr( pos2 + 1, end - pos2 );

	if( version != "SIP/2.0" ){
		throw SipExceptionInvalidMessage("SipRequest malformed - unknown version");
	}	  

	pos2 = requestLine.find_last_not_of( ' ', pos2 );

	uri = requestLine.substr( pos + 1, pos2 - pos );
	if( !uri.isValid() ){
		throw SipExceptionInvalidMessage("SipRequest malformed - uri");
	}

	#ifdef DEBUG_OUTPUT
	cerr << "SipRequest::init - uri = " << uri.getString() << endl;
	#endif
}

SipRequest::~SipRequest(){
}

const string& SipRequest::getType(){
	return method;
}

string SipRequest::getString() const{
	return getMethod() + " " + getUri().getRequestUriString() + " SIP/2.0\r\n"
		+ getHeadersAndContent();
}


void SipRequest::setMethod(const string &method){
	this->method = method;
}

string SipRequest::getMethod() const{
	return method;
}

static string buildUri(const string &name)
{
	string ret ="";
	
	//FIXME sanitize the request uri ... if we used a SipURI object, this would not be needed
	string username; //hide the class::username ... carefull
	size_t pos;
	username = name;
	
	pos = username.find('<');
	if( pos != string::npos ) {
		username.erase( 0, pos + 1 ); //erase the part in front of the '<'
		pos = username.find('>');
		username.erase( pos );
	}

	if (username.length()>4 && username.substr(0,4)=="sip:")
		ret = username;
	else if (username.length()>5 && username.substr(0,5)=="sips:")
		ret = username;
	else
		ret = "sip:"+username;

#if 0
	if (username.find("@")==string::npos)
		ret = ret+"@"+ip;
#endif

	return ret;
}

// void SipRequest::setUri(const string &uri){
// 	setUri(SipUri(buildUri(uri)));
// }

void SipRequest::setUri(const SipUri &uri){
	this->uri = uri;
}

const SipUri &SipRequest::getUri() const{
	return uri;
}

void SipRequest::addRoute(const string &route)
{
	MRef<SipHeaderValue*> routeValue = (SipHeaderValue*)new SipHeaderValueRoute( route );
	MRef<SipHeader*> routeHdr = new SipHeader( routeValue );

	addBefore(routeHdr);
}

void SipRequest::addRoute(const string &addr, int32_t port,
			  const string &transport)
{
	string uri = "<sip:" + addr;

	if( port ){
		char buf[20];
		sprintf(buf, "%d", port);
		uri = uri + ":" + buf;
	}

	if( transport != "" ){
		uri = uri + ";transport=" + transport;
	}

	uri = uri + ";lr>";
	
	addRoute( uri );
}

MRef<SipHeaderValueRoute*> SipRequest::getFirstRoute()
{
	MRef<SipHeaderValueRoute*> route;
	MRef<SipHeader*> header = getHeaderOfType(SIP_HEADER_TYPE_ROUTE, 0);

	if( header ){
		route = MRef<SipHeaderValueRoute*>((SipHeaderValueRoute*)*(header->getHeaderValue(0)));
	}
	return route;
}

void SipRequest::removeFirstRoute(){
	MRef<SipHeader*> hdr = getHeaderOfType(SIP_HEADER_TYPE_ROUTE, 0);

	if( hdr->getNoValues() > 1 ){
		hdr->removeHeaderValue( 0 );
	}
	else{
		removeHeader( hdr );
	}
}
