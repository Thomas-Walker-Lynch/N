1. To see preprocessor output:

gcc -E try_assign.c

You should see:

#define myvar 42
#define pre_ID 99

2. To compile and run:

gcc try_assign.c -o try_assign
./try_assign

Expected output:

myvar: 42
pre_ID: 99

