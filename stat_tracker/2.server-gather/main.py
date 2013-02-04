from __future__ import with_statement #for blobstore
from google.appengine.api import files #for blobstore
from google.appengine.ext import blobstore #for blobstore

from google.appengine.api import backends #needed for backends
import stats_backend	#needed for backends
from google.appengine.api import urlfetch #needed for backends

import os;
import webapp2 #needed for backends
import cgi
import datetime
import urllib
import json
import logging
import wsgiref.handlers
from types import *

_BACKEND_NAME='statgather'
  #needed for backends
if backends.get_backend() == _BACKEND_NAME:
	_stats = stats_backend.StatsGather()
	
def tojson(python_object):
  """Helper function to output and optionally pretty print JSON."""
  return json.JSONEncoder().encode(python_object)
  
def fromjson(msg):
  """Helper function to ingest JSON."""
  try:
    return json.loads(msg)
  except Exception, e:
    raise Exception('Unable to parse as JSON: %s' % msg)
	
	
#-----------------------------------------------------		
class StatsHandler(webapp2.RequestHandler):
	def post(self,fcn):

		if(fcn == "add"):
			type = self.request.get('type',"unknown")
			name = self.request.get('name',"unknown")
			ip = self.request.remote_addr
			metadata = self.request.body
			backendURL = backends.get_url(backend=_BACKEND_NAME, instance=None, protocol='HTTP')
			url = '%s/k/add?type=%s&name=%s' % (backendURL, type,name)
			
			payload = metadata
			
			resp = urlfetch.fetch(url=url,
                          payload=payload,
                          method=urlfetch.POST,
						  deadline=60 #CLM is this OK practice?
                          #headers={'Content-Type': 'application/json'}
						  )
			self.response.set_status(resp.status_code)
			self.response.out.write(resp.content)
			logging.info('%s -> %s -> %s' % (repr(payload), url, resp.content))
	
	def get(self,fcn):

		if(fcn == "blobs"):
		#	self.postBackendFlush()
			gqlQuery = blobstore.BlobInfo.gql("ORDER BY creation DESC")
			blobs = gqlQuery.fetch(10)
			
			outStr = "["
			for blob in blobs:
				blob_reader = blobstore.BlobReader(blob.key())
				value = blob_reader.read()
				#logging.info("VVVVVVV" + value)
				outStr = outStr + value
			outStr = outStr + "]"
			
			#pass back to the client for display
			self.response.headers['Content-Type'] = 'application/json'
			self.response.out.write(outStr)
		elif(fcn == "flush"):
			self.postBackendFlush()
		else:
			self.response.out.write("{}")
				
	def postBackendFlush(self):
		backendURL = backends.get_url(backend=_BACKEND_NAME, instance=None, protocol='HTTP')
		url = '%s/k/flush' % (backendURL)
			
		payload = "{}"
		#rpc = urlfetch.create_rpc()
		#urlfetch.make_fetch_call
		resp = urlfetch.fetch(url=url,
                          payload=payload,
                          method=urlfetch.POST,
						  deadline=60
                          #headers={'Content-Type': 'application/json'}
						  )
		

#-----------------------------------------------------		

	
 #the backend handler
class HybridStatsHandler(webapp2.RequestHandler):
	def post(self,fcn):
		if(fcn == "add"):
			type = self.request.get('type',"unknown")
			name = self.request.get('name',"unknown")
			#logging.info( "====BODY: " +self.request.body)
			#params = fromjson(self.request.body) #CLM this has a problem. the incoming data isn't formatted nicely due to it's header being x-www-form-urlencoded
			ip = self.request.remote_addr
			metadata = self.request.body
			_stats.addGameEvent(type,name,metadata,"now")
			self.response.out.write("success")
		elif(fcn == "flush"):
			_stats.forceFlush()
			self.response.out.write("success")
			
	def get(self,fcn):
		self.response.out.write("ugg")
		
		
	
# handler common to frontends and backends
handlers = [
]

if not backends.get_backend():
  # frontend specific handlers
  handlers.extend([
		('/s/(.*)', StatsHandler)
  ])
elif backends.get_backend() == _BACKEND_NAME:
  # 'stats' backend specific handlers
  handlers.extend([
		('/k/(.*)', HybridStatsHandler)
  ])

app = webapp2.WSGIApplication(handlers, debug=True)

