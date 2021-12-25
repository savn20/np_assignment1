# TCP Server Client Assignment

## Line based TCP Client

### Todos
- [x] Setup connection to reference server `13.53.76.30:5000`  
- [x] Send `OK\n` to server
- [x] Perform operation requested by server
- [x] Send Computed Result to the server

### Notes to my self
- Commit history is important. So save your progress while working
- For floating point use `%8.8g` for formatting
- Documentation is as important as acutal development.
- Log Debugging only when the `DEBUG` flag is activated

## Line based TCP Server

### Todos
- [x] create a server at given `ip` and `port`
- client management
    - [x] handle one client at a time
    - [x] 5 clients should be able to queue for service, the sixth should be rejected
    - [x] when client gets connected, respond with "TEXT TCP 1.0n"
- task management
    - [ ] generate random assignment to client using calcLib
    - [ ] if the client takes longer than 5s, terminate the client and close the connection
    - [ ] compare the result from client and server and respond OK if correct, ERROR if wrong