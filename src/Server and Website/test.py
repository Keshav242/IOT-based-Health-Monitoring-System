from socket import *
soc = socket(AF_INET, SOCK_STREAM)
soc.bind(('192.168.104.33', 4000))