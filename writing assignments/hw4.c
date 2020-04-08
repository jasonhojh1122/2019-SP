// fork first then vfork
pid_t double_fork(){
    int pfd[2], childPid;
    pipe(pfd);
    childPid = fork();
    if (childPid == 0){
        close(pfd[0]);
        int grandPid;
        grandPid = vfork();
        if (grandPid == 0){
            // grand child
            _exit(0);
        }
        else {
            char idString[16];
            int status;
            idString = intToString(grandPid);
            write(pfd[1], idString, strlen(idString));
            waitpid(grandPid, &status, 0);
            close(pfd[1]);
            return 0;
        }
    }
    else {
        close(pfd[1]);
        char idString[16];
        int grandPid, status;
        read(pfd[0], idString, 16);
        grandPid = stringToInt(idString);
        waitpid(childPid, &status, 0);
        close(pfd[0]);
        return grandPid;
    }
}


// strintToInt is a function that transfer a string to integer
pid_t double_fork(){
    int pfd[2];
    pid_t childPID;
    childPID = vfork();
    if (childPID == 0){
        pid_t grandChildPID:
        grandChildPID = fork();
        if (grandChildPID == 0){
            // grand child
            close(pfd[0]);
            close(pfd[1]);
        }
        else {
            char buf[16];
            sprintf(buf, "%d", grandChildPID);
            write(pfd[1], buf, strlen(buf));
            close(pfd[1]);
            return 0;
        }
    }
    else {
        char buf[16];
        int status;
        pid_t grandChildPID;
        waitpid(childPID, &status, 0);
        read(pfd[0], buf, 16);
        grandChildPID = stringToInt(buf);
        close(pfd[0]);
        return grandChildPID;
    }
}