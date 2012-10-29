
import urllib
import xml.dom.minidom as minidom
import subprocess
import os.path
import json
import datetime
import urllib2
import sys
from urlparse import urljoin
import zipfile
import os.path, time
from os import listdir
from os.path import isfile, join

client_buildNum = 1
client_region = 'usa'
localDataPath = "./data/"

#this function will update our local data to match the server version

#grab server version 
url="http://localhost:8080/"
fileList = url + "filelist.json"
f = urllib.urlopen(fileList)
server_manifest_data = f.read()
server_manifest = json.loads(server_manifest_data)
	
#sort our builds by number
builds = server_manifest['builds']
builds.sort(key=lambda bld: bld['buildnum'])

targets=[]
for build in builds:
	if build['buildnum'] > client_buildNum:
		targets.append(build);

if len(targets) == 0: sys.exit()
		
cdnPath = url + "patches/"
if not os.path.exists(localDataPath): os.makedirs(localDataPath)
for build in targets:
	#files to add
	for file in build["add-files"]:
		fileurl = cdnPath + file
		localurl = localDataPath + file
		print "fetching:" + fileurl + " to " + localurl
		
		#write this file data to disk
		#NOTE in python, if this file already exists, then the following will over-write it
		f = urllib.urlopen(fileurl)
		fl = open(localurl,'wb')
		fl.write(f.read());
		fl.close();
	#files to remove
	#files to patch
	#update our build number
	client_buildNum = build['buildnum']

#this function will initalize our filesystem by loading archives
#it does things the slow, boring way just to show off the concepts

gArchives=[]

for f in listdir(localDataPath):
	fname = join(localDataPath,f)
	
	if not isfile(fname): continue
	obj = {"fname":fname,"time":time.ctime(os.path.getmtime(fname)) }
	gArchives.append(obj)
		
#sort our archives based upon time
gArchives.sort(key=lambda app: app["time"],reverse=True )
	
		
	
#FETCH SOME DATA
fname = "arch0/kodim01.png"
for a in gArchives:
	zf = zipfile.ZipFile(a['fname'], 'r')
	try:
		info = zf.getinfo(fname)
	except KeyError:
		print fname + " not found in " + a['fname']
		continue;
	else:
		print '%s is %d bytes in %s' % (info.filename, info.file_size, a['fname'])
		break;
					
	
	

		
	
