Motivation
----------
Have you ever tried to implement your own SQL parser?
That's a very complicated and hard task and the first thing that comes
to your mind may be: "Someone must have already done this, it's not smart
to re-invent the wheel". 
Why would you want to do that anyway?
My motivation was an old database system for MS-DOS where I implemented
a driver to be able to access the old database format. It's not hard
to write an ISAM for that (ISAM = Index Sequential Access, a set of 
functions to read and write datasets and scroll through the index file, 
if there is one).
But the real complicated task is to write a SQL parser, an optimizer etc.,
because this would end up with implementing a whole database system. 
Now what options do you have, if you want to access your database with
SQL but not reimplement a DBMS? And the most important part: It shouldn't
cost you license fees.
The first thing that I thought about was the MySQL Database system, which
has an architecture that allows you to implement your own database drivers.
I successfully did that and it's working great! A few years ago, there were
some problems with dynamically linking the storage engine against MySQL on
Windows, but that was all solved in the meantime.
This all works fine, but from the user's perspecive, it involves installing
a local MySQL-Server just because the user may want to access his old 
database from his MS Office application. Additionally, MySQL has its own
table definition format, so you have to keep the table definitions in sync
which isn't so nice regarding maintainability.
So I thought about a possibility to make access easier for the user.
When thinking about it a bit, it came to my mind that there already is a
full SQL-Parser integrated into Microsoft Windows!
Remember the nice "trick" in VBA where you were able to access CSV files
with fully SQL Grammar from your code?
The technology that makes this possible is Microsoft Joint Engine 
Technology (MS JET)!

About MS Jet
------------
I advise you the read the Wikipedia article About MS JET to get familiar with
its concepts:

http://en.wikipedia.org/wiki/Microsoft_Jet_Database_Engine

After reading the article, you now know that there are Jet Red and Jet Blue.
There is a nice graphic showing the architecture of Jet Red:
There is the MS JET engine that has some IISAM (Installable ISAM) DLLs that
provide functionality to read XBase, Paradox, Text files and other databases.
Now the idea is to implement such a IISAM driver for your own database and
profit from the Jet SQL Parser.
The Jet API itself is a OLEDB Provider. OLEDB is a technology that Microsoft
wanted to push in the 90ies to replace ODBC, but fortunately nobody really
liked it and the older ODBC standard nowadays is the de-facto standard for
cross platform database access for SQL-Databases.
You may also want to read the Wikipedia-Articles about OLEDB to familiarize
yourself with its history.
So The Jet OLEDB-API itself now also has a warpper driver that allows ODBC
Access to Jet, but this will be discussed lateron.
Unfortunately there is no documentation by Microsoft on how to write a Jet
IISAM driver. That makes it a very hard task to find out how it works 
internally.
Luckily there is also Jet Blue (you read about it in the Wikipedia article)
and Microsoft decided to make this API public in 2005. Jet Blue is now called
ESENT (Extensible Storage Engine).
ESENT basically is like an implementation of a Jet ISAM that can be accessed
directly via a bunch of functions, but it's not an IISAM module but a 
standalone module.
It's documented in the MSDN (though there are a few documentation errors
and it doesn't seem that a lot of people are really using it):

http://msdn.microsoft.com/en-us/library/windows/desktop/gg269245%28v=exchg.10%29.aspx

(as MSDN links tend to change very often, just serach for 
 "Extensible Storage Engine Reference" just in case the link isn't valid anymore)

You can see that all functions start with the prefix Jet. The structures used 
are available in the esent.h file that gets shipped with the Platform SDK.
Equipped with the knowledge from this header file, I was able to reconstruct the
structure of a Jet IISAM driver by reverse engineering the MS Text 
JET driver (mstext40.dll). I tried to rewrite most of the code like it's most
likely within the original to make understanding the mstext-driver easier for
you, if you also want to take a closer look into it.

Structure of a IISAM .dll
=========================

I found out that a IISAM driver basically is a .dll file that registers 
like a COM InprocServer via DllRegisterServer function and exports a 
IsamLoad-funtion as ordinal 1 that gets called by MS Jet to load the DLL.
The IsamLoad-Function basically received a backlink to the MS Jet driver
to call various functions within Jet which is needed for proper operation.
It returns back a pointer to a function table that exposes its own 
ISAM-functions which get called by Jet. Fortunately these functions 
resemble those that are exported by ESENT, therefore they are well documented.
A IISAM driver internally uses UNICODE functions and WCHARs, but as it also
has to run on Win95, it has some compatibility wrapper functions that ensure
that there aren't any problems on Win95.
Now let's go a bit deeper into the details of my sourcecode to learn about
the implementation:

esent98.h
---------
Most public available and documented Jet structures and constants are in 
the esent.h available in Platform SDK. If you are still using the old Platform
SDK from 98 that ships with Visual Studio 6.0, you need to grab the esent.h
from a newer Platform SDK (i.e. Windows Server 2003 PSDK is still comptible
with Visual Studio 6.0 and freely available from MS).
But not all constants that get used in a IISAM driver are available there.
Therefore I collected some missing definitions from various sources and formed
esent98.h which contain the missing definitions that are not shipped with
esent.h, fixes for possible bugs in the original header and the internal
structures which get used by the IISAM drivers that I painfully reconstructed.
Therefore you always have to include my esent98.h header file instead of
esent.h
Please look through the comments in the file to find out more.

iisam.c
=======
Registering the IISAM module with the system:
---------------------------------------------
As mentioned, the DLL uses the DllRegisterServer entrypoint to install itself.
So from user perspective, this is pretty straight forward, the user just has
to call

   regsvr32 <iisam.dll>

and it gets installed.
Now how does the information about an IISAM gets populated to the Jet driver?
It's put in the registry under

  HKEY_LOCAL_MACHINE\Software\Microsoft\Jet\4.0

There are 2 subkeys related to the driver: 
  Engines
  ISAM Formats

In "Engines", the driver itself gets registred. There is a subkey for every
storage engine which contains information where the engine file is located
and various driver specific options that the IISAM writer can define.
The only value that has to be there is the value "win32" that contains
path and filename of the IISAM DLL.
To define your application specific settings, declare them in spec.c
and spec.h

"ISAM Formats" contains all the formats that are supported by a IISAM.
i.e. the Text-driver supports Text and HTML.
The Format is linked to the driver (=to the Engine) via the Engine-Key,
which contains the name of the Engine.
There are various other options that define the driver which should be
filled. I documented them in the structure IISAM_CFG_DEF in config.h, 
so have a look at the header file. 
You may notice that i.e. FilterString is UINT. Why is that?
It's because this is a resource Identifier instead of a fixed string.

After the registry keys are filled with this information, the driver
is registered.

Unregistering the driver
------------------------
DllUnregisterServer deletes the keys that were created upon registration.
From the user's point of view, this is as simple as:

  regsvr32 /u <iisam.dll>


ErrIsamLoad
-----------
As mentioned above, the function has to be exported as Oridnal 1.
It gets a reference to the Jet driver which exports some functions
(see strut JET_CALLIN in esent98.h) and you return a Vtable of
the function that your driver provides (see IISAM_Vtbl struct
in esent98.h). You have to provide implementations of all these 
functions. I already implemented them for you. When implementing a driver,
you may want to adapt IISAMOpenDatabase function to parse user- 
and databaselevel parameters from the connection string accordingly.
It's not mandatory, you can also just let them load from the registry
with the function in spec.c, but if they are variable parameters that
depend on the database opened, it's better to get them from the 
connection string. For example, the text driver parses the HDR= or
FixedFormat= parameters here.
If your Database is transactional, you may also want to supply 
functions like IISAMCommitTransaction. If you also support creating 
new databases, you may want to implement IISAMCreateDatabase etc.
As always, to get documentation for the apropriate functions, search
MSDN for the Function name, but prefixed with "Jet", i.e. for 
IISAMCreateDatabase search for the help for "JetCreateDatabase" in
MSDN.

spec.c
------
This module contains Registry-Settings of your driver. It utilizes the
functions from config.c which provide access to driver specific registry
settings as it's also implemented in the text driver.
Please have a look at the documentation of the functions in config.h.

trace.c
-------
As you may have noticed, there is a setting for Tracing in spec.c
It's very useful for debugging your driver, because every function
call to your driver or from your driver back to the Jet engine gets
logged either to a file or - in debug builds - to the MS visual C
debug output window using the OutputDebugString() API.
You can use Sysinternals DebugView to view Debug output too.
This module implements the tracing facility. It is set up in IISAMInit
in iisam.c. It works by overriding the Vtable that gets called by
Jet with the tracing functions which in turn call the real functions
of your driver.

winalloc.c
----------
The Jet drivers have their own memory management routines for heap-
management. The original heap manager from the Text ISAM driver has
been implemented in here. This may be due to the fact that Jet
drivers need to use the global heap (GlobalAlloc etc.) in order to
have memory blocks that can be shared with the Jet driver.
But the main reason may be that the driver has the possibiity to do
a proper cleanup of open memory objects on unload and not rely on the
system heap manager to do this, therefore ensuring that no memory leaks
are left.

jetuwrap.c
----------
This is the Jet UNICODE wrapper. As Jet drivers also need to be able
to run on Windows 9x that doesn't support all the wide character UNICODE
versions of the functions that are available in the NT kernel, this
wrapper ensures that the driver also works properly on Windows 95.
Internally the jet driver uses UNICODE, so it only operates on wide 
character versions of the functions.
Therefore you should ensure that you are using these functions i.e. for
Registry access and file system operations!

netutil.c
---------
The Jet drivers have a very neat feature regarding the filenames you
pass to them as a database/table: If you are specifying an Internet
URL, i.e. prefixed with http:// or ftp://, the driver takes care of fetching
the files from the remote locations before it operates on them locally.
After the user finished modifying the files, they will be uploaded to the
remote location again.
Therefore it's possible for your driver to have network support built-in
without a lot of additional programming overhead!
In order to accomplish this, the driver needs some functions to communicate
with the WININET library to load and save files remotely. It's also possible
to list remote files (enumerate them).
These functions are implemented in this module. They are mainly used by 
windos.c:

windos.c
--------
As previously mentioned, operations on local and remote file systems should be
possible transparently to the driver implementer. Therefore some special 
functions are needed for normal file operations so that it gets ensured that
they are either processed locally or remotely.
As also mentioned before, there is the need to wrap UNICODE functions that are
not present in Windows 95 which is done via jetuwrap.
Additionally these functions provide easier access to certain file system 
operations.
Therefore you should use these functions for easy file system access.
However most of the functionalty for file operations is already implemented
in this raw driver sample, so you don't really need to care about it, your
driver already gets provided with a local file to operate on.

isamutil.c
----------
Most structures of the jet driver are implemented as linked lists. This
module manages these structures. It also contains some functions
for time calculation. The definition for the various linked lists can be
found in isamutil.h. You may notice that there are markers for offsets
of the various structure members which fit the offsets in the ms text
driver so that you can explore these structures on your own, if you are
analyzing the text driver. If the members aren't present like this in the
original driver, they are marked as "Custom attribute" to not confuse you.
The structure of the text Jet driver is as follows
(of course you can change this if it doesn't suit the needs of your driver):

ISAM_Task
Every instance of the driver DLL that gets loaded gets assigned a task handle
which holds a reference count, the link back to the Jet driver to call
functions in it and an instance handle assigned by the jet driver.

ISAM_Session
A session belongs to a task. Ever call to IISAMBeginSession starts a new 
session that gets assigned to the current task handle of the DLL. As the
name implies, it contains session data like for instance virtual tables
and a session identifier.

ISAM_Database
A database belongs to a task and consists of database users and naturally
tables. It also contains handles i.e. for network files that need to be
uploaded again when the user finishes its database tasks and a temporary
location where the files that got downloaded are stored for local operation.

ISAM_DBUser
A user of a database belongs to a database and links it with a session.
It for instance contains the user settings for the database and the user
defined connection string.

ISAM_DBTable
A table belongs to a database and holds for instance an array of cursors
that are open on that database and - most important for you - the instance
handle to your own database driver (pDrvInst) that can operate on that
tables (provided that each table is in its own file and is managed seperatly,
as in the text driver, this makes sense).
In the text driver, it also contains a memory list of columns, but this may
not make sense for your own driver, if the driver is managing them on its
own (most likely).

ISAM_Column
This would hold column information in the real text driver. Most likely you
won't use this.

ISAM_Cursor
An open cursor that belongs to a table and a user. It holds all the information
that is needed to operate on a certain table, i.e. the current row number.

ISAM_VTDef
A virtual table definition that belongs to a session. When reading through
the documentation for the variaous Jet functions that you have to implement
in driver.c, you will find some functions that need to return a virtual
table with information that needs to be able to be seeked in a usual way.
Such virtual tables are defined with this structure.

vtfunct.c
---------
This funtion contains all the necessary functions to seek through a virtual
table. A virtual table just gets assigned the seek functions defined here
as Vtable in g_drvvttblvtbl via callback into the Jet driver via 
ErrAllocateTableidForVsesid (see driver.c for implementation details how
this works).

driver.c
--------
This is the most crucial part of the driver as it contains all the 
implementation specific details of the driver.
As you can see in iisam.c, on OpenDatabase, a new Dbid gets allocated
by the Jet driver via ErrAllocateDbid and also gets passed a pointer to 
a JET_FNDEFDB Vtable (g_drvdbvtbl in driver.c) that contains basic database
operations. For instance, closing a database again is done via one of these
callback functions. The functions also include a function to open a Table.
When opening a table, the ErrAllocateTableidForVsesid functions gets called
in the jet driver that gets passed an instance handle (cursor ID in our case)
and a Vtable that in turn contains table specific functions (see m_drvtblvtbl 
table). As previously mentioned, this is also the way how virtual tables are 
allocated and connected with in-memory seek functions in vtfunct.c. 
As with iisam.c, to get documentation for specific functions, change the 
Isam- Prefix with the Jet-Prefix and search the MSDN-Documentation for a
description, i.e. for IsamGetTableColumnInfo, search MSDN for 
JetGetTableColumnInfo. Therefore I won't describe all the functions here, just
use MSDN for it. If you want to implement your own driver, fill the missing
parts in the functions marked with TODO. If you don't support certain 
functions, you can leave them as they are, if they return 
JET_errFeatureNotAvailable. 
It's interesting that the text file driver and others don't use the original
structures that they should return according to the documentation and esent.h,
but instead use shorter versions of them. This may be, because the structures
have been extended lateron, or because they are specifically trimmed for
ESENT. I have recovered the original structures used in the text driver and
put them into esent98.h with the _OLD suffix, so ensure that these are
returned (it's already in the driver skeleton, so just fill them).

Implementing your own Jet IISAM driver
======================================

1) Create a new directory below the project directory and copy the
   following files to it:
   iisam.c, driver.h, driver.c, spec.h, spec.c, iisam.def
   These are the files that need to be customized by you. 
   Then create a workspace that includes all files except the mentioned 
   from the original project directory (..) and the files mentioned
   above that you copied from the current directory.
2) Fill m_stDrvCfg in iisam.c with the parameters of your driver
   for registering. Fill one structure for every format that you
   support.
3) Edit the struct in spec.h that defines your driver specific
   options. These are available in your driver implementation in the
   ISAM_DBUser struct, member pSettings.
   Intialize them with their defaults in spec.c with the appropriate
   ConfigAdd* function dependent on the type of the option.
4) Optionally parse Database- and Userspecific parameters in 
   function IISAMOpenDatabase in iisam.c
5) Implement the missing functions in iisam.c, if you plan to support
   Transactions, creating databases, etc.
   If you just want to read/write to existing databases, you can leave
   them as they are.
6) Change rowid_t in isamutil.h according to your needs for your row 
   identifiers that can uniquely identify a dataset in a table. For me,
   a DWORD that identified the dataset number in the database file was 
   enough, but in the original text driver this was a larger structure.
7) Set driver information about your driver in driver.h, i.e. supported
   file extensions.
8) Complete all function parts in driver.c skeleton which are marked with
   TODO. To get documentation about what is expected there, read the 
   MSDN documentation for the appropriate functions (exchange Isam with
   Jet prefix to find function name for MSDN search).


How to debug your driver 
========================
After you implemented your driver, it will most likely crash the first time,
if you didn't do everything correctly. But if you are using some external 
program, which uses OLEDB for debugging, it may not be so easy to debug since
you cannot just set breakpoints in source code.
Therefore my advice is to use the Rowsetviewer programm from the Microsoft Data
Access SDK 2.8. You can compile it in your Visual Studio and then start it
and after attaching your driver, you can set breakpoints in your source code.
Of course you need to register your debug build DLL as a driver in the system
with regsvr32 <iisam.dll> first so that rowsetviewer can find your driver.
Now how to connect to your driver in Rowsetviewer and test some SQL queries?
go to File/Full connect...

Provider tab
------------
Provider: Mirosoft.Jet.OLEDB.4.0
DataSource: The directory of your database or table files.

Properties
----------
ProvString: Enter the format name of the engine you want to use, as you 
            defined in struct m_stDrvCfg in iisam.c. For instance for our 
            sample sekeltor driver: Dummy 1.0

Then click OK to connect and if it connects successfully, your driver has been 
loaded now and you can set breakpoints, issue SQL commands etc.

As you are running in a debugger, you are able to see debug output of your
driver in your output window thanks to trace.c.

How to use your driver
======================
Now how to use a OLEDB driver in database applications?
There are many applications that can use OLEDB databases, you can also use
them from VB-Script. I'll show 2 ways how to use it from MS Excel for instance:

1) Excel - External data source
Go to Data/Import External Data/Import Data...
Then select "+Connect to New Data Source" in the file open dialog.
In the following Wizard, select "Others.." as data source.
Then select "Microsoft OLE DB Provider for ODBC Drivers".
In tab "All":
 Set "Data Source" to the directory that contains your database or tables.
 Set "Extended Properties" to the requested engine (see ProvString above)

Now your should be connected to your data source and i.e. refresh fetched
data in your Excel spreadsheet.

2) Use VBA to conveiently access your database.
Here is a simple example. Same settings as in 1) apply for the properties:
------------------------------------------------------------------------------
Sub test()
Dim oConScores As ADODB.Connection
Set oConScores = New ADODB.Connection

With oConScores
    .Provider = "Microsoft.Jet.OLEDB.4.0"
    .Properties("Extended Properties").Value = "Dummy 1.0"
    .Open "Data Source = f:\temp\txtdb\"
End With

Dim rsScores As ADODB.Recordset
Set rsScores = New ADODB.Recordset

With rsScores
    .Source = "Select * from testtable"
    .ActiveConnection = oConScores
    .CursorLocation = adUseClient
    .CursorType = adOpenForwardOnly
    .LockType = adLockReadOnly
    .Open
End With

Dim sResults As String

Do Until rsScores.EOF
    sResults = sResults & rsScores.Fields(0).Value & " " & _
	rsScores.Fields(1).Value & Chr(13)
    rsScores.MoveNext
Loop
MsgBox sResults

End Sub
------------------------------------------------------------------------------

