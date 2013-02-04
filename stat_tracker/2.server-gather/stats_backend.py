from __future__ import with_statement #for blobstore
from google.appengine.api import files #for blobstore
from google.appengine.ext import blobstore #for blobstore
from google.appengine.api import backends
from google.appengine.api import users

import copy
import logging
import pprint as pp
import random
import sys
import json

def tojson(python_object):
  """Helper function to output and optionally pretty print JSON."""
  return json.JSONEncoder().encode(python_object)
  
def e(msg):
  """Convient method to raise an exception."""
  raise Exception(repr(msg))


def w(msg):
  """Log a warning message."""
  logging.warning('##### %s' % repr(msg))


class StatsGather:

	buffer0=""
	curBufferIdx=0
	curBuffer=None
	runningByteLen=0
	bufferSize = 1024*5
	def __init__(self):
		self.buffer0 = ""
		self.curBuffer = self.buffer0
		self.runningByteLen = 0


	def addGameEvent(self,eventType, eventName, eventMeta, dob):
		v = {
			"eventType":str(eventType),
			"eventName":str(eventName),
			"eventMeta":str(eventMeta),
			"dob":str(dob)
			}
		enc = tojson(v)
		dlen = len(enc)
		if(dlen +  self.runningByteLen >= self.bufferSize):
			self.forceFlush()
			
		self.runningByteLen = self.runningByteLen + len(enc)
		if(self.runningByteLen == 0):
			self.buffer0  = str(v)	
		else:
			self.buffer0  = self.buffer0 + "," + str(v)
			
	#	logging.info("buffer-" + self.buffer0)
	
	def forceFlush(self):
		
		self.flushBuffer(self.buffer0 + " ")
		self.buffer0 = ""
		self.runningByteLen = 0;
			
	def flushBuffer(self,batchData):
		#logging.info("buffer-" + batchData)
	
		if(len(batchData)==0):
			return
		#flush the string to the DB
		file_name = files.blobstore.create(mime_type='application/octet-stream')
		with files.open(file_name, 'a') as f:
			f.write(batchData)
		files.finalize(file_name)# Finalize the file. Do this before attempting to read it.
		blob_key = files.blobstore.get_blob_key(file_name)
		
		logging.info("BK:FLUSHED")
		
		return blob_key
	