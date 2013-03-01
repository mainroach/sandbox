Needle.js
=========

Needle.js is a minimalistic javascript inline-profiling library which aims to add the least amount of memory and runtime overhead possible. Right now we have a _~0.0001ms - ~0.0004ms_ overhead (tested on windows) per event;  (e.g 10,000 begin/end blocks adds about ~2.6ms - 7ms overhead to your frame.)

## Why Needle.js?


The HTML5 implimentation for console.begin/endTime add too much overhead per sample (~0.02ms), and are not usable for fine-grain sampling. Game developers often need to check the performance differences between two inner-loop algorithms, and need more samples / frame.

## Why is Needle.js fast?


Needle.js focuses on being fast-first, not being polyfilled or functional. It's built to give you timings in your code.

1. Reduces memory-churn by using a dynamic array of bucketed samples; allowing you to pre-allocate, and also add samples as you run.

2. Takes advantage of the fact that static strings should be hashed in JS already. So doing being("foo") will just pass a reference around. That being said being("foo" + str(entity.ID)) will add a NEW string to the JS heap, EACH call; So try to stay using static strings.

3. Note this library uses window.performance.now(); which is a high-frequency timer; but does add overhead (88% right now) to the overall sampling process.



## USAGE

To Enable/Disable Needle, call the `needle.enable` and `needle.disable()` functions. 
NOTE Needle is DISABLED by default.

Call `Needle.init` with the number of up-front samples you want to allocate for:

` Needle.init(10000);`


 
Then you need to add begin/end scopes around blocks of code you are interested in timing:

` Needle.beginCoarse("start of scope");

     //....do some stuff

 Needle.endCoarse()`

    
Also fine to nest Needle scopes:

` Needle.beginCoarse("start of scope");

    Needle.beginCoarse("MORE scope");

    //....do some stuff

    Needle.endCoarse()

 Needle.endCoarse()`


To clean/reset Needle simply call:

` needle.init(..) `

again to clean and resue it immediatly.


## Choosing the right API

Note that Needle provides two sets of APIs beginCoarse/endCoarse and beginFine/endFine.

The *Coarse APIs will return timings a the 1ms resolution.

The *Fine APIs will return timings at < 1ms resolution, but will run almost 3x slower as a result.


The intent is that a developer will place the *Coarse versions of calls around scoping blocks that are fine with 1ms resolution, and the *Fine calls in places that need more details.


## DISCLAIMER

*I've only tested this in CHROME 26 beta; I make no warrenties that it works in other browsers / versions

