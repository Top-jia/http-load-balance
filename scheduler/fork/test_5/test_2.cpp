#include <http_parser.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int on_message_begin(http_parser *ptr){
	printf("\n on_message_begin() \n");
	return 0;
}

int on_message_complete(http_parser *ptr){
	printf("\n on_message_complete() \n");
	return 0;
}

int on_headers_complete(http_parser *ptr){
	printf("\n on_headers_complete() \n");
	return 0;
}

int on_url(http_parser *ptr, const char *field, long unsigned len){
	printf("\n on_url Url: %.*s\n", len, field);
	return 0;
}


int on_body(http_parser *ptr, const char *field, long unsigned len){
	printf("\n on_body : %.*s\n", len, field);
	return 0;
}

int on_header_field(http_parser *ptr, const char *field, long unsigned len){
	printf("\n on_header_field : %.*s\n", len, field);
	return 0;
}

int on_header_value(http_parser *ptr, const char *value, long unsigned len){
	printf("\n on_header_value : %.*s\n", len, value);
}

int main() {
	http_parser_settings settings;
	char buffer[] = "GET http://admin.omsg.cn/uploadpic/2016121034000012.png HTTP/1.1\r\nHost: admin.omsg.cn\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n";
	settings.on_message_begin = on_message_begin;
	settings.on_url = on_url;
	settings.on_header_field = on_header_field;
	settings.on_header_value = on_header_value;
	settings.on_headers_complete = on_headers_complete;
	//settings.on_body = on_body;
	settings.on_message_complete = on_message_complete;

	http_parser *parser = (http_parser*)malloc(sizeof(http_parser));
	assert(parser != NULL);

	http_parser_init(parser, HTTP_REQUEST);
	http_parser_execute(parser, &settings, buffer, strlen(buffer));
	free(parser);
	return 0;
}
