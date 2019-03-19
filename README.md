# Basic PoW Blockchain implemented in C

**Meant for educational purposes only**

Here is a PoW blockchain implemented in C. It's intended to show the implementation of a very simple PoW blockchain in straight C (with OpenSSL libraries for SHA256.)

To build it, simply: `gcc -lcrypto -lssl blockchain.c`

### How it works

Using a linked-list like structure, we build a chain of blocks. There are three steps to this:

1. Initialise the block with some data and its parent.
2. Generate a random nonce for the proof of work.
3. Verify the proof of work.
4. Push the block onto the linked list.

### Blockchain is actually very simple

Although undoubtfully an ingenious idea, the implementation of a blockchain is actually quite simple. You are simply verifying blocks against previous blocks using crypto magic. Of course, the structure and functions in chains such as Bitcoin are much more complicated than this one, but I want to just show people a taster of where to start.

### Proof of work function

The proof of work function basically checks if the first N bits of a hash are equal to 0, where N is the difficulty. Every 50 blocks, it will then average out the timestamps and see if it needs adjusting. We want to keep it inbetween 1-2 seconds for this little test - so if it's taking longer than 2 seconds per block, we decrease the difficulty. If shorter than 1 second, then we increase it. All of these constants can be edited using the macros at the top.

##### Additional disclaimer

I'm not a C programmer by heart, so I apologise to the C crowd if this code is kind of sucky ;).

**Enjoy !**