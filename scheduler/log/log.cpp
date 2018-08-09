#include"log.hpp"

/*
 *	获取当前的时间
 * */
std::string Logger::GetCurTime(){
	time_t cur_time;
	time(&cur_time);
	void *ptr = ctime(&cur_time);
	assert(ptr != NULL);
	return std::string((const char *)ptr);
}

/*
 * 	写入文件信息
 * */
void Logger::WriteFile(bool errno_level, int errno_num, std::string location){
	char buffer[BUFF_SIZE];
	bzero(buffer, BUFF_SIZE);
	sprintf(buffer, "errno_level:\e[31m %s \e[0m location:%s errno:%d str_errno:%s \n",\
		   	errno_level == true?"dead_err":"warn", location.c_str(), errno_num, strerror(errno));
	
	std::string cur_time = GetCurTime();
	std::string error_info = cur_time + std::string(buffer);
	
	mutex.Lock();
	int fd = open(log_place.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
	if(fd <= 0){
		std::cout << "Logger::WriteFile_open failed errno = " << errno \
			<< " stderror(errno) = "<< strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	write(fd, cur_time.c_str(), cur_time.size()-1);
	write(fd, " ", 1);
	write(fd, buffer, strlen(buffer));
	if(errno_level == true){
		close(fd);
		mutex.Unlock();
		exit(EXIT_FAILURE);
	}
	close(fd);
	mutex.Unlock();
}

/*
 *	构造函数
 * */
Logger::Logger():log_place(LOG_FILE){
}
