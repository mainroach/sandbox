
var currentTab = null;
function loadScripts()
{
   chrome.tabs.executeScript(currentTab.id, {file: "src/js/needle.js"});
  chrome.tabs.executeScript(currentTab.id, {file: "src/js/needle_injector.js"});
  
}
function inject()
{
  
  chrome.tabs.executeScript(currentTab.id, {code: "needle.enable();needle.init(10000);console.log(\"needle ready\");needleInject.inject();"});
  
}


function eject()
{
  chrome.tabs.executeScript(currentTab.id, {code: "needleInject.eject();"});
}


function enable()
{
  chrome.tabs.executeScript(currentTab.id, {code: "needleInject.enable();"});
}


function disable()
{
  chrome.tabs.executeScript(currentTab.id, {code: "needleInject.disable();"});
}


function ndl_export()
{

chrome.tabs.executeScript(currentTab.id, {code: "var expData = needle.getExportReadyData();var str = needle.tracingPrint(expData);chrome.extension.sendMessage({needle_results: str}, function(response) {});"});


}


document.getElementById("loadscripts").addEventListener("click", loadScripts);
document.getElementById("inject").addEventListener("click", inject);
document.getElementById("eject").addEventListener("click", eject);
document.getElementById("enable").addEventListener("click", enable);
document.getElementById("disable").addEventListener("click", disable);
document.getElementById("export").addEventListener("click", ndl_export);


chrome.extension.onMessage.addListener(
  function(request, sender, sendResponse) {
    console.log(sender.tab ?
                "from a content script:" + sender.tab.url :
                "from the extension");
    if (request.needle_results != null)
    {
        document.getElementById("code").innerHTML = request.needle_results;
    }
  });

chrome.tabs.getSelected(null, function(tab) {
    console.log(tab.url);

    currentTab = tab;
   
    
  });

