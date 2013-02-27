Needle.js
=========

needle.js is a minimalistic javascript inline-profiling library which aims to add the least amount of memory and runtime overhead possible. Right now we have a _~0.0004ms_ overhead per event;  (e.g 10,000 begin/end blocks adds about ~7ms overhead to your frame.)

Why needle.js?
-----------

The HTML5 implimentation for console.begin/endTime add too much overhead per sample (~0.02ms), and are not usable for fine-grain sampling. Game developers often need to check the performance differences between two inner-loop algorithms, and need more samples / frame.

Why is needle.js so fast?
-----------

needle.js focuses on being fast-first, not being polyfilled or functional. It's built to give you timings in your code.
1) Reduces memory-churn by using a dynamic array of bucketed samples; allowing you to pre-allocate, and also add samples as you run.
2) Takes advantage of the fact that static strings should be hashed in JS already. So doing being("foo") will just pass a reference around. That being said being("foo" + str(entity.ID)) will add a NEW string to the JS heap, EACH call; So try to stay using static strings.
3) Note this library uses window.performance.now(); which is a high-frequency timer; but does add overhead (88% right now) to the overall sampling process.

USAGE
--------------

 Call _needle.init_ with the number of up-front samples you want to allocate for:
 ```needle.init(10000);
 ```
 
Then you need to add begin/end scopes around blocks of code you are interested in timing:
```needle.begin("start of scope");
    ....do some stuff
    needle.end()
```
    
Also fine to nest needle scopes:
```needle.begin("start of scope");
        needle.begin("MORE scope");
        ....do some stuff
        needle.end()
    needle.end()
```

Calling _needle.init_ again will clean the needle, so you can re-use it immediatly.