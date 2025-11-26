# ============================================================================
#
#    Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
#    Qualcomm Technologies Proprietary and Confidential.
#
# ============================================================================

import datetime
import os
import json
import sys
import re
# import pg
from WikiCache import Change, Cache
import time
from subprocess import Popen, PIPE, STDOUT
import cgi
import shlex
import git

def usage():
    print 'usage: %s [--debug] <branchname> '% (sys.argv[0])
    sys.exit(1)

def readGitCommits(allCommits):
  commits = []
  if DEBUG: sys.stderr.write("allCommits=%s\n" % (allCommits))
  for p,changes in allCommits:
    for c in changes:
      commits.append((p,c.id,c.message))
  return commits

crRE = re.compile(r'CRs-fixed:\s*([\s,0-9]+)', re.IGNORECASE|re.MULTILINE)
crSubRE = re.compile(r'([0-9]+)[,\s]*')
cpckRE = re.compile('.*\(cherry picked from commit ([0-9a-f]*)\).*')
#gerritRE = re.compile(r'https?://([^/]*)/(?:#change,)?([0-9]+)')

def huntCRsInComments(change, path):
  # if DEBUG: sys.stderr.write('change=%s\n' % (change))
  if not change.isdigit():
    sys.stderr('change %s not a digit\n' % str(change))
    return []
  dbcnxn = GetGerritServerCnxn(path)
  if dbcnxn == None:
    return []
  rows = dbcnxn.query("SELECT * FROM change_messages WHERE change_id = %s" % (change))
  crs = []
  for record in rows.dictresult():
    crList = crRE.findall(record['message'])
    crs += extractCRnumbers(crList)
  # if DEBUG: sys.stderr.write('comments: %s\n' % (str(crs)))
  return crs

def extractCRnumbers(crList):
  global DEBUG
  crs = []
  for cr in crList:
    for cr1 in re.split('[ \n\t,]+', cr):
      if cr1 != '':
        crs.append(cr1.rstrip())
  # if DEBUG: sys.stderr.write("Found CRs=%s\n" % (crs))
  return crs

def huntCRs(text):
  crList = crRE.findall(text)
  return extractCRnumbers(crList)

def dedup(l):
    return list(set(l))

def formatCRs(crList):
    crs = ""
    for cr in crList:
        #crs = crs + crSubRE.sub(r'<a href="http://prism/CR/\1">\1</a> ', cr)
        crs = crs +" "+cr
    return crs


GREY_COLOUR = '#808080'

def getNumberFromURL(url):
  n = url.rfind(',')
  if n == -1:
    n = url.rfind('/')
  return url[n+1:]

def generateAUtable(page_fob, allCommits, cache, au, commits):
    global DEBUG
    count = 0
    url=" "
    sourcegerriturl=" "
    if commits == None:
        cachedCommits = False
        commits = readGitCommits(allCommits)
    else:
        cachedCommits = True
    for c in commits:
        #bgcolor is default if not master.  If master is set to DDD8C2
        backcolour = ''
        isMaster = False
        found = False
        if cachedCommits == False:
            path,commit,commitText = c
            if DEBUG: sys.stderr.write("commitText=%s\n" % (commitText))
            sourcecommitid=""
            sourcegerriturl=" "
            cc = cache.getCommit(commit)
        else:
            cc = c
        if cachedCommits == True:
            if DEBUG: sys.stderr.write('cc=%s\n' % (cc))
            #sourcegerriturl = cc.getMainURL()
            sourcegerriturl=" "
            url = cc.getBrnchURL()
            number = getNumberFromURL(url)
            subject = cc.getSubject()
	    path = cc.getPath()
            project = cc.getProject()
            commitText = cc.getCommitText()
            if DEBUG: sys.stderr.write("Commit CRs=%s, comments CRs=%s\n" %
                                       (cc.getCommitCRs(), huntCRsInComments(number, path)))
            crlist = cc.getCommitCRs() + huntCRsInComments(number, path)
            if cc.getBranch() == 'master':
                backcolour = 'bgcolor="#DDD8C2" title="Branch:Master"'
            if cc.getBranch() == 'dev': #ignore changes in branch dev
                continue
            if sourcegerriturl != " ":
                srcnumber = getNumberFromURL(sourcegerriturl)
                crlist +=  huntCRsInComments(srcnumber, path)
        else:
            path,commit,commitText = c
            url=path
	    lines=commitText.split("\n")
	    subject=lines[0]
	    path=path
            project=commit[:10]
            crlist=""
            if DEBUG: sys.stderr.write("Hunting CRs in %s" % (commitText))
            commitCRs = huntCRs(commitText)
            crlist = commitCRs
            searchres= cpckRE.search(commitText)
            if searchres != None:
              sourcecommitid = cpckRE.search(commitText).group(1)
              if sourcecommitid != None:
                sourcegerrit = sourcecommitid
            change = Change(commit, url, commitText, sourcecommitid, sourcegerriturl, subject, map(lambda x: x.rstrip(), crlist), branch, project, path)
            cache.add(change, au)
        if url != " ":
            lastSlash = url.rfind('/')
            url = url[:lastSlash+1]+url[lastSlash+1:]
        if sourcegerriturl != " ":
            lastSlash = sourcegerriturl.rfind('/')
            sourcegerriturl = sourcegerriturl[:lastSlash+1]+sourcegerriturl[lastSlash+1:]

        # prune duplicates
        crlist = dedup(crlist)

        #replace HTML special characters found in subject
	subject = cgi.escape(subject);

        #replace HTML special char in commit details popup
        newCommitText = cgi.escape(commitText)

        count += 1
        print >>page_fob, """        <tr %s><td class="col1">%s</td><td class="col2">%s</td><td class = "col3" title="%s">%s</td><td class="col4">%s</td></tr>"""  % \
                (backcolour,
                 url,
                 project,
                 newCommitText,
                 subject,
                 formatCRs(crlist))

        found = True
    #append an empty line if this AU had no changes
    if count == 0:
        print >>page_fob, '''
            <tr><td>No changes</td></tr>
              </table></td></tr>
            </tbody>
          </table>
          '''
    else:
        print >>page_fob, '''
              </table></td></tr>
            </tbody>
          </table>
          '''
    return count

def html_generate_head(page_fob, num):
  '''Generate the <head> portion of the html, including javascript and css'''
  print >>page_fob, '''
  <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
  <html>
  <head>
  <meta http-equiv="Content-type" content="text/html;charset=UTF-8">
  <title>Cherrypick Log</title>
  <script type="text/javascript">
  var numAUs = %s;
  ''' % (num)

  #cookie management functions to remember
  #expand/collapse states
  print >>page_fob, '''
  function createCookie(name,value,days) {
    if (days) {
      var date = new Date();
      date.setTime(date.getTime()+(days*24*60*60*1000));
      var expires = "; expires="+date.toGMTString();}
    else var expires = "";
    document.cookie = name+"="+value+expires+"; path=/";}

  function readCookie(name) {
    var nameEQ = name + "=";
    var ca = document.cookie.split(\';\');
    for(var i=0;i < ca.length;i++) {
      var c = ca[i];
        while (c.charAt(0)==\' \') c = c.substring(1,c.length);
	if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
    }
    return null;
  }

  function eraseCookie(name) {
    createCookie(name,"",-1);
  }

  //called on every load to see if cookies exist
  //or to invalidate cookies from a previous version of the page
  function checkCookie()
  {
    var nc = readCookie("numAU");
    if(nc && (parseInt(nc) == numAUs)){
      for (var i=1; i<= numAUs; i++)
      {
        var e = readCookie("tbl"+i);
          if(e){
	  if(document.all){document.getElementById("tbl"+i).style.display = "block";}
	  else{document.getElementById("tbl"+i).style.display = "table";}
            document.getElementById("lnk"+i).value = "[-]";
	}
      }
    }
    else{
      createCookie("numAU", numAUs.toString(), 7);
      for (var i=1; i<= numAUs; i++)
      {
        var e = readCookie("tbl"+i);
        if(e){
	eraseCookie("tbl"+i);
        }
      }
    }
  }
  '''
  #start scripts used to expand/collapse
  print >>page_fob, '''
    //collapse/expand functions
  function toggle(tbid,lnkid)
  {
    if(document.all){
      if(document.getElementById(tbid).style.display == "block"){
        document.getElementById(tbid).style.display = "none";
        eraseCookie(tbid);
      }else{
        document.getElementById(tbid).style.display = "block";
        createCookie(tbid, "E", 7);
      }
    }
    else{
      if(document.getElementById(tbid).style.display == "table"){
        document.getElementById(tbid).style.display = "none";
        eraseCookie(tbid);
      }else{
        document.getElementById(tbid).style.display = "table";
        createCookie(tbid, "E", 7);
      }
    }
    document.getElementById(lnkid).value = document.getElementById(lnkid).value == "[-]" ? "[+]" : "[-]";
  }
  function expand_one(tbid,lnkid)
  {
    if(document.all){document.getElementById(tbid).style.display = "block";}
    else{document.getElementById(tbid).style.display = "table";}
    document.getElementById(lnkid).value = "[-]";
    createCookie(tbid, "E", 7);
  }
  function expand_all()
  {
    for (var i=1; i<=numAUs; i++)
      {
        expand_one("tbl"+i,"lnk"+i);
      }
  }
  function collapse_all(){
    for (var i=1; i<= numAUs; i++)
    {
      if(document.all){document.getElementById("tbl"+i).style.display = "none";}
      else{document.getElementById("tbl"+i).style.display = "none";}
      document.getElementById("lnk"+i).value = "[+]";
      eraseCookie("tbl"+i);
    }
  }
  function stopProp(e) {
    e.cancelBubble = true;
    if (e.stopPropagation) e.stopPropagation();
  }
  </script>
  '''

  #css rules
  print >>page_fob, '''
  <style type="text/css">
  #IE {text-align:center;}
  #outside {text-align:left;width:90%;min-width:768px;margin:0 auto;font-family:Calibri,Arial Unicode MS, Arial, sans-serif;word-wrap:break-word;}
  #banner {background: #BEE395;padding-top:10px;padding-bottom:5px;text-align:center;}
  #expandcollapseallcontainer {text-align:right;}
  #expandall,#collapseall {border:none;background:#FEE395;width:100px;padding-top:10px;padding-bottom:10px;font-size:14px;}
  #expandall:hover {font-weight:bold;}
  #collapseall:hover {font-weight:bold;}
  #labelcontainer{padding-bottom:2px;background:#FFFFFF;}
  #labels {border-top-left-radius: 15px;border-top-right-radius: 15px;background:#C6D9F1;font-size:14px;padding-top:10px;}
  .innertable {display:none;table-layout:fixed;background: #EEECE1}
  .innertable tr:hover td {background: #d0dafd;}
  .fixed{table-layout:fixed;}
  .expandcollapsebutton {border:none;background:#BEE395;padding-top:10px;padding-bottom:10px;font-size:16px;}
  .col1{width:17%;}
  .col2{width:14%;}
  .col3{width:54%;}
  .col4{width:20%;}
  .col5{width:7%;}
  .col6{width:25%;}
  .col7{width:75%;}
  .hcol1{width:40%;background:#BEE395;text-align:center;}
  .hcol2{width:30%;background:#BEE395;text-align:center;}
  .hcol3{width:20%;background:#BEE395;text-align:center;}
  .hcol4{width:10%;background:#BEE395;text-align:center;}
  .ccol1{width:60%;}
  .ccol2{width:40%;}
  </style>

  </head>
  '''

def html_generate_top(page_fob, num,aus,branch,now):
  '''Generate leading code to start off divs, table of contents, and labels at top'''
  print >>page_fob, '''
  <body onload="checkCookie();" >
  <div id="IE">
  <div id="outside">

  <div id="banner">
  <h2>%s Cherrypick Log</h2>
  <h5><em>Last updated on %s at %s</em></h5>
  </div>
  <br>''' % (branch,now.strftime("%b-%d-%Y"),now.strftime("%H:%M"))

  try:
    fob = open('wiki.static.html', 'rb')
    data = fob.read()
    print >>page_fob, data
  except IOError:
    pass
  print >>page_fob, '''
  <br>

  <table border="0" cellpadding="0" cellspacing="0" width="100%" id="labelcontainer" class="fixed">
  <tbody>
    <tr>
     <td colspan="4">
      <table width="100%" border="0" cellpadding="10" cellspacing="0"  id="labels" class="fixed">
       <tr><td class="col1">Project path#</td><td class="col2">Commit</td><td class="col3">Description</td><td class="col4">CR#</td></tr>
      </table>
     </td>
    </tr>
  </tbody></table>
'''

  print >>page_fob, '''<br><div id="expandcollapseallcontainer">
  <input id="expandall" type="button" value="Expand All" onclick="expand_all();">
  <input id="collapseall" type="button" value="Collapse All" onclick="collapse_all();">
  </div>
  ''' # % (branch,now.strftime("%b-%d-%Y"),now.strftime("%H:%M"))

def html_generate_AUheader(page_fob, auThis,auThisDate,AUstatus,i):
  '''Generates leading code for each table'''
  print >>page_fob, '''
  <table border="0" cellpadding="1" cellspacing="0" width="100%%" class="fixed">
  <tbody>
  <tr onclick="toggle(\'tbl%s\',\'lnk%s\');">
     <th class="hcol1">%s</th>
     <th class="hcol2">%s</th>
     <th class="hcol3">%s</th>
     <th class="hcol4"><input id="lnk%s" type="button" value="[+]" class="expandcollapsebutton" ></th>
  </tr><tr><td colspan="4">
  <table width="100%%" border="0" cellpadding="10" cellspacing="0" id="tbl%s" class="innertable">
  ''' %(str(i+1),str(i+1),auThis, auThisDate, AUstatus,str(i+1),str(i+1))

def html_generate_bottom(page_fob):
  '''Closes html tags and adds a thin border'''
  print >>page_fob, '''
  <table border="0" cellpadding="1" cellspacing="0" width="100%" class="fixed">
    <tbody><tr>
      <td bgcolor="#BEE395"></td><td bgcolor="#BEE395"></td><td bgcolor="#BEE395"></td><td bgcolor="#BEE395"></td><td bgcolor="#BEE395"></td><td bgcolor="#BEE395"></td><td bgcolor="#BEE395"></td><td bgcolor="#BEE395"></td>
    </tr></tbody></table>
  </div></div></body></html>
  '''

############################################
#            Main Method                   #
############################################
page_fob = open('wiki.html.new','w')

current_dir = os.getcwd()

argc = len(sys.argv)
if argc < 2 or argc > 3:
    usage()
if sys.argv[1] == "--debug":
    DEBUG=True
    branch = sys.argv[2]
    AU_RE=sys.argv[3]
else:
    DEBUG=False
    branch=sys.argv[1]
    AU_RE=sys.argv[2]
DEBUG=True
now = datetime.datetime.now()

os.chdir("android")

cmd=shlex.split("repo forall -c 'echo -n \"$REPO_PATH $REPO_PROJECT \" && git config --get-regexp remote.*review'")
if DEBUG: sys.stderr.write("Calling repo list %s\n" % (cmd))
cmdPipe=Popen(cmd,stdout=PIPE)
(output,error)=cmdPipe.communicate()

proj_map={}
path_map={}
proj_list=[]
lines = output.split("\n")
for line in lines:
  if line == "":
    continue
  words=line.split(' ')
  if DEBUG: sys.stderr.write("line=%s, words=%s\n" % (line, words))
  # The output from the repo command above is
  # <path> <project> remote.quic[-secure].review <review-server>
  proj_map[words[0]]=(words[1],words[3])
  path_map[words[1]]=words[0]
  proj_list.append(words[0])

if DEBUG: sys.stderr.write("proj_map=%s\n" % proj_map)

if AU_RE=="ICS_STRAWBERRY":
   AU="AU_LINUX_ANDROID_"+AU_RE+".04.00.04.19"
if AU_RE=="JB":
   AU="AU_LINUX_ANDROID_"+AU_RE+".04.01.01.00"
if AU_RE=="JB_REL":
   AU="AU_LINUX_ANDROID_"+AU_RE+".04.01.01.02"

AU_HTML="AU_LINUX_ANDROID_"+AU_RE

cmd=["git","tag","-l",AU+".???" ]
root_dir=os.getcwd()
os.chdir("vendor/qcom/proprietary/kernel-tests")

cmdPipe=Popen(cmd,stdout=PIPE)
(output,error)=cmdPipe.communicate()

if error != None:
    quit()
if DEBUG: sys.stderr.write(output)
aus = output.split()
auList=[]
for au in aus:
    cmd=["git","show",au]
    cmdPipe=Popen(cmd,stdout=PIPE,stderr=PIPE)
    (output,error)=cmdPipe.communicate()
    date = output.split('\n')[2]
    if date.startswith('Date:'):
	auList.append((au,date[8:]))

os.chdir(root_dir)

aus = auList
if DEBUG: sys.stderr.write(str(aus))
#aus = [i for i in output if i.find(branch) != -1]
jot = time.time()
n = len(aus)

aus = aus + [('HEAD',time.ctime(jot))]
html_generate_head(page_fob, n)

cache = Cache()
cache.read()
totCount = 0
html_generate_top(page_fob,n,aus,branch,now)
for i in range(n):
    auLast, auLastDate=aus[n-i-1]
    auThis, auThisDate=aus[n-i]

    AUstatus = " "

    html_generate_AUheader(page_fob, auThis,auThisDate,AUstatus,i)

    aupair = "%s..%s" % (auLast, auThis)
    if DEBUG: sys.stderr.write("Writing AU %s\n" % (auThis))

    changes=[]
    for p in proj_list:
      if (p=="ndk" or p=="sdk"):
	continue
      repo = git.Repo("%s/.git" % (p))
      if p=="kernel":
	p="msm"
      try:
        proj_changes = repo.commits_between(auLast, auThis)
      except:
        if auThis == "HEAD":
           continue
        auThisCommit=""
        auLastCommit=""
	current_wd= os.getcwd()
	print "working directory"
	print current_wd
        fil="../caf-manifest/caf_"+auThis+".xml"
        searchfile = open(fil, "r")
        for line in searchfile:
          if "/"+p+"\"" in line:
	    print "line is %s, project is %s \n" %(line, p)
            line=line.lstrip(" <project ").rstrip()
            line=line.rstrip("/>")
            result={}
            for pair in line.split(' '):
       	        (key,value) = pair.split('=')
                result[key] = value
            auThisCommit=result['revision'].lstrip("\"").rstrip("\"")
            print "This commit %s, file %s project %s \n" %(auThisCommit, fil, p)
        searchfile.close()

        fil="../caf-manifest/caf_"+auLast+".xml"
        searchfile = open(fil, "r")
        for line in searchfile:
           if "/"+p+"\"" in line:
	     print "line is %s, project is %s \n" %(line, p)
             line=line.lstrip(" <project ").rstrip()
             line=line.rstrip("/>")
             result={}
             print "Last commit line is %s project %s \n" %(line, p)
             for pair in line.split(' '):
       	        (key,value) = pair.split('=')
                result[key] = value
             auLastCommit=result['revision'].lstrip("\"").rstrip("\"")
             print "Last commit %s, file %s project is %s\n" %(auLastCommit, fil, p)
             if auThisCommit != auLastCommit:
	        proj_changes = repo.commits_between(auLastCommit+".", auThisCommit)

      if p=="msm":
         p="kernel/msm"
      if DEBUG: sys.stderr.write("found %s change in project %s\n" % (proj_changes, p))
      changes.append((p,proj_changes))

    count = generateAUtable(page_fob, changes, cache, auThis, None)

    if DEBUG: sys.stderr.write('Change count = %s\n' % (count))
    totCount += count

html_generate_bottom(page_fob)

cache.store()

# So we're all do, rename the new file to the main one which is better
# than updating the file in place (which means the contents is less
# useful while the update is happening)a
os.chdir(current_dir)
os.rename('wiki.html.new', 'wiki.html')

duration = time.time() - jot
sys.stderr.write('Total change count = %s in %s secs (%s changes/sec)\n' % (totCount, duration, totCount/duration ))
