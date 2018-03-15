/* CSci4061 F2016 Assignment 2
* login: cselabs login name (login used to submit)
* date: 10/05/16
* name: Abiola Adimi, Khoa Le, David Schommer
* id: adimi001@umn.edu, lexxx611@umn.edu, schom120@umn.edu */

1. Who did what.

	Abiola Adimi: polling child processes; handling CREATE_TAB, NEW_URI_ENTERED and TAB_KILLED messages; error checking

	Khoa Le: modified uri_entered_cb, create_new_tab_cb and url_rendering_rendering_process, Documenting README.md

	David Schomer: poll children processes, create new tab when CREATE_TAB message is sent, handling TAB_KILLED message and waiting, url_rendering_process message handlers (NEW_URI_ENTERED, TAB_KILLED), create CONTROLLER process, README

2. How to compile the program: run `make` on the provided Makefile.

3. How to use the program from the shell (syntax)

	a. First, run the program
	
		./browser
	
	b. To create new tab, click the create-new-tab button in the CONTROLLER window.
	
	c. Enters the tab number (e.g 1, or 2) where the requested URL is to be rendered in the Tab-selector region.
	
	d. Enters the URL (e.g., http://www.google.com) in the URL-region of the CONTROLLER window. Note that "http://" must be included in the URL-region.
	
	e. Hits the keyboard Enter key in the URL-region.
	
	f. If the user want to open a new tab with a different webpage, repeat the steps from b to e. 
		Note that the tab number must match with where the user want the requested URL to be rendered 
		(e.g, tab 1 will be rendered if the user type 1 in the Tab-selector region)
	
	
	g. To close a tab, click on Tab-close button of the specific tab.
	
	h. To close the CONTROLLER window, click on the Tab-close button of this window. This will close any opening tabs and exit the router process.
	
	
4. Explicit assumptions: 
- The maximum number of tabs is 99
- The maximum length of a URL is 512 characters 

