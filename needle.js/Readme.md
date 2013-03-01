Needle.js
=========

Needle.js is a minimalistic javascript inline-profiling library which aims to add the least amount of memory and runtime overhead possible. Right now we have a _~0.0001ms - ~0.0004ms_ overhead (tested on windows) per event;  (e.g 10,000 begin/end blocks adds about ~2.6ms - 7ms overhead to your frame.)

## Why Needle.js?


The HTML5 implimentation for console.begin/endTime add too much overhead per sample (~0.02ms), and are not usable for fine-grain sampling. Game developers often need to check the performance differences between two inner-loop algorithms, and need more samples / frame.

## Why is Needle.js fast?


Needle.js focuses on being fast-first, not being polyfilled or functional. It's built to give you timings in your code.

1. Reduces memory-churn by using a dynamic array of bucketed samples; allowing you to pre-allocate, and also add samples as you run.

2. Takes advantage of the fact that static strings should be hashed in JS already. So doing begin("foo") will just pass a reference around. That being said begin("foo" + str(entity.ID)) will add a NEW string to the JS heap, EACH call; So try to stay using static strings.

3. Utilizes typed arrays and direct memory assignment rather than new-object creation.



## USAGE

To Enable/Disable Needle, call the `needle.enable()` and `needle.disable()` functions. 
NOTE Needle is DISABLED by default.

Call `Needle.init` with the number of up-front samples you want to allocate for:

`Needle.init(10000,false);`

Note that 2nd parameter is a boolean value representing if needle should run in high-precision mode or not.

* The Low precision mode will return timings a the 1ms resolution.

* The High precision mode will return timings at < 1ms resolution, but will run almost 3x slower as a result.



 
Then you need to add begin/end scopes around blocks of code you are interested in timing:

`
    Needle.begin("start of scope");

        //....do some stuff

    Needle.end()
`

    
Also fine to nest Needle scopes:

`
    Needle.begin("start of scope");

        Needle.begin("MORE scope");

            //....do some stuff

        Needle.end()

    Needle.end()
`


To clean/reset Needle simply call:

`needle.init(..) `

again to clean and resue it immediatly.



## DISCLAIMER

*I've only tested this in CHROME 26 beta; I make no warranties that it works in other browsers / versions

