# ============================================================================
#
#    Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
#    Qualcomm Technologies Proprietary and Confidential.
#
# ============================================================================

import cPickle
import os
import sys

class Change:
    def __init__(self, brnchCmt, brnchURL, commitText, mainCmt, mainURL, subject, commitCRs, branch, project, path):
        self.brnchCmt = brnchCmt
        self.brnchURL = brnchURL
	self.commitText = commitText
        self.mainCmt = mainCmt
        self.mainURL = mainURL
        self.subject = subject
        self.commitCRs = commitCRs
        self.branch = branch
        self.project = project
	self.path = path

    def getBrnchCmt(self):
        return self.brnchCmt
    def getBrnchURL(self):
        return self.brnchURL
    def getMainCmt(self):
        return self.mainCmt
    def getMainURL(self):
        return self.mainURL
    def getCommitCRs(self):
        return self.commitCRs
    def getSubject(self):
        return self.subject
    def getProject(self):
        return self.project
    def getBranch(self):
        return self.branch
    def getCommitText(self):
	return self.commitText
    def getPath(self):
	return self.path
    def __str__(self):
        return 'Change<%s->%s, %s->%s, %s(%s), %s, %s, %s>\n%s' % \
			(self.brnchCmt, self.brnchURL,
                                               self.mainCmt, self.mainURL,
                                               self.project, self.path, self.subject,
                                               self.commitCRs, self.branch,
					       self.commitText)

class Cache:
    FILENAME='wiki.cache'
    FILENAME_TMP='wiki.cache.tmp'

    def __init__(self):
        self.changes = {}
        self.aus = {}
	self.aus_order = []

    def add(self, change, au):
        if au == "HEAD":
            return
#        sys.stderr.write('%s added to cache %s\n' % (str(change), au))
        commit = change.getBrnchCmt()
#	sys.stderr.write('%s commit\n' % (commit))
        if commit != None:
            self.changes[commit] = change
            if self.aus.has_key(au):
#		sys.stderr.write('Added to dictionary/list')
                self.aus[au].append(change)
            else:
#		sys.stderr.write('Started new dictionary/list')
                self.aus[au] = [change]
		self.aus_order.append(au)

    def getCommit(self, commit):
        try:
            return self.changes[commit]
        except KeyError:
            return None

    def getAUcommits(self, au):
        try:
            return self.aus[au]
        except KeyError:
            return []

    def isAUPresent(self, au):
	try:
	    x = self.aus[au]
	    return True
	except KeyError:
	    return False

    def getAUs(self, start, end):
	found = False
	for i in self.aus_order:
	    if i == start:
		found = True
	    if found:
		yield i
	    if i == end:
		break

    def store(self):
        fob = open(Cache.FILENAME_TMP, 'wb')
	data = (self.changes, self.aus, self.aus_order)
        cPickle.dump(data, fob)
        fob.close()
	os.rename(Cache.FILENAME_TMP, Cache.FILENAME)

    def read(self):
        try:
            fob = open(Cache.FILENAME, 'rb')
            self.changes, self.aus, self.aus_order = cPickle.load(fob)
            fob.close()
        except:
            self.changes = {}

