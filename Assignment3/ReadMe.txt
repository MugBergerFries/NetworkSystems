This code successfully acts as a proxy between a client and a web server.
It transforms and relays requests between the agents.
It creates a cache of webpages, though in the current state it does not propely utilize it.
It checks a file 'blacklist.txt' for hostnames and IPs to restrict the client from viewing
Usage: ./webproxy <port> <timeout> //Note: the timeout function for the cache isn't functional, but it expects the input regardless
Compiling: make clean && make
Note: Any files relayed over an HTTPS connection will not be retrieved

Note to grader: the cache does not function due to a lack of foresight on my part.
Forked processes cannot share memory, so the vectors tracking the cache and the logic syncing them are acting on individual copies of the empty cache
The logic should be fully sound, and the solution is to use pthreads instead of forking, but that requires a full reformatting of the project, which I do not have the time for.
I understand if this hurts my grade significantly, but I would request that some leniency be granted, if possible.
I will continue to work on the code when I can and implement the rest of the caching functionality