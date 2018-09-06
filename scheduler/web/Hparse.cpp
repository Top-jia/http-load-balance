#include"Hparse.hpp"

/*初始化相关的解析的资源*/
Hparse::Hparse(int fd):accept_fd{
	memset(buffer, '\0', HTTP_BUFF);
	data_read = 0;
	read_index = 0;
	check_index = 0;
	start_line = 0;
	checkstate = CHECK_STATE_REQUESTION;
}

HTTP_CODE Hparse::main_loop(){
	while(true){
		data_read = recv(accept_fd, buffer + read_index, HTTP_BUFF - read_index, 0);
		if(data_read == -1){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				break;
			}
			log.WriteFile(true, errno, "recv failed in Hparse::main_loop() ");
		}
		else if(data_read == -1){
			log.WriteFile(false, 1, "peer client closed in Hparse::main_loop() ");
			break;
		}
		read_index += data_read;
		HTTP_CODE result = parse_content();
		if(result == NO_REQUEST){
			continue;
		}
		else if(result == GET_REQUEST){
			return result;
		}
		else{
			return result;
		}
	}
}

HTTP_CODE parse_content(){
	LINE_STATUS linestatus = LINE_OK;
	HTTP_CODE recode = NO_REQUEST;
	while((linestatus == ))

}
