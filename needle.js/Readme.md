Needle.js
=========

Needle.js is a minimalistic javascript inline-profiling library which aims to add the least amount of memory and runtime overhead possible. Right now we have a _~0.0001ms - ~0.0004ms_ overhead (tested on windows) per event;  (e.g 10,000 begin/end blocks adds about ~2.6ms - 7ms overhead to your frame.)

Why Needle.js?
-----------

The HTML5 implimentation for console.begin/endTime add too much overhead per sample (~0.02ms), and are not usable for fine-grain sampling. Game developers often need to check the performance differences between two inner-loop algorithms, and need more samples / frame.

Why is Needle.js so fast?
-----------

Needle.js focuses on being fast-first, not being polyfilled or functional. It's built to give you timings in your code.
1. Reduces memory-churn by using a dynamic array of bucketed samples; allowing you to pre-allocate, and also add samples as you run.
2. Takes advantage of the fact that static strings should be hashed in JS already. So doing being("foo") will just pass a reference around. That being said being("foo" + str(entity.ID)) will add a NEW string to the JS heap, EACH call; So try to stay using static strings.
3. Note this library uses window.performance.now(); which is a high-frequency timer; but does add overhead (88% right now) to the overall sampling process.

USAGE
--------------

 Call _Needle.init_ with the number of up-front samples you want to allocate for:
 > _Needle.init(10000);_
 
Then you need to add begin/end scopes around blocks of code you are interested in timing:
> _Needle.beginCoarse("start of scope");_
>     _//....do some stuff_
> _Needle.endCoarse()_

    
Also fine to nest Needle scopes:
> _Needle.beginCoarse("start of scope");_
>    _Needle.beginCoarse("MORE scope");_
>    _//....do some stuff_
>    _Needle.endCoarse()_
> _Needle.endCoarse()_

To clean/reset Needle simply call:
> needle.init(..)
again to clean and resute it immediatly.

You can optionally turn off needle by calling
> needle.makeBlunt();
Which will replace the begin/end functions with stub operations; This is highly useful for shipping production builds (w/o wanting to strip all the needle references from the codebase)

Choosing the right API
--------------
Note that Needle provides two sets of APIs beginCoarse/endCoarse and beginFine/endFine.
The *Coarse APIs will return timings a the 1ms resolution.
The *Fine APIs will return timings at < 1ms resolution, but will run almost 3x slower as a result.

The intent is that a developer will place the *Coarse versions of calls around scoping blocks that are fine with 1ms resolution, and the *Fine calls in places that need more details.


DISCLAIMER
--------------
*I've only tested this in CHROME 26 beta; I make no warrenties that it works in other browsers / versions

