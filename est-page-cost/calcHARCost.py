import urllib
import xml.dom.minidom as minidom
import subprocess
import os.path
import json
import logging
import csv
import sys
import datetime
import math
import fnmatch
import operator
import os, os.path
from subprocess import call


'''usage : fetchHARData.py amazon.har'''
#======================================
def giveFilesWithExtention(mypath,ext):
    matches = []
    for root, dirnames, filenames in os.walk(mypath):
        for filename in fnmatch.filter(filenames, '*.' + ext):
            matches.append(os.path.join(root, filename))
    return matches
#======================================
def safeDirCheck(path):
    if not os.path.exists(path): os.makedirs(path)    
#======================================
def genHAR(url,outFile):
    os.system("phantomjs.exe netsniff.js " + url + " > " + outFile)
#======================================
def cacheURLData(url,outFile,isBinary):

    respData=[]
    try:
        f = urllib.urlopen(url)
        respData = f.read()
    except Exception:
        #print "Couldn't load " + url + "\n"
        return
    
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

#===============================================
mimeLUT=[
#label,mime array, extention array, subfolder, isBinary
("text", ["css","text","json","html","xml","javascript"],["htm","html","json","js","xml","txt","css"],"text",False),
("image", ["image/"],["png","jpg","jpeg","gif","webp","ico"],"img",True),
("video", ["video/"],["mpg","mpeg","mov","avi","webm","flv","f4v"],"video",True),
("misc", ["octet"],["bin"],"misc",True),
("plugins", ["x-shockwave-flash"],["swf"],"plugin",True),
("audio", ["ogg"],["ogg","mp3"],"audio",True),
]


#======================================
def guessContentType(mimetype,name):

    subPath=""
    ext = ""
    isBinary = False
    
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

 
    #print "UNKNOWN object:\'" + mimetype + "\' for " + name
    

    return ("","",False)



#======================================
def estCosts(byteCountDict,estNumUsersDay,estCostPerGig):

    miniDict = {}
    for k in byteCountDict:
        
        
        oneLoad = byteCountDict[k]# / 1024.0 / 1024.0
        if(oneLoad == 0): 
            miniDict[k]="0"
            miniDict[k+"-cost"] =0
            continue

        #compute the cost based on yearly patterns
        dailyLoad = estNumUsersDay * oneLoad
        gigCount = dailyLoad / (1024.0*1024.0*1024.0)
        gigCost = estCostPerGig * gigCount
        yearCost = gigCost * 365

        miniDict[k] =str(oneLoad)#/(1024.0*1024.0))
        miniDict[k+"-cost"] =int(yearCost)
        
    return miniDict

        
        

    

#======================================
#parse the HAR file, and for each mimetype, bucketize it and calculate costs.
def grabAssets(harFile):
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


    byteCountDict={}
    for k in range(len(mimeLUT)):
        byteCountDict[mimeLUT[k][3]]=0
        byteCountDict[mimeLUT[k][3] + ".gz"]=0
    urlDict=[]

    #walk through the assets, doing the work
    entsArray = harData["log"]["entries"]
    for e in entsArray:
        req = e["request"]
        resp = e["response"]
        resp_cont = resp["content"]
        mimetype = resp_cont["mimeType"]
        if mimetype == None:
            mimetype = "text"
        contentURL = req["url"]

        if(contentURL in urlDict):
            continue

        size = resp["bodySize"]
        comprSize = 0

        urlDict.append(contentURL)

        mimePair = guessContentType(mimetype,contentURL)

        
        subPath=mimePair[0]
        ext=mimePair[1]
        isBinary = mimePair[2]

        if(len(subPath)==0):
            continue


        #is this gzipped?
        isGZipped = False
        headers = resp["headers"]
        for idx in range(len(headers)):
            vk = headers[idx]
            if(vk["name"] == "Content-Encoding"):
                if vk["value"] == "gzip":
                    isGZipped = True
                    break

        if isGZipped:
            byteCountDict[subPath+".gz"] += size
        else:
            byteCountDict[subPath] += size

    return byteCountDict
    
    
        


#===============================================
if __name__ == '__main__':
    harFile = sys.argv[1]
    costPerGig = sys.argv[2] #eg 0.12 cents
    numDailyUsers = sys.argv[3] #eg 1,000,000
   
    byteCountDict = grabAssets(harFile)

    typeCosts = estCosts(byteCountDict,float(costPerGig),float(numDailyUsers))


    #simplistic print of the data.
    totalBytes = 0
    totalCost = 0
    
    print "Site : " + harFile
    print "Est. # Users : " + str(numDailyUsers)
    print "Est. $ / gig : " + str(costPerGig)
    

    print "Assets: cost per year"
    sortedKeys = sorted(typeCosts.keys())
    
    for k in sortedKeys:
        if 'cost' in k: continue #skip cost values, we query them directly.

        totalBytes += int(typeCosts[k])
        cost = typeCosts[k + '-cost']
        totalCost += int(cost)
        
        print "\t" + str(k) + ": $" + str(cost) + " (" + str(typeCosts[k]) + " bytes)"

    print "Total size : " + str(totalBytes) + " bytes"
    print "Total cost : $"  + str(totalCost) 
        
        