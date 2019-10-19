#include"bstage.hpp"

Logger log;
/*
 *	构造函数
 * */
Bstage::Bstage():new_dir(SWITCH_DIR){
	pid_t pid = fork();
	if(-1 == pid)
	{
		log.WriteFile(true, errno, "Bstage::Bstage()_fork failed ");
	}
	else if(0 < pid)
	{
		exit(0);
	}
	else
	{
		/*
		 *	创建新的会话, 变成首领进程
		 * */
		pid_t sid = setsid();
		if(-1 == sid)
		{
			log.WriteFile(false, errno, "Bstage()_fork failed ");
		}
	}
}

/*
 *	相对于对于进程的相关信息的设置, 等需要的时候在进行设置.
 * */
void Bstage::CloseFD(){
	log.WriteFile(true, errno, "Bstage::CloseFD_close failed ");
}

void Bstage::OpenFD(){

}

void Bstage::SetSigMask(){

}

void Bstage::SetFileMask(){

}

void Bstage::ChFileDir(){

}

