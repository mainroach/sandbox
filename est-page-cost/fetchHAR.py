import urllib
import xml.dom.minidom as minidom
import subprocess
import os.path
import json
import sys
import time
import threading

from BeautifulSoup import BeautifulSoup

#========================================
#We provide this class, which allows us to abort a system operation if its' taking too long.
class RunCmd(threading.Thread):
    def __init__(self, cmd,  timeoutInSeconds,outfile):
        threading.Thread.__init__(self)
        self.cmd = cmd
        self.timeout = timeoutInSeconds
        self.out = outfile
    def run(self):
        outStream = open(self.out, 'w')#this will ignore any stdout output
        self.p = subprocess.Popen(self.cmd,stdout=outStream)
        self.p.wait()

    def Run(self):
        self.start()
        self.join(self.timeout)

        if self.is_alive():
            print "!!!!!Took too long, aborting!!!!!"
            self.p.terminate()
            self.join()

#========================================
def _fetchPhantom(targetURL,outFileName):

    outName = "../" + outFileName

    
    os.chdir("phantomJS")
    args = ""
    if "darwin" in sys.platform: #mac
        args += "phantomjs"
    else:#windows?
        args += "phantomjs.exe"
    
    args += " examples/netsniff.js"
    args += " http://www." + targetURL #TODO do the right thing here, determine if there's # of "." or "www" in the list...
    
    #NOTE that sometimes phantom JS can hang for no reason fetching data
    #so we create this safety runner that allows us to run and abort if it takes more than 2 minutes to fetch
    RunCmd(args, 120,outName).Run()
    os.chdir("../")


#======================================
def _fetchHTTPArchive(targetURL,outFileName):
    targetSiteID = 0

    #NOTE this is based on a specific version of the HTTPArchive site, taken on July1st 2013
    #if the site changes, please notify me and update this script

    #step 1, get the siteID from httpArchive
    url = 'http://httparchive.org/findurl.php?term=' + targetURL
    
    f = urllib.urlopen(url)
    respData = f.read()
    harData = json.loads(respData)
    #if HTTPArchive doesn't have our data, run a phantom instance
    if len(harData) ==0:
        return -1

    targetSiteID = harData[0]["data-pageid"]
    print targetSiteID

    #step 2, get the HAR file
    url = 'http://httparchive.org/viewsite.php?pageid=' + targetSiteID
    f = urllib.urlopen(url)
    html = f.read()
    
    soup = BeautifulSoup(html)
        
    harURL=""
    
    #parse the data blocks
    for lnk in soup.findAll('a'):
        v = lnk['href']
        if v.find("http://httparchive.webpagetest.org/export.php?") != -1:
            harURL = v
            break
    
    print harURL
    f = urllib.urlopen(harURL)
    respData = f.read()
    
    #cache to disk directly
    fl = open(outFileName,'w')
    fl.write(respData);
    fl.close();

    print "COMPLETED"
    return 0

#======================================
def fetchHarFile(inURL,outHarFile):
	#can we grab from HTTP archive first?
	result = _fetchHTTPArchive(inURL,outHarFile)
	if result == 0: return
	_fetchPhantom(inURL,outHarFile)

#======================================
if __name__ == '__main__':
	inURL = sys.argv[1]
	outDir = sys.argv[2]

	fetchHarFile(inURL,outDir)

	