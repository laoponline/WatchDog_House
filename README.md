# Dog_House
A watch dog house, based on Qt,  holding multiple watch dog. Each dog could monitor a target process.
When a target process is missing from system tasklist, a dog can reboot it. Also a ram monitor function can be enabled, which keep tracking target process ram usage(not accuratly) to determine whether target process is stuck somewhere but still in system tasklist. 
When socket function is enabled, target porcess can feed the dog through a socket. When the coutdown get to 0 and no feeding message is received, target process will be killed and rebooted. Also target porcess can send log to a dog, the dog can write logs into files.


基于QT5.13的独立看门狗程序，可以监控多个进程。
通过进程名，判断目标是否还存活。如果目标消失，则会通过预设的路径重启目标。
目标进程可以通过Socke连接看门狗，主动进行喂狗。同时，可以记录目标发送的Log，保存到指定文件。
也可以不用Socket，仅仅由看门狗程序监控目标进程是否活动。通过其RAM占用量（绝对值不准）来判断目标是否还在活动。连续不变的内存占用，会被认为目标是假死了，也可以设置自动重启。
主界面上可以显示当前系统的CPU、内存和硬盘占用率
