# RemoteDebugView
A DLL that serves OutputDebugString content over a TCP connection

# Usage
You will need to compile the DLL and then call the exported function (Default: DebugView). You can invoke the function using rundll

```
rundll32 RemoteDebugView.dll,DebugView
```

This will start a TCP server bound to localhost on a configured port (Default: 3232). Here is a sample program to write to Debug buffer:

<img width="394" alt="Screen Shot 2021-09-23 at 2 19 05 PM" src="https://user-images.githubusercontent.com/61955543/134562933-dfd4f931-c30a-41ec-bd27-9641dd217861.png">


Then, use a tool such as netcat to read the debug output. Included is a rudimentary python tool that reads output from a socket.


<img width="763" alt="Screen Shot 2021-09-23 at 2 23 13 PM" src="https://user-images.githubusercontent.com/61955543/134562733-ceed0078-57f7-45a8-8b24-64fa26b2eec4.png">


# Why
There's been a few times where a tool I wrote is crashing in target space, and I don't have a great way to capture debug output. Starting up DebugView on the target machine is not an option, unless RDP is an option. RemoteDebugView will capture any strings passed to the OutputDebugString(W) functions and host it on a TCP port, so that an operator can use a wide variety of tools to extract helpful debug messages from the target.

Additionally, many programs output *very* interesting information to the Debug buffer, and may assist operators in finding local vulnerabilities or information leakage.


... and then sometimes, you tested and tested, only to have your tool crash in target space. You *could* try to replicate the enivironment locally and trigger the issue, but why not just do it live?


![bill-o-reilly-we-will-do-it-live](https://user-images.githubusercontent.com/61955543/134563891-bfb5bc5f-b39a-4a6c-a26f-4612678d1535.gif)

I find myself doing this more often than I care to admit.
