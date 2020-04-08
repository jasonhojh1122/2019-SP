1. I/O multiplexing
In the main while loop,
first I loop through the client_socket to find out all the connection,
then I use select and FD_ISSET to find out any incoming io operatons.

2. Read server
HandleReadRequest will handle any read request.
THe fucntion does the follow things:

IF the requested id is valid
    IF the account is not locked
        lock the account
        read out the information of the account
        unlock the account
    ELSE   
        the account is locked
ELSE
    invalid request


3. Write server
HandleWriteRequest will handle any write request.
THe fucntion does the following things:

IF it is the first time the fd connect
    IF the requested id is valid
        IF the account logged in by other user on the same server
            IF account is not locked
                lock the account and wait for response
            ELSE   
                the account is locked
        ELSE
            the account is locked
    ELSE
        invalid request

ELSE
    IF the upcoming requested is valid
        IF the ammount of money is valid
            save/withdraw/transfer/balance
        ELSE
            operation failed
    ELSE
        operation failed

    unlock the account
    logout the account in the same server


reference
https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/
https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
http://man7.org/linux/man-pages/man2/fcntl.2.html