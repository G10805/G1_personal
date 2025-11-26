Example command to generate wiki.html file for the required customer branch

./make_wiki_grease.sh <path_to_branches_folder> <branch_name> <mailid_for_log_output>

path_to_branches_folder : Please create the folder for customer branch.
                          Ex: CDR_TCL and please use sync_all.sh to sync the
                          sources from grease and codeaurora

Branch_name : Internal Branch name used for the customer.
              For example JB_REL, ICS_STRAWBERRY

mailid_for_log_output : Mail ID to send the logs. This is useful when the
                        process is automated during the production using
                        cronjob. Without logging into the server we can get the
                        errors if any during the wiki creation.

Instructions:

1. Please create dir named tmp where the branches dir is existed
2. The dir named branches must be existed at the path which was given as
   argument (path_to_branches_folder) for make_wiki_grease.sh
3. The code repository must be synced with repo only inside the dir
   which is having the name of branch for which the repo sync was done.
   This branch dir must be inside branches folder which was specified as
   argument (path_to_branches_folder)
4. To make this tool work, you must have the module named "python-git" be
   installed in your machine. Please use below command to install the above
   module

   sudo apt-get install python-git
