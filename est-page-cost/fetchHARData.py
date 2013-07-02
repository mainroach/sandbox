import urllib2
import socket
import xml.dom.minidom as minidom
import subprocess
import os.path
import json
import logging
import csv
import sys
import datetime
import math
import os, os.path
from subprocess import call

'''usage : fetchHARData.py amazon.har amazonDataFolder'''
#======================================
def safeDirCheck(path):
    if not os.path.exists(path): os.makedirs(path)    
#======================================
def genHAR(url,outFile):
    os.system("phantomjs.exe netsniff.js " + url + " > " + outFile)
#======================================
def fetchURLData(url):
    try:
        f = urllib2.urlopen(url, timeout = 60)
        respData = f.read()
        return respData
    except urllib2.URLError, e:
        print("There was an error: " + str(e))
    except Exception, e:
        print("There was an error: " + str(e))
    return []

#======================================
def cacheURLData(url,outFile,isBinary):

    respData=fetchURLData(url)
    if len(respData) == 0: return
    
    #cache to disk directly
    safeDirCheck(outFile.replace(os.path.basename(outFile),""))
    fl = None
    if(isBinary):
        fl = open(outFile,'wb')
    else:
        fl = open(outFile,'w')
    fl.write(respData);
    fl.close();
#======================================
def giveFilenameFromURL(url):
    return os.path.basename(url)
#======================================
def giveExthard(pth):
    pthlen = len(pth)
    idx = pth.rfind('.')
    if(idx < pthlen-5):
        return ""
    return pth[idx+1:pthlen]


mimeLUT=[
#label,mime array, extention array, subfolder, isBinary
("image", ["image/"],["png","jpg","jpeg","gif","webp","ico"],"/img/",True),
("video", ["video/"],["mpg","mpeg","mov","avi","webm","flv","f4v"],"/video/",True),
("text", ["css","text","json","html","xml","javascript"],["htm","html","json","js","xml","txt","css"],"/text/",False),
("misc", ["octet"],["bin"],"/misc/",True),
("plugins", ["shockwave","flash"],["swf"],"/plugin/",True),
("audio", ["ogg"],["ogg","mp3"],"/audio/",True),
]

#======================================
def guessContentType(mimetype,name):

    subPath=""
    ext = ""
    isBinary = False
    
    if mimetype == None:
        print "No Mimetype for " + name
        return ("","",False)

    tExt = giveExthard(name)
    #if(not "?" in name):
    #search for an extention match first    
        
    if(len(tExt) > 0):
        tExt = tExt.lower()
        for typ in mimeLUT:
            if(tExt in typ[2]):
                return (typ[3], "." + tExt,typ[4])

    #didn't find extention match, try the mime
    for typ in mimeLUT:
        for mim in typ[1]:
            if(mim in mimetype):

                #see if we can't find an extention match
                for exttyp in typ[2]:
                    if(exttyp in mimetype):
                        ext = exttyp
                        break
                if ext == "":
                    ext = typ[2][0]
                return (typ[3], "." +ext,typ[4])

 
    print "UNKNOWN object:\'" + mimetype + "\' for " + name
    

    return ("","",False)



#======================================
def grabAssets(harFile,outDir):
    harData=None
    #open and read har file
    with open(harFile,'r') as f: 
        #read the file here
        dat = f.read()
        f.close()
        try:
            harData = json.loads(dat)
        except Exception:
            print "Couldn't parse HAR file:" + harFile
            return

    dynamicNameCounter=0


    urlDict=[]

    #walk through the assets, doing the work
    entsArray = harData["log"]["entries"]
    for e in entsArray:
        req = e["request"]
        resp = e["response"]
        resp_cont = resp["content"]
        mimetype = resp_cont["mimeType"]

        contentURL = req["url"]

        if(contentURL in urlDict):
            continue

        urlDict.append(contentURL)

        mimePair = guessContentType(mimetype,contentURL)

        
        subPath=mimePair[0]
        ext=mimePair[1]
        isBinary = mimePair[2]

        if(len(subPath)==0):
            continue
        #determine the filename
        fname = ""

        tExt = os.path.splitext(contentURL)[1]
        if("?" in contentURL or len(tExt)==0):
            fname = "dyn_" + str(dynamicNameCounter)
            dynamicNameCounter+=1
        else:
            fname = giveFilenameFromURL(contentURL)
            if(len(fname) ==0 or os.path.splitext(fname)[1] ):
                fname = "dyn_" + str(dynamicNameCounter)
                dynamicNameCounter+=1


        sys.stdout.write(".")
        cacheURLData(contentURL,outDir + subPath + fname + ext,isBinary)
        


        

#======================================
def fetch(harFilePath,outDir):


    print( "\n=================" + harFilePath + "=================")

    safeDirCheck(outDir)
   

    #os.chdir(os.path.realpath(__file__).replace("build.py",""))

    #first, copy the har file into the data folder

    grabAssets(harFilePath,outDir)

if __name__ == '__main__':
    harFilePath = sys.argv[1]
    outDir = sys.argv[2]
    doForce = False
    if(len(sys.argv) >4):
        if sys.argv[3] == "f":
            doForce = True

 


    fetch(harFilePath,outDir)