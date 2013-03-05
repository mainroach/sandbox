/*Copyright 2013 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
#limitations under the License.*/

/*
    Needle.js is a javascript inline-profiling library which aims to add the least amount of memory and runtime overhead possible. Right now we have a ~0.0001ms - 0.0004ms overhead per event; 
    For example, 10,000 begin/end blocks adds about ~2.6ms - 7ms overhead to your frame.
    See github.com/mainroach for more.
*/


var needle={
    mBatchIndex : 0,    //what's our current batch
    mCurrBatch : null,
    mArraySize : 2048,  //the size of our bucket of samples
    mArrayIndex : 0,    //counter in this bucket

    mBatches:[],

    mIsHighPrecision:false,

    init:function(preAllocSamples, isHighPrecision)
    {
        this.mIsHighPrecision = isHighPrecision;
        this.mBatchIndex = 0;
        this.mArrayIndex = 0;

        var numBatches = preAllocSamples / this.mArraySize;

        for(var i =0; i < numBatches; i++)
            this.addBatch();

        this.mCurrBatch = this.mBatches[0];
    },

   
    // Add a batch, mostly internal only...
    addBatch:function()
    {
        var btc = {};
        btc.mSamType = new Uint8Array(this.mArraySize);
        btc.mSamName = new Array();
        btc.mSamTime = new Float64Array(this.mArraySize);

        
        //pre-alloc string array
        btc.mSamName.length = this.mArraySize;


        this.mCurrBatch = btc;
        this.mBatches.push(btc);
    },


    // Called at the start of a scope, returns < 1ms precision
    _beginFine:function(name)
    {
        var btch = this.mCurrBatch;

        btch.mSamType[this.mArrayIndex] = 1;
        btch.mSamName[this.mArrayIndex] = name;
        btch.mSamTime[this.mArrayIndex] = window.performance.now();

        this.mArrayIndex++;
        if(this.mArrayIndex  >= this.mArraySize)
        {
            if(this.mBatchIndex >= this.mBatches.length-1)
            {
                this.addBatch();
                this.mBatchIndex = this.mBatches.length-1;
            }
            else
            {
                this.mBatchIndex++;
                this.mCurrBatch = this.mBatches[this.mBatchIndex];
            }
            this.mArrayIndex = 0;
        }
    },

    // Called at the end of a scope, returns < 1ms precision
    _endFine:function()
    {
        var btch = this.mCurrBatch;

        btch.mSamType[this.mArrayIndex] = 2;
        btch.mSamTime[this.mArrayIndex] = window.performance.now();

        this.mArrayIndex++;
        if(this.mArrayIndex  >= this.mArraySize)
        {
            if(this.mBatchIndex >= this.mBatches.length-1)
            {
                this.addBatch();
                this.mBatchIndex = this.mBatches.length-1;
            }
            else
            {
                this.mBatchIndex++;
                this.mCurrBatch = this.mBatches[this.mBatchIndex];
            }
            this.mArrayIndex = 0;
        }
    },

     // Called at the start of a scope, returns 1ms precision
    _beginCoarse:function(name)
    {
        var btch = this.mCurrBatch;

        btch.mSamType[this.mArrayIndex] = 1;
        btch.mSamName[this.mArrayIndex] = name;
        btch.mSamTime[this.mArrayIndex] = Date.now();

        this.mArrayIndex++;
        if(this.mArrayIndex  >= this.mArraySize)
        {
            if(this.mBatchIndex >= this.mBatches.length-1)
            {
                this.addBatch();
                this.mBatchIndex = this.mBatches.length-1;
            }
            else
            {
                this.mBatchIndex++;
                this.mCurrBatch = this.mBatches[this.mBatchIndex];
            }
            this.mArrayIndex = 0;
        }
    },

    // Called at the end of a scope, returns 1ms precision.
    _endCoarse:function()
    {
        var btch = this.mCurrBatch;

        btch.mSamType[this.mArrayIndex] = 2;
        btch.mSamTime[this.mArrayIndex] = Date.now();

        this.mArrayIndex++;
        if(this.mArrayIndex  >= this.mArraySize)
        {
            if(this.mBatchIndex >= this.mBatches.length-1)
            {
                this.addBatch();
                this.mBatchIndex = this.mBatches.length-1;
            }
            else
            {
                this.mBatchIndex++;
                this.mCurrBatch = this.mBatches[this.mBatchIndex];
            }
            this.mArrayIndex = 0;
        }
    },

     // public accessors - we use vtable swaps rather than enabled/disabled flags
    begin : function(name){},
    end : function(){},
  
    // needle is disabled by default, call this function to 
    enable:function()
    {
        if(!this.mIsHighPrecision)
        {
            this.begin = this._beginCoarse;
            this.end = this._endCoarse;
        }
        else
        {
            this.begin = this._beginFine;
            this.end = this._endFine;
        }
    },

    // once you've added needle sampling code all over your codebase, you can null it's influence out via calling needle.makeBlunt
    // this will stub out the begin/end functions so that you don't incur overhead
    disable:function()
    {
        this.begin = function(name){};
        this.end = function(){};
    },


};

// Call this at the end of sampling to get a list of all samples in a usable form
// don't expect this to be fast; Only call at the end of profiling.
needle.getExportReadyData =function()
{
    var oneArray = new Array();
    for (var q =0; q < this.mBatchIndex; q++)
    {
        var bkt = this.mBatches[q];
        for(var i =0; i < this.mArraySize; i++)
        {
            if(bkt.mSamType[i] == 0)
                continue;
             var evt = {
                type:bkt.mSamType[i],
                name:bkt.mSamName[i],
                time:bkt.mSamTime[i]
            };
            oneArray.push(evt);
        }
    }

    var bkt = this.mBatches[this.mBatchIndex];
        for(var i =0; i < this.mArrayIndex; i++)
        {
            if(bkt.mSamType[i] == 0)
                continue;
             var evt = {
                type:bkt.mSamType[i],
                name:bkt.mSamName[i],
                time:bkt.mSamTime[i]
            };
            oneArray.push(evt);
        }
        
    return oneArray;
};

// Right now simply dumps linear results out to console; should do something smarter with outputing a about:tracing layout.
needle.consolePrint = function(samples)
{
    var stack = new Array();
    for (var q =0; q < samples.length; q++)
    {
        var evt = samples[q];
        if(evt.type == 1)
        {
            stack.push(evt);
        }
        else if(evt.type == 2)
        {
            var lastEvt = stack.pop();
            var delta = (evt.time - lastEvt.time ) ;

            console.log(lastEvt.name + ": " + delta + "ms");
        }   
    }
};


needle.tracingPrint = function(samples)
{
    var traceString = "[";
    var traceEventGen = function(name,time,isStart){
        var evt = {name:name,pid:42,tid:"0xBEEF",ts:time,ph:"B"};
        if(!isStart)
            evt.ph = "E"
        return JSON.stringify(evt);
    };

    var stack = new Array();
    for (var q =0; q < samples.length; q++)
    {
        var evt = samples[q];
        if(evt.type == 1)
        {
            stack.push(evt.name);
            traceString += traceEventGen(evt.name,evt.time,true) + ",\n";
        }
        else if(evt.type == 2)
        {
            var nm = stack.pop();
            traceString += traceEventGen(nm,evt.time,false) + ",\n";
        }   
    }

    traceString += "{}]";
    return traceString;
}













