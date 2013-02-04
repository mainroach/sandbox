from __future__ import with_statement #for blobstore
from google.appengine.api import files #for blobstore
from google.appengine.ext import blobstore #for blobstore
import os;
from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext.webapp.util import run_wsgi_app
from google.appengine.ext.db import stats#https://developers.google.com/appengine/docs/python/datastore/stats

import os;
import cgi
import datetime
import urllib
import json
import logging
import wsgiref.handlers
from google.appengine.ext.webapp import util
from google.appengine.ext import db
import wsgiref.handlers
import random
from types import *

def tojson(python_object):
  """Helper function to output and optionally pretty print JSON."""
  return json.JSONEncoder().encode(python_object)
  
def fromjson(msg):
  """Helper function to ingest JSON."""
  try:
    return json.loads(msg)
  except Exception, e:
    raise Exception('Unable to parse as JSON: %s' % msg)
	
class RootHandler(webapp.RequestHandler):
    def get(self):
		path = os.path.join(os.path.dirname(__file__), 'index.html')
		self.response.out.write(template.render(path, {}))
		

#-----------------------------------------------------	


def addGameBatchEvent(batchData):
	# Create the file
	file_name = files.blobstore.create(mime_type='application/octet-stream')

	# Open the file and write to it
	with files.open(file_name, 'a') as f:
		f.write(batchData)
	files.finalize(file_name)# Finalize the file. Do this before attempting to read it.

	# Get the file's blob key
	blob_key = files.blobstore.get_blob_key(file_name)

	return blob_key
  
  
#-----------------------------------------------------		
class StatsHandler(webapp.RequestHandler):
	def post(self,fcn):
		if(fcn == "addbatch"):
			ip = self.request.remote_addr
			metadata = self.request.body
			key = addGameBatchEvent(metadata)
			self.response.out.write(key)
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
		
			

application = webapp.WSGIApplication(
									[('/', RootHandler),
									('/s/(.*)', StatsHandler)],

									debug=True)

def main():
	run_wsgi_app(application)

if __name__ == '__main__':
	main()
