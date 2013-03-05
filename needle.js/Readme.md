Needle.js
=========

Needle.js is a minimalistic javascript inline-profiling library which aims to add the least amount of memory and runtime overhead possible. Right now we have a _~0.0001ms - ~0.0004ms_ overhead (tested on windows) per event;  (e.g 10,000 begin/end blocks adds about ~2.6ms - 7ms overhead to your frame.)

## Why Needle.js?


The HTML5 implimentation for console.begin/endTime add too much overhead per sample (~0.02ms), and are not usable for fine-grain sampling. Game developers often need to check the performance differences between two inner-loop algorithms, and need more samples / frame.

## Why is Needle.js fast?


Needle.js focuses on being fast-first, not being polyfilled or functional. It's built to give you timings in your code.

1. Reduces memory-churn by using a dynamic array of bucketed samples; allowing you to pre-allocate, and also add samples as you run.

2. Takes advantage of the fact that static strings should be hashed in JS already. So doing being("foo") will just pass a reference around. That being said being("foo" + str(entity.ID)) will add a NEW string to the JS heap, EACH call; So try to stay using static strings.

3. Utilizes typed arrays and direct memory assignment rather than new-object creation



## USAGE

* Include Needle.js (minimize it if you need to)
* Start placing begin/end blocks in strategic locations in your code. 
  * Place some begin/end statements in some of the root functions of your code.
    * Things like input callbacks, RequestAnimationFrame callbacks, etc
  * Scoping every function is a bad idea (because of the overhead) and also creates noisy output. 
  * Note that if a begin/end blocks’ execution time is < 0.001ms, you should remove the sampling from that area; you’re adding more overhead to the simulation to measure than the execution is worth.
* Call needle.init(numPreAllocSamples,isHighPrecision)
  * numPreAllocSamples is an interesting balancing number. You want to set it just high enough that you can collect samples safely while reducing allocations, but not too high that it starts to put pressure on the Garbage Collector to start firing often. I find that with most of the tests I run, somewhere between 10k and 30k is a strong value.
  * isHighPrecision should always be set to FALSE at first. Don’t use high-precision unless you need it.
  * If you need to, you can start fiddling with the bucket size. I’m sure there’s some nice edge-case tradeoffs with tweaking that value...
* During runtime, call needle.enable() when the time is right
  * Unless you want to start recording all events from the dawn of your page, you’ll generally want some trigger event before you start tracing
* Call needle.disable() when you’re done collecting
  * Calling disable is important; It will keep Needle.js from collecting more samples while you’re doing processing or writing data out.
  * NOTE that you can enable / disable Needle.js as you see fit. For instance, if you only want needle to collect samples during input callbacks.
* Call needle.getExportReadyData
  * This will return to you a linear array of all the samples in a friendly format so you can output / process them
* Choose some method to output the data, and call one of the needle.*print* functions to get your data
  * Most of the printing functions output a string, which you can add to a ghost div, or write to the console etc
  * Needle.js comes with a printing option that will output to a format that Chrome’s about://tracing will read and display; Which is really nice and handy as a visualization tool!
* When doing your release builds, add logic to auto-remove needle.js function calls from the code.
  * It’s worth pointing out that Needle.js in DISABLED mode only adds 0.00004ms overhead per call. Which is really, really tiny for web-apps. For games however, that much overhead can cause some problems over time.


## DISCLAIMER

*I've only tested this in CHROME 26 beta; I make no warrenties that it works in other browsers / versions

