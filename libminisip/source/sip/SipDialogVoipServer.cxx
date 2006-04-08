/*
 Copyright (C) 2004-2006 the Minisip Team
 
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
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/* Copyright (C) 2004 
 *
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
 *	    Joachim Orrblad <joachim[at]orrblad.com>
*/

/* Name
 * 	SipDialogVoipServer.cxx
 * Author
 * 	Erik Eliasson, eliasson@it.kth.se
 * Purpose
 * 
*/
#include<config.h>

#include<libminisip/sip/SipDialogVoipServer.h>

#include<libmutil/massert.h>

//#include<libmsip/SipDialogContainer.h>
#include<libmsip/SipMessageTransport.h>
#include<libmsip/SipTransactionInviteClientUA.h>
#include<libmsip/SipTransactionInviteServerUA.h>
#include<libmsip/SipTransactionNonInviteClient.h>
#include<libmsip/SipTransactionNonInviteServer.h>
#include<libmsip/SipTransactionUtils.h>
#include<libmsip/SipCommandString.h>
#include<libmsip/SipHeaderWarning.h>
#include<libmsip/SipHeaderContact.h>
#include<libmsip/SipHeaderFrom.h>
#include<libmsip/SipHeaderRoute.h>
#include<libmsip/SipHeaderRequire.h>
#include<libmsip/SipHeaderTo.h>
#include<libmsip/SipMIMEContent.h>
#include<libmsip/SipMessageContent.h>
#include<libmutil/itoa.h>
#include<libmutil/base64.h>
#include<libmutil/Timestamp.h>
#include<libmutil/termmanip.h>
#include<libmutil/dbg.h>
#include<libmsip/SipSMCommand.h>
#include <time.h>
#include<libminisip/gui/LogEntry.h>

#include<libmutil/print_hex.h>
#include <iostream>
#include<time.h>

#ifdef _WIN32_WCE
#	include"../include/minisip_wce_extra_includes.h"
#endif

using namespace std;


/*

 The responsibility of the voip invite server is to accept
 or reject an incoming call request. It should either end
 up in the in_call state or be terminated. The super class,
 SipDialogVoip handles calls after they have been established.

 This class adds two states, start and ringing. The in_call,
 termwait and terminated states are inherited from the super-class
 (they have dotted borders in the diagram below).

 The "dotted" states are implemented in SipDialogVoip.cxx.

 
                 +---------------+
                 |               |
                 |     start     |
                 |               |
                 +---------------+
                         | 
                         | INVITE
		         | a3001:transIR; send 180
		         V
                 +---------------+
                 |               |
                 |    ringing    |----------+  reject
                 |               |          |  a3005:send40X
                 +---------------+          |
                         |                  |  CANCEL
                         | accept_invite &  |  a3004:transCancelResp
                         | authok           |
		         | a3002: send 200  |  BYE
		         V                  |  a3003:transByeResp
                 +. . . . . . . .+          |   
                 .               .          |  INVITE & a11 fail
                 .   in call     .          |  a3006: send 606
                 .               .          |   
                 +. . . . . . . .+          |
                                            |
                                            |
                                            |
                 +. . . . . . . .+          |
                 .               .          |
                 .   termwait    .<---------+
                 .               .
                 +. . . . . . . .+
                          
                                               
                                                 
                 +. . . . . . . .+
                 .               .
                 .  terminated   .
                 .               .
                 + . . . . . . . +
   
*/


bool SipDialogVoipServer::a3001_start_ringing_INVITE( const SipSMCommand &command)
{
	if (transitionMatch("INVITE", command, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer)){
		MRef<SipRequest*> inv = (SipRequest *)*command.getCommandPacket();
		string branch = inv->getDestinationBranch();
		
		setLastInvite(inv);
		dialogState.updateState( inv );
		
		//string peerUri = command.getCommandPacket()->getFrom().getString().substr(4);
		string peerUri = dialogState.remoteUri;
		
		getDialogConfig()->inherited->sipIdentity->setSipUri(inv->getHeaderValueTo()->getUri().getUserIpString());
		
		//MRef<SipMessageContent *> Offer = *command.getCommandPacket()->getContent();
		if(!sortMIME(*inv->getContent(), peerUri, 10)){
			merr << "No MIME match" << end;
			return false;
		}
#ifdef IPSEC_SUPPORT
		// Check if IPSEC was required
		if (ipsecSession->required() && !ipsecSession->offered){
			cerr << "I require IPSEC or nothing at all!" << endl;
			return false;
		}
#endif
		
//		cerr << "EEEEEEEE: finding transaction and adding to dialogs set"<<endl;
//		MRef<SipTransaction*> trans = dispatcher->getLayerTransaction()->findTransaction(branch);
//		massert(trans);
//		registerTransactionToDialog(trans);
		
/*		MRef<SipHeaderValue*> identity = command.getCommandPacket()->getHeaderValueNo(SIP_HEADER_TYPE_IDENTITY, 0);
		MRef<SipHeaderValue*> identityinfo = command.getCommandPacket()->getHeaderValueNo(SIP_HEADER_TYPE_IDENTITYINFO, 0);
		
		bool identityVerified=false;
		if (identity && identityinfo){
			cerr << "IDENTITY: found identity and identity-info header values"<< endl;
			assert(dynamic_cast<SipHeaderValueIdentity*>( *identity));
			assert(dynamic_cast<SipHeaderValueIdentityInfo*>( *identityinfo));
			MRef<SipHeaderValueIdentity*> ident = (SipHeaderValueIdentity*) *identity;
			MRef<SipHeaderValueIdentity*> identinfo = (SipHeaderValueIdentity*) *identityinfo;

			cerr << "IDENTITY: algorithm is: <"<< identinfo->getParameter("alg") << ">"<< endl;
			
			//downloadCertificate( identinfo->getCertUri() );
			
			identityVerified = verifyIdentityHeader(ident);


			//TODO: check that the identity is rsa-sha1
			
			if (!identityVerified){
#ifdef DEBUG_OUTPUT
				cerr << "IDENTITY: the verification FAILED!"<< endl;
#endif
			}
			
			
		}else{
			cerr << "IDENTITY: did not find identity header value"<< endl;
		}
*/		
		CommandString cmdstr(dialogState.callId, 
				SipCommandString::incoming_available, 
				dialogState.remoteUri, 
				(getMediaSession()->isSecure()?"secure":"unprotected")
				);
		dispatcher->getCallback()->handleCommand("gui", cmdstr );
		
		sendRinging(branch);
		
		if( getDialogConfig()->inherited->autoAnswer ){
			CommandString accept( dialogState.callId, SipCommandString::accept_invite );
			SipSMCommand sipcmd(accept, SipSMCommand::dialog_layer, SipSMCommand::dialog_layer);
			dispatcher->enqueueCommand(sipcmd,HIGH_PRIO_QUEUE/*, PRIO_LAST_IN_QUEUE*/);
		}
		return true;
	}else{
		return false;
	}
}

bool SipDialogVoipServer::a3002_ringing_incall_accept( const SipSMCommand &command)
{
	if (transitionMatch(command, 
				SipCommandString::accept_invite,
				SipSMCommand::dialog_layer,
				SipSMCommand::dialog_layer)){
#ifdef ENABLE_TS
		ts.save(USER_ACCEPT);
#endif

		CommandString cmdstr(dialogState.callId, 
				SipCommandString::invite_ok,"",
				(getMediaSession()->isSecure()?"secure":"unprotected")
				);
		dispatcher->getCallback()->handleCommand("gui", cmdstr );

		massert( !getLastInvite().isNull() );
		sendInviteOk(getLastInvite()->getDestinationBranch() );

		getMediaSession()->start();

		MRef<LogEntry *> logEntry = new LogEntryIncomingCompletedCall();

		logEntry->start = time( NULL );
		logEntry->peerSipUri = getLastInvite()->getFrom().getString();
		
		setLogEntry( logEntry );
		
		return true;
	}else{
		return false;
	}
}

bool SipDialogVoipServer::a3003_ringing_termwait_BYE( const SipSMCommand &command)
{
	
	if (transitionMatch("BYE", 
				command, 
				SipSMCommand::transaction_layer, 
				SipSMCommand::dialog_layer) &&
	    dialogState.remoteTag != ""){
		MRef<SipRequest*> bye = (SipRequest*) *command.getCommandPacket();

		//mdbg << "log stuff"<< end;
		if( getLogEntry() ){
			((LogEntrySuccess *)(*( getLogEntry() )))->duration = 
			time( NULL ) - getLogEntry()->start; 

			getLogEntry()->handle();
		}

/*
		MRef<SipTransaction*> byeresp = new SipTransactionNonInviteServer(sipStack, 
				//MRef<SipDialog*>(this), 
				bye->getCSeq(),
				bye->getCSeqMethod(),
				bye->getLastViaBranch(), 
				dialogState.callId); //TODO: remove second argument

		dispatcher->getLayerTransaction()->addTransaction(byeresp);
		registerTransactionToDialog(byeresp);

		
		SipSMCommand cmd(command);
		cmd.setDestination(SipSMCommand::transaction_layer);
		cmd.setSource(command.getSource());
		dispatcher->enqueueCommand(cmd, HIGH_PRIO_QUEUE);
*/
		sendByeOk(bye, /*byeresp->getBranch()*/"" );

		CommandString cmdstr(dialogState.callId, SipCommandString::remote_hang_up);
		dispatcher->getCallback()->handleCommand("gui", cmdstr);

		getMediaSession()->stop();

		
		signalIfNoTransactions();
		return true;
	}else{
		return false;
	}
}

bool SipDialogVoipServer::a3004_ringing_termwait_CANCEL( const SipSMCommand &command)
{

	if (transitionMatch("CANCEL", command, SipSMCommand::transaction_layer,
			    SipSMCommand::dialog_layer)) {
		MRef<SipRequest*> cancel =
			(SipRequest*)*command.getCommandPacket();
		const string branch = cancel->getLastViaBranch();

		if (getLastInvite()->getLastViaBranch() != branch)
			return false;

#if 0
		// Create CANCEL transaction and handle the request
		MRef<SipTransaction*> cr( 
			new SipTransactionNonInviteServer(sipStack, 
				//MRef<SipDialog*>(this), 
				command.getCommandPacket()->getCSeq(), 
				command.getCommandPacket()->getCSeqMethod(), 
				command.getCommandPacket()->getLastViaBranch(), 
				dialogState.callId) );
		dispatcher->getLayerTransaction()->addTransaction(cr);
		registerTransactionToDialog(cr);

		SipSMCommand cmd(command);
		cmd.setDestination(SipSMCommand::transaction_layer);

		cr->handleCommand(cmd);
#endif

		// Send 487 Request Cancelled for INVITE
		MRef<SipResponse*> cancelledResp = new SipResponse( 
				branch, 
				487,
				"Request Cancelled", 
				*getLastInvite() 
				);
		cancelledResp->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
		MRef<SipMessage*> cancelledMsg(*cancelledResp);
		SipSMCommand cancelledCmd( cancelledMsg, SipSMCommand::dialog_layer,
				  SipSMCommand::transaction_layer);
		dispatcher->enqueueCommand(cancelledCmd, HIGH_PRIO_QUEUE/*, PRIO_LAST_IN_QUEUE*/);

		// Send 200 OK for CANCEL
		MRef<SipResponse*> okResp = new SipResponse( branch, 200,"OK", MRef<SipMessage*>(*cancel) );
		okResp->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
		MRef<SipMessage*> okMsg(*okResp);
		SipSMCommand okCmd( okMsg, SipSMCommand::dialog_layer,
				  SipSMCommand::transaction_layer);
		//cr->handleCommand(okCmd);
		dispatcher->enqueueCommand(okCmd, HIGH_PRIO_QUEUE);

		/* Tell the GUI */
		CommandString cmdstr(dialogState.callId, SipCommandString::remote_cancelled_invite,"");
		dispatcher->getCallback()->handleCommand("gui", cmdstr );

		getMediaSession()->stop();
		signalIfNoTransactions();
		return true;
	}else{
		return false;
	}
}

bool SipDialogVoipServer::a3005_ringing_termwait_reject( const SipSMCommand &command)
{
	
	if (		transitionMatch(command, 
				SipCommandString::reject_invite,
				SipSMCommand::dialog_layer,
				SipSMCommand::dialog_layer) || 
			transitionMatch(command,
				SipCommandString::hang_up,
				SipSMCommand::dialog_layer,
				SipSMCommand::dialog_layer)){


		sendReject( getLastInvite()->getDestinationBranch() );

		getMediaSession()->stop();
		signalIfNoTransactions();
		return true;
	}else{
		return false;
	}
}

//In case a3001 does not accept the invite
bool SipDialogVoipServer::a3006_start_termwait_INVITE( const SipSMCommand &command)
{	
	if (transitionMatch("INVITE", command, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer)){

		setLastInvite(MRef<SipRequest*>((SipRequest *)*command.getCommandPacket()));
		
		dialogState.updateState( getLastInvite() );

#if 0
		MRef<SipTransaction*> ir( new SipTransactionInviteServerUA(sipStack, 
						//MRef<SipDialog*>(this), 
						command.getCommandPacket()->getCSeq(), 
						command.getCommandPacket()->getCSeqMethod(), 
						command.getCommandPacket()->getLastViaBranch(), 
						dialogState.callId ));

		dispatcher->getLayerTransaction()->addTransaction(ir);
		registerTransactionToDialog(ir);

		SipSMCommand cmd(command);
		cmd.setDestination(SipSMCommand::transaction_layer);

		dispatcher->enqueueCommand(cmd, HIGH_PRIO_QUEUE/*, PRIO_LAST_IN_QUEUE*/);
#endif
		sendNotAcceptable( command.getCommandPacket()->getDestinationBranch() );

		signalIfNoTransactions();
		return true;
	}else{
		return false;
	}
}

void SipDialogVoipServer::setUpStateMachine(){

	State<SipSMCommand,string> *s_start=new State<SipSMCommand,string>(this,"start");
	addState(s_start);

	State<SipSMCommand,string> *s_ringing=new State<SipSMCommand,string>(this,"ringing");
	addState(s_ringing);

	MRef<State<SipSMCommand,string> *> s_incall = getState("incall");
	MRef<State<SipSMCommand,string> *> s_termwait= getState("termwait");


	new StateTransition<SipSMCommand,string>(this, "transition_start_ringing_INVITE",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &SipDialogVoipServer::a3001_start_ringing_INVITE,
			s_start, s_ringing);

	new StateTransition<SipSMCommand,string>(this, "transition_ringing_incall_accept",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &SipDialogVoipServer::a3002_ringing_incall_accept,
			s_ringing, s_incall);

	new StateTransition<SipSMCommand,string>(this, "transition_incall_termwait_BYE",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &SipDialogVoipServer::a3003_ringing_termwait_BYE,
			s_ringing, s_termwait); 

	new StateTransition<SipSMCommand,string>(this, "transition_ringing_termwait_CANCEL",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &SipDialogVoipServer::a3004_ringing_termwait_CANCEL,
			s_ringing, s_termwait);
	
	new StateTransition<SipSMCommand,string>(this, "transition_ringing_termwait_reject",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &SipDialogVoipServer::a3005_ringing_termwait_reject,
			s_ringing, s_termwait);

	new StateTransition<SipSMCommand,string>(this, "transition_start_termwait_INVITE",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &SipDialogVoipServer::a3006_start_termwait_INVITE,
			s_start, s_termwait);

	
	setCurrentState(s_start);
}


#ifdef IPSEC_SUPPORT
SipDialogVoipServer::SipDialogVoipServer(MRef<SipStack*> stack, MRef<SipDialogConfig*> callconfig, MRef<SipSoftPhoneConfiguration*> pconf, MRef<Session *> mediaSession, string cid, MRef<MsipIpsecAPI *> ipsecSession) : 
		SipDialogVoip(stack,callconfig,pconf,mediaSession,cid,ipsecSession)
#else
SipDialogVoipServer::SipDialogVoipServer(MRef<SipStack*> stack, MRef<SipDialogConfig*> callconfig, MRef<SipSoftPhoneConfiguration*> pconf, MRef<Session *> mediaSession, string cid) : 
		SipDialogVoip(stack, callconfig, pconf, mediaSession, cid) 
#endif

{
	setUpStateMachine();
}

SipDialogVoipServer::~SipDialogVoipServer(){	
	mediaSession->unregister();
}


void SipDialogVoipServer::sendInviteOk(const string &branch){
	MRef<SipResponse*> ok= new SipResponse(branch, 200,"OK", MRef<SipMessage*>(*getLastInvite()));	
	ok->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
	
	MRef<SipHeaderValue *> contact = 
		new SipHeaderValueContact( 
			getDialogConfig()->inherited->sipIdentity->getSipUri(),
			getDialogConfig()->inherited->externalContactIP,
			getDialogConfig()->inherited->getLocalSipPort(phoneconf->useSTUN),
			"", getDialogConfig()->inherited->getTransport(),
			-1); //set expires to -1, we do not use it (only in register)
	ok->addHeader( new SipHeader(*contact) );
	
	//There might be so that there are no SDP. Check!
	MRef<SdpPacket *> sdp;
	if (mediaSession){
#ifdef ENABLE_TS
		ts.save("getSdpAnswer");
#endif
		sdp = mediaSession->getSdpAnswer();
#ifdef ENABLE_TS
		ts.save("getSdpAnswer");
#endif
		if( !sdp ){
		// FIXME: this most probably means that the
		// creation of the MIKEY message failed, it 
		// should not happen
		merr << "Sdp was NULL in sendInviteOk" << end;
		return; 
		}
	}
	
	/* Add the latter to the INVITE message */ // If it exists
	

//-------------------------------------------------------------------------------------------------------------//
#ifdef IPSEC_SUPPORT	
	// Create a MIKEY message for IPSEC if stated in the config file.
	MRef<SipMimeContent*> mikey;
	if (getIpsecSession()->required()){
		ts.save("getMikeyIpsecAnswer");
		mikey = ipsecSession->getMikeyIpsecAnswer();
		ts.save("getMikeyIpsecAnswer");
		if (!mikey){
			merr << "Mikey was NULL in sendInviteOk" << end;
			merr << "Still some errors with IPSEC" << end;
			//return; 
		}
	}
	else
		mikey = NULL;
	MRef<SipMimeContent*> multi;
	if (mikey && mediaSession){
		multi = new SipMimeContent("multipart/mixed");
		multi->addPart(*mikey);
		multi->addPart(*sdp);
		ok->setContent( *multi);
	}
	if (mikey && !mediaSession)
		ok->setContent( *mikey);
	if (!mikey && mediaSession)
		ok->setContent( *sdp );
#else
	
	ok->setContent( *sdp );
#endif
//-------------------------------------------------------------------------------------------------------------//
//	/* Get the SDP Answer from the MediaSession */
//	MRef<SdpPacket *> sdpAnswer = mediaSession->getSdpAnswer();
//
//	if( sdpAnswer ){
//		ok->setContent( *sdpAnswer );
//	}
//	/* if sdp is NULL, the offer was refused, send 606 */
//	// FIXME
//	else return; 

	MRef<SipMessage*> pref(*ok);
	SipSMCommand cmd( pref, SipSMCommand::dialog_layer, SipSMCommand::transaction_layer);
	dispatcher->enqueueCommand(cmd, HIGH_PRIO_QUEUE/*, PRIO_LAST_IN_QUEUE*/);
}

void SipDialogVoipServer::sendReject(const string &branch){
	MRef<SipResponse*> ringing = new SipResponse(branch,486,"Temporary unavailable", MRef<SipMessage*>(*getLastInvite()));	
	ringing->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
	MRef<SipMessage*> pref(*ringing);
	SipSMCommand cmd( pref,SipSMCommand::dialog_layer, SipSMCommand::transaction_layer);
	dispatcher->enqueueCommand(cmd, HIGH_PRIO_QUEUE/*, PRIO_LAST_IN_QUEUE*/);
}

void SipDialogVoipServer::sendRinging(const string &branch){
	MRef<SipResponse*> ringing = new SipResponse(branch,180,"Ringing", MRef<SipMessage*>(*getLastInvite()));	

	//ringing->addHeader(new SipHeader(new SipHeaderValueRequire("100rel"))); //EEEEEE
	
	ringing->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
	
	MRef<SipHeaderValue *> contact = 
		new SipHeaderValueContact( 
			getDialogConfig()->inherited->sipIdentity->getSipUri(),
			getDialogConfig()->inherited->externalContactIP,
			getDialogConfig()->inherited->getLocalSipPort(phoneconf->useSTUN),
			"", getDialogConfig()->inherited->getTransport(),
			-1); //set expires to -1, we do not use it (only in register)
	ringing->addHeader( new SipHeader(*contact) );

	MRef<SipMessage*> pref(*ringing);
	SipSMCommand cmd( pref, SipSMCommand::dialog_layer, SipSMCommand::transaction_layer);
	dispatcher->enqueueCommand(cmd, HIGH_PRIO_QUEUE/*, PRIO_LAST_IN_QUEUE*/);
}

void SipDialogVoipServer::sendNotAcceptable(const string &branch){
	MRef<SipResponse*> not_acceptable = new SipResponse(branch,606,"Not Acceptable", MRef<SipMessage*>(*getLastInvite()));	
	if( mediaSession && mediaSession->getErrorString() != "" ){
		not_acceptable->addHeader( 
			new SipHeader(
				new SipHeaderValueWarning(
					getDialogConfig()->inherited->externalContactIP, 
					399, 
					mediaSession->getErrorString() ) ));
	}

	not_acceptable->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
	MRef<SipMessage*> pref(*not_acceptable);
	SipSMCommand cmd( pref, SipSMCommand::dialog_layer, SipSMCommand::transaction_layer);
	dispatcher->enqueueCommand(cmd, HIGH_PRIO_QUEUE/*, PRIO_LAST_IN_QUEUE*/);
}
