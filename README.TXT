Features:
1. accept file ends with empty lines
2. support the case when run command appear in the exec file, 
   in this case, the memory manager will also copy lines from the run file into the new file in BackingStore
3. As the professor said in piazza, DO NOT EXEC FILE WITH 11 OR MORE PAGES! we cant find victim in this case
4. Any quit in a script when exec will exit the shell  but a quit in a run script will only exit running this script but not        shell


do 

chmod +x compile.sh
./compile

to let the bash file work
then do ./mykernel to run the programs
Note that I implemented a "detail mode"
When you run the programs
do
$ detail.on 
to turn it on 
and 
$ detail.off 
to turn it off
This mode will display every detail of exec like what function is called, which file is opened, the state of the ready list, 
AND THE STATE OF THE PAGE TABLE AND PAGE FAULT HANDLING


