1. Installation and Usage
    Use make to compile all the required code
    $ make

    Use bidding_system to start a bidding system
    $ ./bidding_system [host num] [player num]
    [host num]   should be greater then 0 and no greater then 10
    [player num] should be greater then 7 and no greater then 14

    To delete all the compiled files and unused *.FIFO files
    $ make clean

2. Description
    briefly description of what is done in each .c file
    bidding_system
        (1) mkfifo for Host.FIFO and Host[id].FIFO.
        (2) exec givien number of hosts.
        (3) loop through combinations and assign task to unused host.
        (4) receive all the message in Host.FIFO
        (5) calculate rank and print the result
        (6) send "-1"s message to root hosts send "-1" message to root host
        (7) close *.FIFO and remove *.FIFO

    host
        (1) receive player ids from parent
        (2) fork two children to recursivly solve the problem
        (3) receive two winners from children, find out the true winner and send winner id to children
        (4) loop
        (5) after 10 rounds, (receive "-1" message from parent) or (receive new tasks from hosts)
        (6) if received "-1" message, send "-1" message to children and wait
        (7) close open file descriptors and _exit

    player
        (1) send id id*100 to leaf child
        (2) receive winner id if it's not the first round.
        (3) loop
        (4) after 10 rounds, _exit

    Special Algorithm
        For random keys generation I used Knuth algorithm to generate given number unique keys.
        Reference
            https://stackoverflow.com/questions/1608181/unique-random-numbers-in-an-integer-array-in-the-c-programming-language
            https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle

