o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
	Video_Streaming Application
		By Trevor Hodde
o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

Prerequisites:
--------------
1. The Streaming Video Client and Server were written under the Ubuntu Linux System (Version 12.04).

2. The Compiler version is GCC 4.7.2.

3. Using GTK
   Before compiling and running the Video Client, you must install GTK+ on the Unix system.
   If you do not know how to install GTK+, please see http://www.gtk.org/
   If you are using an Ubuntu system (Which this program was built on) type "sudo apt-get install gtk2-engines-pixbuf" to install the necessary libraries.
   If anything else is needed, Ubuntu will complain to you.

Running the Program:
--------------------
1. To compile the project, just run the following steps:
	cd server
	make
	cd ../client
	make 

	You can also choose to "make clean" in case of any unexpected errors or changes.

2. To run the video server, you must provide a valid port number.
	Example: ./VideoServer 5000

3. To run the video client, you must provide the following parameters:
	IP Address or hostname of the server
	Valid port number
	Existing Video File on the server

	Example: ./VideoClient 10.244.174.79 5000 tmp.mp4
