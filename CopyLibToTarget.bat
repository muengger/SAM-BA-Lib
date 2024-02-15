
#!/usr/bin/env bash


# connect via scp
echo sshpass -p 'R0ssm@nnchen' scp -r $(pwd)/libSAMBALib.so root@$1:/usr/local/lib

sshpass -p 'R0ssm@nnchen' scp -r $(pwd)/libSAMBALib.so root@$1:/usr/local/lib
echo sshpass -p 'R0ssm@nnchen' ssh root@$1 'ldconfig'
sshpass -p 'R0ssm@nnchen' ssh root@$1 'ldconfig'



