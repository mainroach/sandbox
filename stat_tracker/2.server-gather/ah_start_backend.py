

from google.appengine.api import apiproxy_stub_map
from google.appengine.api import backends
from google.appengine.api import runtime
from google.appengine.api import users

from google.appengine.ext import webapp

from google.appengine.ext.webapp import template

import cgi
import datetime
import logging
import os
import urllib


my_name = '%s.%s (%s)' % (backends.get_backend(), backends.get_instance(), backends.get_url())


logging.info(my_name + ' is registering shutdown hook')


def my_shutdown_hook():
  logging.warning('shutdown hook called')
  apiproxy_stub_map.apiproxy.CancelApiCalls()
  # save_state()
  # May want to raise an exception

# register our shutdown hook, which is not guaranteed to be called
runtime.set_shutdown_hook(my_shutdown_hook)

