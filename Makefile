http: 
	gcc response/response.c \
		request/request_parser.c \
		request/request_line.c \
		request/header.c \
		utils/list/list.c \
		utils/map/map.c \
		utils/string/string_manipulator.c \
  	utils/mime/mime_checker.c \
  	main.c \
  	-lmagic -g3 -o deploy/http.out
