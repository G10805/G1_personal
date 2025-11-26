
/*********************************************************************
 Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

package vendor.qti.imsrcs.siptransport;

import android.telephony.ims.SipMessage;
import android.util.Log;

import java.lang.String;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class ImsSipMessageParser {

    private final String LOGTAG = "ImsSipMessageParser";

    public static final int PROTOCOL_UNSPECIFIC = -1;
    public static final int PROTOCOL_UDP        = 0;
    public static final int PROTOCOL_TCP        = 1;

    public static final int MESSAGETYPE_UNSPECIFIC = -1;
    public static final int MESSAGETYPE_REQUEST    = 0;
    public static final int MESSAGETYPE_RESPONSE   = 1;

    public static final int SESSION_STATUS_UNKNOWN    = 0;
    public static final int SESSION_STATUS_ACTIVE     = 1;
    public static final int SESSION_STATUS_HOLD       = 2;
    public static final int SESSION_STATUS_INACTIVE   = 3;

    private String startLine;
    private String headerSection;
    private String messageContent;
    private String acceptContactFt;
    private Boolean isSDPPresent;
    private boolean isInviteMessage;
    private boolean isByeMessage;

    private String callID;
    private String outboundProxy;
    private int messageType;
    private int protocol;
    private int remotePort;

    private int sessionStatus;

    /* used in send sip message scenario,
     * parse SipMessage from Java Client
     */
    public ImsSipMessageParser(SipMessage msg) {
        startLine = msg.getStartLine();
        headerSection  = msg.getHeaderSection();
        messageContent = msg.getContent().toString();

        callID = msg.getCallIdParameter();
        parseSipMessage();
    }

    /* used in receive sip message scenario,
     * parse string-formed sip message from native/modem
     */
    public ImsSipMessageParser(String sipMsg) {
        int beginPosOfHeader = 0;
        int endPosOfHeader = 0;

        // Line end is line feed ('\n') or carriage return ('\r'), or \r\n;
        // get startLine
        int endPosOfStartLine = sipMsg.indexOf("\r\n", 0);
        startLine = sipMsg.substring(0, endPosOfStartLine);
		//TOCHECK: Needs further review before merge
        startLine = startLine.concat("\r\n");

        // get Header Section
        beginPosOfHeader = endPosOfStartLine + 2;
        endPosOfHeader = sipMsg.indexOf("\r\n\r\n", beginPosOfHeader);
        if (endPosOfHeader != -1) { // SIP message have content
            headerSection = sipMsg.substring(beginPosOfHeader, endPosOfHeader);
        }
        else // SIP message don't have content, have only header
            headerSection = sipMsg.substring(beginPosOfHeader);

        // copy message content
        if (endPosOfHeader != -1)
        {
            messageContent = sipMsg.substring(endPosOfHeader+4);
        }

        // get CallID
        String callIDName = "Call-ID:";
        int beginPosOfCallID = headerSection.indexOf(callIDName);
        if (beginPosOfCallID == -1) {
            String callIDNameCompact = "i:";
            beginPosOfCallID = headerSection.indexOf(callIDNameCompact);
            beginPosOfCallID += callIDNameCompact.length();
        }
        else {
            beginPosOfCallID += callIDName.length();
        }
        int endPosOfCallID = headerSection.indexOf("\r\n", beginPosOfCallID);
        callID = headerSection.substring(beginPosOfCallID, endPosOfCallID).trim();

        parseSipMessage();
    }


    /*
     * parse SIP message from string startLine/headerSection/sdpContent
     * to get parameters need for SipTransport/SipConnection
     */
    private Boolean parseSipMessage() {
        outboundProxy = "";
        messageType = MESSAGETYPE_UNSPECIFIC;
        protocol = PROTOCOL_UNSPECIFIC;
        remotePort = 0;
        isInviteMessage = false;
        isByeMessage = false;
        sessionStatus = SESSION_STATUS_UNKNOWN;

        checkSDPStatus();

        String[] startLineSplit = startLine.split(" ");
        if (startLineSplit[0].startsWith("SIP/")) {
            // it is status line with format: SIP-Version SP Status-Code SP Reason-Phrase CRLF
            messageType = MESSAGETYPE_RESPONSE;
        } else {
            // it is request line with format: Method SP Request-URI SP SIP-Version CRLF
            messageType = MESSAGETYPE_REQUEST;
        }

        //TOCHECK: Needs further review before merge
        headerSection = headerSection.concat("\r\n");

        String[] headerSplit = headerSection.split("\r\n", 0);
        String routeHeaderName = "Route:";
        for (int i = 0; i < headerSplit.length; i++) {

            // System.out.println("process for line :" + headerSplit[i]);
            if (headerSplit[i].startsWith("Via:") || headerSplit[i].startsWith("v:")) {
                parseViaHeader(headerSplit[i]);
            }
            else if (headerSplit[i].startsWith(routeHeaderName)) {
                parseRouteHeader(headerSplit[i].substring(routeHeaderName.length()));
            }
            else if(headerSplit[i].startsWith("Accept-Contact")) {
                //copy the feature tags
                String[] acceptContactSplit = headerSplit[i].split(" *;",0);
                acceptContactFt = "";
                if(acceptContactSplit.length > 1) {
                    acceptContactFt = acceptContactSplit[1].replace("\r\n","");
                }
            }
            else if(headerSplit[i].startsWith("CSeq:")) {
                if( messageType == MESSAGETYPE_REQUEST ||
                        (messageType == MESSAGETYPE_RESPONSE &&
                                   startLineSplit[1].equals("200"))) {
                    parseCSeqHeader(headerSplit[i]);
                }
            }
        }

        if(isInviteMessage) {
            parseMessageContent();
        } else if(isByeMessage) {
            sessionStatus = SESSION_STATUS_INACTIVE;
        }

        return true;

    }

    private void checkSDPStatus() {
        // get SDP content
        int contentTypePos = headerSection.indexOf("c: application/sdp");
        if (contentTypePos == -1)
            contentTypePos = headerSection.indexOf("Content-Type: application/sdp");

        if (contentTypePos != -1) {
            // confirmed that content is SDP
            isSDPPresent = true;
        }
    }

    /* Parse Via header to get protocol information */
    private void parseViaHeader(String viaHeader) {
        String viaHeaderSplit[] = viaHeader.split(" ");
        String viaSplit[] = viaHeaderSplit[1].split("/"); // split SIP/2.0/TCP to 3 parts;

        // System.out.println("via length :" + viaSplit.length);
        if (viaSplit.length == 3) {
           if (viaSplit[2].compareToIgnoreCase("TCP") == 0 ||
               viaSplit[2].compareToIgnoreCase("TLS") == 0 ||
               viaSplit[2].compareToIgnoreCase("SCTP") == 0) {
               protocol = PROTOCOL_TCP;
           }
           else if (viaSplit[2].compareToIgnoreCase("UDP") == 0) {
               protocol = PROTOCOL_UDP;
           }
           else {
               protocol = PROTOCOL_UNSPECIFIC;
               Log.e(LOGTAG, ("protocol[" + viaSplit[2] + "] is PROTOCOL_UNSPECIFIC"));
          }
        }
    }

    /* Parse Route Header to get outboundproxy and remote port*/
    private Boolean parseRouteHeader(String routeHeader) {
        /* RFC 3261:
         Route        =  "Route" HCOLON route-param *(COMMA route-param)
         route-param  =  name-addr *( SEMI rr-param )
         name-addr    =  [ display-name ] LAQUOT addr-spec RAQUOT
         addr-spec    =  SIP-URI / SIPS-URI / absoluteURI
         SIP-URI      =  "sip:" [ userinfo ] hostport
                             uri-parameters [ headers ]
         SIPS-URI     =  "sips:" [ userinfo ] hostport
                             uri-parameters [ headers ]
         hostport     =  host [ ":" port ]
         host         =  hostname / IPv4address / IPv6reference
         hostname     =  *( domainlabel "." ) toplabel [ "." ]
         toplabel     =  ALPHA / ALPHA *( alphanum / "-" ) alphanum
         IPv6reference  =  "[" IPv6address "]"
         IPv6address    =  hexpart [ ":" IPv4address ]
         IPv4address    =  1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT
      */
        // split Route with "," to get route-param list, will use the first one only
        String routeHeaderSplit[] = routeHeader.split(",", 2);
        String firstRoute = routeHeaderSplit[0];

        // use the first route-param, jump to SIP/SIPS URI start poistion
        String strSIPUrlPrefix = "sip:";
        String strSIPSUrlPrefux = "sips:";
        int startPos = firstRoute.indexOf(strSIPUrlPrefix);
        if (startPos == -1) {
            startPos = firstRoute.indexOf(strSIPSUrlPrefux);
            if (startPos == -1) {
                // unexpected Route Header value
            }
            startPos += strSIPSUrlPrefux.length();
        }
        else {
            startPos += strSIPUrlPrefix.length();
        }

        // System.out.println("firstRoute before skip userinfo:" + firstRoute);
        // skip userinfo
        int userInfoEnd = firstRoute.indexOf("@", startPos);
        if (userInfoEnd != -1)
            firstRoute = firstRoute.substring(userInfoEnd+1);
        else {
            firstRoute = firstRoute.substring(startPos);
            // System.out.println("firstRoute after skip userinfo:" + firstRoute);
        }

        // abstract the hostport string
        int hostportEndPos = firstRoute.indexOf(";"); // beginning of rr-param;
        firstRoute = firstRoute.substring(0, hostportEndPos);

        // System.out.println("firstRoute:" + firstRoute);
        if (firstRoute.startsWith("[")) {
            // IPv6 address scenario
            int addrEndPos = firstRoute.indexOf("]");
            outboundProxy = firstRoute.substring(1, addrEndPos);

            if (firstRoute.charAt(addrEndPos+1) == ':') {
                String portStr = firstRoute.substring(addrEndPos + 2);
                remotePort = Integer.parseInt(portStr);
            }
            else {
                // no port present;
                remotePort = 0;
            }
        }
        else {
            // domain-name and IPv4 scenario
            int portBeginPos = firstRoute.indexOf(":");

            if(portBeginPos != -1) {
              String portStr = firstRoute.substring(portBeginPos+1);
              firstRoute = firstRoute.substring(0, portBeginPos);
              remotePort = Integer.parseInt(portStr);
            }

            try {
                outboundProxy = InetAddress.getByName(firstRoute).getHostAddress();
            }
            catch (UnknownHostException e) {
                Log.e(LOGTAG, "outboundProxy parse failure on " + firstRoute);
            }
        }
        return true;
    }

    /* Parse CSeq header to check if the SIP message is one of INVITE or BYE */
    private void parseCSeqHeader(String cSeqHeader) {
        String cSeqHeaderSplit[] = cSeqHeader.split(" ");

        if (cSeqHeaderSplit.length == 3) {
           if (cSeqHeaderSplit[2].compareToIgnoreCase("INVITE") == 0) {
               isInviteMessage = true;
           } else if(cSeqHeaderSplit[2].compareToIgnoreCase("BYE") == 0) {
                isByeMessage = true;
           }
        }
    }

    private void parseMessageContent() {
       String[] contentSplit = messageContent.split("\r\n");

       for (int i = 0; i < contentSplit.length; i++) {
           if(contentSplit[i].startsWith("a=")) {
               if(contentSplit[i].contains("sendrecv")) {
                   sessionStatus = SESSION_STATUS_ACTIVE;
               } else if(contentSplit[i].contains("sendonly") ||
                         contentSplit[i].contains("recvonly") ){
                   sessionStatus = SESSION_STATUS_HOLD;
               }
           }
       }
    }

    public String getOutboundProxy() {
        return outboundProxy;
    }
    public int getProtocol() {
        return protocol;
    }
    public int getMessageType() {
        return messageType;
    }
    public int getRemotePort() {
        return remotePort;
    }
    public String getCallID() {
        return callID;
    }
    public String getStartLine() {
        return startLine;
    }
    public String getHeaderSection() {
        return headerSection;
    }
    public String getSdpContent() {
        if(isSDPPresent)
            return messageContent;
        else
            return "";
    }
    public String getMessageContent() {
        return messageContent;
    }
    public boolean isRequestMessage() {
        if(messageType == MESSAGETYPE_REQUEST) return true;
        return false;
    }
    public boolean isResponseMessage() {
        if(messageType == MESSAGETYPE_RESPONSE) return true;
        return false;
    }
    public boolean isInviteMessage() {
        return isInviteMessage;
    }
    public boolean isByeMessage() {
        return isByeMessage;
    }
    public boolean isSessionStatusActive() {
        if(sessionStatus == SESSION_STATUS_ACTIVE) return true;
        return false;
    }
    public boolean isSessionStatusHold() {
        if(sessionStatus == SESSION_STATUS_HOLD) return true;
        return false;
    }
    public String getAcceptContactFt() {
        return acceptContactFt;
    }
    // before build/run main test, comment out few android related code
    public static void main(String[] args){

        System.out.println("Invite Test:\r\n" );

        String inviteMsg = "INVITE tel:15301378925;phone-context=ims.mnc000.mcc460.3gppnetwork.org SIP/2.0\r\n";
        inviteMsg += "f: <sip:+8613761248767@sh.ims.mnc000.mcc460.3gppnetwork.org>;tag=3439550951\r\n";
        inviteMsg += "t: <tel:15301378925;phone-context=ims.mnc000.mcc460.3gppnetwork.org>\r\n";
        inviteMsg += "CSeq: 218325474 INVITE\r\n";
        inviteMsg += "i: 3439550946_1333397176@2409:8100:2d90:125b:60f7:edff:fef7:b9ac\r\n";
        inviteMsg += "v: SIP/2.0/TCP [2409:8100:2d90:125b:60f7:edff:fef7:b9ac]:41937;branch=z9hG4bK3576816423\r\n";
        inviteMsg += "Max-Forwards: 70\r\n";
        inviteMsg += "m: <sip:0a281f06-b191-4a72-a03a-ebdc2c0803e8@[2409:8100:2d90:125b:60f7:edff:fef7:b9ac]:41937>;+g.3gpp.ps2cs-srvcc-orig-pre-alerting\r\n";
        inviteMsg += "Route: <sip:[2409:8000:5002:1203:2::]:9900;lr>,<sip:orig@shscscf4bhw.sh.chinamobile.com;lr;Dpt=7e74_cdd76246;ca=175e6;TRC=ffffffff-ffffffff>\r\n";
        inviteMsg += "P-Access-Network-Info: 3GPP-E-UTRAN-TDD; utran-cell-id-3gpp=46000113B1499403\r\n";
        inviteMsg += "Security-Verify: ipsec-3gpp;alg=hmac-md5-96;prot=esp;mod=trans;ealg=null;spi-c=2198232621;spi-s=4228275757;port-c=9950;port-s=9900\r\n";
        inviteMsg += "Proxy-Require: sec-agree\r\n";
        inviteMsg += "Require: sec-agree\r\n";
        inviteMsg += "P-Preferred-Identity: <sip:+8613761248767@sh.ims.mnc000.mcc460.3gppnetwork.org>\r\n";
        inviteMsg += "Allow: INVITE,ACK,CANCEL,BYE,UPDATE,PRACK,MESSAGE,REFER,NOTIFY,INFO,OPTIONS\r\n";
        inviteMsg += "c: application/sdp\r\n";
        inviteMsg += "l: 804\r\n";
        inviteMsg += "\r\n";
        inviteMsg += "v=0\r\n";
        inviteMsg += "o=- 1 1000 IN IP6 2409:8100:2d90:125b:60f7:edff:fef7:b9ac\r\n";
        inviteMsg += "s=QC VOIP\r\n";
        inviteMsg += "c=IN IP6 2409:8100:2d90:125b:60f7:edff:fef7:b9ac\r\n";
        inviteMsg += "b=AS:50\r\n";
        inviteMsg += "b=RS:600\r\n";
        inviteMsg += "b=RR:2000\r\n";
        inviteMsg += "t=0 0\r\n";
        inviteMsg += "m=audio 50018 RTP/AVP 126 127 104 102 96 97\r\n";
        inviteMsg += "b=AS:50\r\n";
        inviteMsg += "b=RS:600\r\n";
        inviteMsg += "b=RR:2000\r\n";
        inviteMsg += "a=rtpmap:126 EVS/16000/1\r\n";
        inviteMsg += "a=fmtp:126 br=9.6-24.4;bw=swb;ch-aw-recv=2;max-red=0\r\n";
        inviteMsg += "a=rtpmap:127 EVS/16000/1\r\n";
        inviteMsg += "a=fmtp:127 br=9.6-24.4;bw=wb;ch-aw-recv=2;max-red=0\r\n";
        inviteMsg += "a=rtpmap:104 AMR-WB/16000/1\r\n";
        inviteMsg += "a=fmtp:104 mode-change-capability=2;max-red=0\r\n";
        inviteMsg += "a=rtpmap:102 AMR/8000/1\r\n";
        inviteMsg += "a=fmtp:102 mode-change-capability=2;max-red=0\r\n";
        inviteMsg += "a=rtpmap:96 telephone-event/16000\r\n";
        inviteMsg += "a=fmtp:96 0-15\r\n";
        inviteMsg += "a=rtpmap:97 telephone-event/8000\r\n";
        inviteMsg += "a=fmtp:97 0-15\r\n";
        inviteMsg += "a=curr:qos local none\r\n";
        inviteMsg += "a=curr:qos remote none\r\n";
        inviteMsg += "a=des:qos mandatory local sendrecv\r\n";
        inviteMsg += "a=des:qos optional remote sendrecv\r\n";
        inviteMsg += "a=sendrecv\r\n";
        inviteMsg += "a=maxptime:240\r\n";
        inviteMsg += "a=ptime:20\r\n";

        ImsSipMessageParser parser = new ImsSipMessageParser(inviteMsg);
        System.out.println("getStartLine=" + parser.getStartLine());
        System.out.println("getHeaderSection=" + parser.getHeaderSection());
        System.out.println("getSdpContent=" + parser.getSdpContent());
        System.out.println("getCallID=" + parser.getCallID());
        System.out.println("getMessageType=" + parser.getMessageType());
        System.out.println("getProtocol=" + parser.getProtocol());
        System.out.println("outboundProxy=" + parser.getOutboundProxy());
        System.out.println("remoteport=" + parser.getRemotePort());

        String progressMsg = "SIP/2.0 183 Session Progress\r\n";
        progressMsg += "Via: SIP/2.0/TCP [2409:8100:2D90:125B:60F7:EDFF:FEF7:B9AC]:41937;branch=z9hG4bK3576816423\r\n";
        progressMsg += "Record-Route: <sip:[2409:8000:5002:1203:0002:0000:0000:0000]:9900;transport=tcp;lr;Hpt=8f22_116;CxtId=3;TRC=ffffffff-ffffffff;X-HwB2bUaCookie=416>\r\n";
        progressMsg += "Call-ID: 3439550946_1333397176@2409:8100:2d90:125b:60f7:edff:fef7:b9ac\r\n";
        progressMsg += "From: <sip:+8613761248767@sh.ims.mnc000.mcc460.3gppnetwork.org>;tag=3439550951\r\n";
        progressMsg += "To: <tel:15301378925;phone-context=ims.mnc000.mcc460.3gppnetwork.org>;tag=xjtw9wtw\r\n";
        progressMsg += "CSeq: 218325474 INVITE\r\n";
        progressMsg += "Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,INFO,PRACK,NOTIFY,MESSAGE,REFER,UPDATE\r\n";
        progressMsg += "Contact: <sip:[2409:8000:5002:1203:0002:0000:0000:0000]:9900;Hpt=8f22_16;CxtId=3;TRC=ffffffff-ffffffff>\r\n";
        progressMsg += "Require: 100rel,precondition\r\n";
        progressMsg += "RSeq: 1\r\n";
        progressMsg += "Content-Length: 519\r\n";
        progressMsg += "Content-Type: application/sdp\r\n";
        progressMsg += "\r\n";
        progressMsg += "v=0\r\n";
        progressMsg += "o=- 163441475 163441475 IN IP6 2409:8000:5002:1203:0002:0000:0000:0003\r\n";
        progressMsg += "s=SBC call\r\n";
        progressMsg += "c=IN IP6 2409:8000:5002:1203:0002:0000:0000:0003\r\n";
        progressMsg += "t=0 0\r\n";
        progressMsg += "m=audio 16512 RTP/AVP 102 97\r\n";
        progressMsg += "a=rtpmap:102 AMR/8000\r\n";
        progressMsg += "a=fmtp:102 mode-set=7;mode-change-neighbor=1;mode-change-period=2;mode-change-capability=2\r\n";
        progressMsg += "a=ptime:20\r\n";
        progressMsg += "a=maxptime:20\r\n";
        progressMsg += "a=curr:qos local sendrecv\r\n";
        progressMsg += "a=curr:qos remote none\r\n";
        progressMsg += "a=des:qos optional local sendrecv\r\n";
        progressMsg += "a=des:qos mandatory remote sendrecv\r\n";
        progressMsg += "a=conf:qos remote sendrecv\r\n";
        progressMsg += "a=rtpmap:97 telephone-event/8000\r\n";
        progressMsg += "a=fmtp:97 0-15\r\n";

    System.out.println("183 Test:\r\n" );
    parser = new ImsSipMessageParser(progressMsg);
    System.out.println("getStartLine=" + parser.getStartLine());
    System.out.println("getHeaderSection=" + parser.getHeaderSection());
    System.out.println("getSdpContent=" + parser.getSdpContent());
    System.out.println("getCallID=" + parser.getCallID());
    System.out.println("getMessageType=" + parser.getMessageType());
    System.out.println("getProtocol=" + parser.getProtocol());
    System.out.println("outboundProxy=" + parser.getOutboundProxy());
    System.out.println("remoteport=" + parser.getRemotePort());


    }

}
