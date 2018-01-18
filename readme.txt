-Information:
  +ID: 1412468
  +Name: Trịnh Công Sơn
  +Email: congson0710@gmail.com

-List of functions:
	+User press a shortcut key (LCtrl + LShift + Space), a dialog will appear. 
	Two controls are there: a textbox for typing and a listbox for listing available apps.
	+ Everytime user types a key, a list of available apps will appear on top of listbox.
	+ User can select using arrow keys from the list.
	+ User press enter or double click and the selected app will start.
	+ Do statistics and chart on apps usage frequency.


-Main flow: User launches the app. An icon  will be added to notification area. 
	+ User can right click on it to see menu: Scan Apps, View statistics, Exit.
		- Exit menu, of course, will exit the app.
		- Scan to build database: will scan all exe files in Program Files folder and build aa 				dictionary. 
		- View statistics: Display chart based on usage frequency of apps.
	+User presses the shortcut key (LCtrl + LShift + Space), a dialog will appear or press again to add 		to notification area.
	+Everytime user  types a key, a list of available apps will appear. App is suggested based on 			priority of recent use.
	+User can select using arrow keys from the list.
	+User press enter or double click and the selected app will start.
	+After a while of using our app, user can see the statistics chart based on his usage frequency.
	+After close the app, name of used-app will be saved to file and load to the app for next usage.

-Addition flow: +App only search on System32 and Program File(x86) folder.
	+Apps that recent use will be pushed on top of listbox.
	+Statistic shows last 5 apps that have the most usage.


