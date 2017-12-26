#include <iostream>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
using namespace std;

int32_t myexec(const char *cmd, std::vector<std::string> &resvec) {
    resvec.clear();
    FILE *pp = popen(cmd, "r"); //建立管道
    if (!pp) {
        return -1;
    }
    char tmp[1024]; //设置一个合适的长度，以存储每一行输出
    while (fgets(tmp, sizeof(tmp), pp) != NULL) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0'; //去除换行符
        }
        resvec.push_back(tmp);
    }
    pclose(pp); //关闭管道
    return resvec.size();
}

int main(int argc, const char * argv[]) {
    
    std::vector<std::string> *vect = new std::vector<std::string>();
    pid_t pid = getpid();
    char *cmd = new char[1024];
    char *cmd2 = new char[1024];
    char *cmd3 = new char[1024];

    char file_name[3] = {'b','b','b'};
	//file_name = get_file_name(); //生成文件名字1st
	//generate_data_file_txt();//生成数据文件2nd
     sprintf(cmd, "tar -cvf %s.tar %s.txt",file_name,file_name);
    int32_t a = myexec(cmd, *vect);

    sprintf(cmd2, "mv %s.tar /home/pi/data_iov/USER_FOLDER/",file_name);//修改为你自己的路径3rd
    a = myexec(cmd2, *vect); 

    sprintf(cmd3, "rm %s.txt",file_name);
    a = myexec(cmd3, *vect);/**/
    
    return 0;
}
