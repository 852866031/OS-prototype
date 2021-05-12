compile.sh:
   do 
   chmod +x compile.sh
   ./compile
   to let the bash file work
   then do ./mykernel to run the programs
detail mode:
   Note that I implemented a "detail mode"
   When you run the programs
   do
   $ detail.on 
   to turn it on 
   and 
   $ detail.off 
   to turn it off
   This mode will display details when running the kernel, 

Test files:1
   the file testfile.txt is the professor provided testfile 
   (hand in through windows system (tested on mini) windows not support
    two files with name TESTFILE.TXT and testfile.txt exist at the same folder)
   the file TEST1.TXT and TEST2.TXT are my own test files

file system:
   This is a non-contiguous file system!!!!  
   file system commands: mount read write info echo rewind close seek are all implemented!!!
   when use my test files, better do rm -r PARITION to clean up the PARTITIONS
   and let it test all the cases as intended (such as create files)
   details as following
   Note that a file name is at most 16 char in length

command features:

mount filename number_of_blocks block_size
   1 if the PARTITION with the input name exists, it will open that PARTITION
   2 when you open a new PARTITION, the active file table will be empty 
     all files that is currently open will be closed and the file pointer (current location) will be put back to 0
   3 number_of_blocks and block_size must be greater than 0
   4 when you mount a existing PARTITION, you need to be aware that all file inside it is closed
     and all the files' current_location is at 0 (remember this so you will not overwrite you file accidentally)

info
   type "info" when you have already mount a PARTITION, shell will display all the information about this PARTITION
     example:
            $ info
            Partition name: PARTITION/new
            This partition has 10 blocks, each has size 10
               FILENAME     LENGTH     CUR_POSITION     STATUS     BLOCKS
               file1          2              2          open       0 2 
               file2          1              1          open       1 
            3 blocks used and 7 blocks free
      Note that BLOCKS shows all the blocks that this file takes up in this PARTITION
      you can see from here that this is a non-contiguous file system

write filename [  ]:
   1 if no PARTITION has been mounted, this will display error
   2 [] is required
   3 when this command will create a new file but there is no free pointer in the active file table
     this command will still create a new file in the fat but will not open it
   4 when this command will create a new file but there is no free block in the PARTITION, this will give an error
     and file will not be created or open

read filename variable
   1 if it is at the end of file, variable will contain "NULL"
   2 since write and read both increment the file pointer (current_location) if you want to read what you just write
     you need to do seek or rewind (see below)
   3 you can only read a file that exists in this PARTITION, read command will not create a new file
   4 error when there is no free pointer in the active file table

close filename
   1 close the file "filename", it will be removed from the active file table and its current_location will be 0
   2 the file must exist and be open

rewind filename
   1 put the file "filename"'s current_location to 0, so you can read or write from 0
   2 the file must exist and be open (note that all closed file's file pointer is at 0)

seek filename position 
   1 move the file "filename"'s current_location to position
   2 the file must exist and be open
   3 you cannot seek a file beyond its length, use info to see your file's length
     (if a file has length 3 you cannot seek file 4)
   4 system will give you a message if you seek to the end of the file  

echo messege
   display \nmessage\n\n

Note that by the pdf, both write and read increment the file pointer
so the testfile.txt: 
mount partition1 10 5
write file1 [hello world test] \\ write at 0 of file1 and pointer is at 2
read file1 var\\this will read NULL into var
print var\\NULL

and when you do the same script again, since you re-mount the partition
all files' pointer is put back to 0
write file1 [hello world test]\\ this will overwrite the [hello world test] at position 0
so var still contains NULL the same result will be displayed as the first time executing this script