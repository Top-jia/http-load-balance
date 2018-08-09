/*
 *	这个类的目地是设置程序后台化,
 *		进程信号掩码的设置
 *		描述符的设置
 *		改变工作的目录
 *		打开文件的权限(文件掩码)
 * */
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include<string>
#include"../log/log.hpp"

/*
 *	切换程序目录
 * */
#define SWITCH_DIR ".."

class Bstage{
		std::string new_dir;
	public:
		Bstage();
		void CloseFD();
		void SetFileMask();
		void SetSigMask();
		void OpenFD();
		void ChFileDir();
};
