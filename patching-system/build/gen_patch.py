
import subprocess
import os.path
from os import listdir
from os.path import isfile, isdir, join
from filecmp import dircmp
import json
import sys
import shutil
'''
This will analyze two directories and report the list of files that are different.
'''
#========================================================
#in reality these should be passed in from your build tool

dir0_buildNum = 0
dir0 = "./" + str(dir0_buildNum)

dir1_buildNum = 1
dir1 = "./" + str(dir1_buildNum)
#========================================================
#some helper functions
def give_files(mypath):
	fs = []
	for f in listdir(mypath):
		if (isdir(join(mypath,f))):
			ks = give_files(join(mypath,f))
			fs.extend(ks)
		if isfile(join(mypath,f)) :
			fs.append(join(mypath,f))
	return fs
	
def safe_copy_file(src,dst):
	dstfldr = dst.replace(os.path.basename(dst),"")
	if not os.path.exists(dstfldr): os.makedirs(dstfldr)
	shutil.copyfile(src,dst)
#========================================================
class Build(object):
	def __init__(self):
		self.buildnum = 0
		self.add_files=[]
		self.del_files=[]
		self.patch_files=[]
	buildnum=0
	add_files=[]
	del_files=[]
	patch_files=[]

latestBuild = Build()
#==============================================================
#http://docs.python.org/library/filecmp.html
def print_diff_files(dcmp):
	for name in dcmp.left_only:
		fname = dcmp.left +"/" + name
		if isdir(fname):
			latestBuild.del_files.extend(give_files(fname))
		else:
			latestBuild.del_files.append(dcmp.left +"/" + name)
	for name in dcmp.right_only:
		fname = dcmp.right +"/" + name
		if isdir(fname):
			latestBuild.add_files.extend(give_files(fname))
		else:
			latestBuild.add_files.append(dcmp.right +"/" + name)
	for name in dcmp.diff_files:
		latestBuild.add_files.append(dcmp.right +"/" + name)
	for sub_dcmp in dcmp.subdirs.values():
		print_diff_files(sub_dcmp)

dcmp = dircmp(dir0, dir1)
print_diff_files(dcmp)


#====================================================================
#extract the add/diff files and compress them in a zip file
print 'creating zip of differences'

import zipfile
zipFName = str(dir0_buildNum) + "_" + str(dir1_buildNum) + ".patch.zip"
zf = zipfile.ZipFile(zipFName, mode='w')
try:
	for file in latestBuild.add_files:
		
		zf.write(file,arcname=file.replace(dir1,""))
		print file
finally:
	zf.close()
	print "zip created"

#create a leaf build name
latestBuild.add_files[:] = []
latestBuild.add_files.append(zipFName)
#copy the file to the build server
safe_copy_file("./" + zipFName,"../server/patches/" + zipFName)
os.remove(zipFName)

#=========================================================================
#open our manifest file, add this to it.

#Helper function to output and optionally pretty print JSON.
_JSON_ENCODER = json.JSONEncoder()
_JSON_ENCODER.indent = 4
_JSON_ENCODER.sort_keys = True
def tojson(python_object):  
  return _JSON_ENCODER.encode(python_object)

#open the existing file list
mani_file = "../server/filelist.json"

build_mani={}
try:
	f = open(mani_file,'r')
	build_mani = json.loads(f.read())
	f.close()
except:
	build_mani={}
	build_mani['builds'] = []

vbd={}
#do we already have this build in the list?
for build in build_mani['builds']:
	if build['buildnum'] == dir1_buildNum:
		vbd = build
		
#if this is a new build, append it
if len(vbd.keys()) == 0: 
	build_mani['builds'].append(vbd);
	
vbd['buildnum'] = dir1_buildNum
vbd['add-files'] = latestBuild.add_files



f = open(mani_file,"w")
f.write(json.dumps(build_mani))
f.close()

