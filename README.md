# slowloris
A powerful, low-bandwidth DoS/Stress Testing tool for web servers.
Written in C

_______________

# How Slowloris Works
A slowloris DoS attack initiates a seemingly valid HTTP/HTTPS connection with a web server and hangs the connection and keeps it open as it sends dummy headers regularly to keep the connection alive and denies any future clients from connecting due to the current amount of clients already connected to the web server via the DoS attack.

________________

# Usage
_Compile_

    $ gcc slowloris.c -o slowloris
    
_Run_

            $ ./slowloris -host <HOST>  -port <PORT NUMBER>  -clients <NUMBER OF CLIENTS>
    Example $ ./slowloris -host example.com  -port 80  -clients 350
    
