*************************Error Cases*************************

0. Usage

sh cdr_au_integrate.sh  -d /local/mnt/workspace/proj6 -t
/local/mnt/workspace/proj5 -l "cdr-server-name:port"
-p manifest.git -r "cdr-mainline-branch"
-a "AU_LINUX_ANDROID_JB_REL_2.0.3.04.01.01.21.020"
-u "grease-user" -w "grease-passwd"
-b "grease-branch"
-v 2

**************Case1***************

**********Error inside add_fetch_merge_remote()*********

1. Output will contain lines of the form

ERROR: line 137 - command exited with status: 1
Error at function add_fetch_merge_remote() called at line 310

line 137 ( maybe a different one in your run ) is the lineno
inside the function add_fetch_merge_remote() where error
occurred and 310 is the line from where the function
add_fetch_merge_remote() was called.

2. Commands + Error trace

error: Exited sync due to fetch errors
++ [06:56:20] traperror 49 au_sync 287
++ [06:56:20] local errcode=1
++ [06:56:20] local lineno=49
++ [06:56:20] '[' au_sync '!=' '' ']'
++ [06:56:20] local funcstack=au_sync
++ [06:56:20] '[' 287 '!=' '' ']'
++ [06:56:20] local linecallfunc=287
++ [06:56:20] echo 'ERROR: line 49 - command exited with status: 1'
++ [06:56:20] '[' au_sync '!=' '' ']'
++ [06:56:20] echo -n 'Error at function au_sync() '
++ [06:56:20] '[' 287 '!=' '' ']'
++ [06:56:20] echo -n 'called at line 287'
++ [06:56:20] echo

3. Possible reasons for error

a) Error in adding the remote - remote already exists.
   "fatal: remote quic_au already exists."
b) Error in fetching remote
c) Some projects gave merge conflict while
   AU merge is performed.

4. Actions

i)   Identify the error by looking at line where error occured
     (line 137 in our case).
ii)  If the error is a) then remove the remote from each
     project and run the script again.
iii) For error b remove the remote from each project, otherwise
     you will face error a, then run the script again.
iv)  For c resolve merge conflicts.

**************Case2***************
**********CDR Sync error**********

1. Output will contain lines of the form

ERROR: line 65 - command exited with status: 1
Error at function cdr_sync() called at line 293

Same understanding as for case 1 with error occuring inside cdr_sync().

2. Trace

ssh: connect to host ***.**.**.** port **: Connection timed out^M
fatal: The remote end hung up unexpectedly
ssh: connect to host ***.**.**.** port **: Connection timed out^M
fatal: The remote end hung up unexpectedly
error: Cannot fetch project_name
++ [04:29:43] traperror 65 cdr_sync 293
++ [04:29:43] local errcode=1
++ [04:29:43] local lineno=65
++ [04:29:43] '[' cdr_sync '!=' '' ']'
++ [04:29:43] local funcstack=cdr_sync
++ [04:29:43] '[' 293 '!=' '' ']'
++ [04:29:43] local linecallfunc=293
++ [04:29:43] echo 'ERROR: line 65 - command exited with status: 1'
++ [04:29:43] '[' cdr_sync '!=' '' ']'
++ [04:29:43] echo -n 'Error at function cdr_sync() '
++ [04:29:43] '[' 293 '!=' '' ']'
++ [04:29:43] echo -n 'called at line 293'
++ [04:29:43] echo

3. Action

i) Run the script again.

**************Case3***************

**********AU sync error**********

1. Output will contain lines of the form

ERROR: line 49 - command exited with status: 1
Error at function au_sync() called at line 287

2. Trace

error: Exited sync due to fetch errors
++ [06:56:20] traperror 49 au_sync 287
++ [06:56:20] local errcode=1
++ [06:56:20] local lineno=49
++ [06:56:20] '[' au_sync '!=' '' ']'
++ [06:56:20] local funcstack=au_sync
++ [06:56:20] '[' 287 '!=' '' ']'
++ [06:56:20] local linecallfunc=287
++ [06:56:20] echo 'ERROR: line 49 - command exited with status: 1'
++ [06:56:20] '[' au_sync '!=' '' ']'
++ [06:56:20] echo -n 'Error at function au_sync() '
++ [06:56:20] '[' 287 '!=' '' ']'
++ [06:56:20] echo -n 'called at line 287'
++ [06:56:20] echo

3. Action

i) Run the script again.

**************Case4***************
**********Missing projects**********

1. Output will contain lines of the form

Following QuIC projects are not present in CDR.
project_name
Follow the below steps before running the script again
1. Create the absent projects.
2. Add the projects to the manifest project.
3. Run the script

2. Trace

+ [02:21:16] [[ -n \nproject_name ]]
+ [02:21:16] echo -n 'Following QuIC projects are not present in CDR.'
+ [02:21:16] echo -e '\nproject_name'
+ [02:21:16] echo 'Follow the below steps before running the script again'
+ [02:21:16] echo '1. Create the absent projects.'
+ [02:21:16] echo '2. Add the projects to the manifest project.'
+ [02:21:16] echo '3. Run the script.'
+ [02:21:16] exit 1

3. Reason for failure
The mentioned project is present in the AU but not in the CDR server.

4. Action

Output log contains the action items.

**************Case5***************
**********Untagged heads of projects**********

1. Output will contain lines of the form

HEAD of following projects aren't tagged.
proj1
proj2

2. Reason for failure

Projects (proj1 and proj2 in the output above) doesn't have their HEAD
tagged.

3. Action

Tag the heads of all projects before running the script again.
