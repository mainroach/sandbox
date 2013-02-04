
import os;
from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext.webapp.util import run_wsgi_app
from google.appengine.ext import ndb
from google.appengine.ext import db
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
class GameEvent(ndb.Model):
	eventType = ndb.StringProperty(indexed=False)
	eventName = ndb.StringProperty(indexed=False)
	eventMeta = ndb.StringProperty(indexed=False)
	createDate = ndb.DateTimeProperty(indexed=False)
	
	def tojson(self):
		v = {
			'eventType':str(self.eventType),
			'eventName':str(self.eventName),
			'eventMeta':str(self.eventMeta),
			'dob':str(self.createDate)
			}
		return tojson(v)
 
#-----------------------------------------------------	
def addGameEvent(eventType, eventName, eventMeta):
  usr = GameEvent()
  usr.eventType = eventType
  usr.eventName = eventName
  usr.eventMeta = eventMeta
  usr.put()
  return usr
  
#-----------------------------------------------------
def getAllEvents():
	zones = ndb.gql("SELECT * FROM GameEvent")
	return zones
	

#-----------------------------------------------------		
class StatsHandler(webapp.RequestHandler):
	def post(self,fcn):
		if(fcn == "add"):
			type = self.request.get('type',"unknown")
			name = self.request.get('name',"unknown")
			#logging.info( "====BODY: " +self.request.body)
			#params = fromjson(self.request.body) #CLM this has a problem. the incoming data isn't formatted nicely due to it's header being x-www-form-urlencoded
			ip = self.request.remote_addr
			metadata = self.request.body
			addGameEvent(type,name,metadata)
	def get(self,fcn):
		if(fcn == "events"):
			for evt in getAllEvents():
				self.response.out.write(evt.tojson())
		
		
			

application = webapp.WSGIApplication(
									[('/', RootHandler),
									('/s/(.*)', StatsHandler)],

									debug=True)

def main():
	run_wsgi_app(application)

if __name__ == '__main__':
	main()
