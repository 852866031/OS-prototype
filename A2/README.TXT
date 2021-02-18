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
This mode will display every detail of exec like what function is called, which file is opened, the state of the ready list, etc