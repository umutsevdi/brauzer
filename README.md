# brauzer
A graphical web browser from scratch in C programming language that implements various protocols


- [x] TCP Request
- [x] SSL System
- [x] User Agent
- [ ] Protocol Analysis
- [ ] Protocol Specific Calls (HTTP 1.1 Polling)


```c
FOR HTTP 1.1 
void Start();
void GetFile();
void GetHeader();
#if HTTP
    ParseLinks();
    PollLinks();
#endif
void Close();





```
